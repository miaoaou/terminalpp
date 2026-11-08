#include "terminal_window.h"

#include <fstream>
#include <stdexcept>

#include "helpers/filesystem.h"

#if (ARCH_WINDOWS)
#include <cctype>

namespace {
    class ScopedPowerShellUpdateCheck {
    public:
        ScopedPowerShellUpdateCheck() {
            SetLastError(ERROR_SUCCESS);
            DWORD const size = GetEnvironmentVariableW(L"POWERSHELL_UPDATECHECK", nullptr, 0);
            DWORD const error = GetLastError();
            if (size > 0) {
                std::vector<wchar_t> value(size);
                DWORD const length = GetEnvironmentVariableW(L"POWERSHELL_UPDATECHECK", value.data(), size);
                if (length > 0 && length < size) {
                    previous_.assign(value.data(), length);
                    hadPrevious_ = true;
                }
            } else if (error != ERROR_ENVVAR_NOT_FOUND) {
                hadPrevious_ = true;
            }
            SetEnvironmentVariableW(L"POWERSHELL_UPDATECHECK", L"Off");
        }

        ~ScopedPowerShellUpdateCheck() {
            SetEnvironmentVariableW(L"POWERSHELL_UPDATECHECK", hadPrevious_ ? previous_.c_str() : nullptr);
        }

    private:
        std::wstring previous_;
        bool hadPrevious_ = false;
    };

    void configurePowerShellUtf8(Command & command) {
        std::string executable = command.command();
        std::transform(executable.begin(), executable.end(), executable.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        size_t const separator = executable.find_last_of("\\/");
        if (separator != std::string::npos)
            executable.erase(0, separator + 1);
        if (executable != "pwsh.exe" && executable != "pwsh" &&
            executable != "powershell.exe" && executable != "powershell")
            return;

        std::vector<std::string> args = command.args();
        auto hasSwitch = [&args](std::initializer_list<char const *> switches) {
            for (std::string const & arg : args) {
                std::string lower = arg;
                std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
                for (char const * option : switches) {
                    if (lower == option)
                        return true;
                }
            }
            return false;
        };

        // PowerShell's own command line or script may depend on its existing
        // startup semantics. For an interactive shell, add an initialization
        // command before the first prompt so ConPTY UTF-8 input is decoded as
        // UTF-8 even when Windows' active console code page is legacy.
        if (hasSwitch({"-command", "-c", "-file", "-f", "-encodedcommand", "-e"}))
            return;
        if (!hasSwitch({"-nologo"}))
            args.push_back("-NoLogo");
        if (!hasSwitch({"-noexit"}))
            args.push_back("-NoExit");
        args.push_back("-Command");
        args.push_back(
            "$utf8 = [System.Text.UTF8Encoding]::new($false); "
            "[Console]::InputEncoding = $utf8; "
            "[Console]::OutputEncoding = $utf8; "
            "$OutputEncoding = $utf8");
        command.setArgs(args);
    }
}
#endif

namespace tpp {

    using namespace ui;

#if (defined ARCH_WINDOWS && defined RENDERER_NATIVE)
    JSON TerminalWindow::tabSnapshot(SessionInfo const & session, size_t index) const {
        std::vector<std::string> rows = session.terminal->textRows();
        std::string text;
        for (size_t row = 0; row < rows.size(); ++row) {
            if (row != 0)
                text.push_back('\n');
            text += rows[row];
        }

        JSON result = JSON::Object();
        result.add("id", JSON{static_cast<int>(index)});
        result.add("title", JSON{session.tabTitle.empty() ? session.title : session.tabTitle});
        result.add("terminal_title", JSON{session.title});
        result.add("active", JSON{activeSession_ == &session});
        result.add("text", JSON{text});
        JSON rowList = JSON::Array();
        for (std::string const & row : rows)
            rowList.add(JSON{row});
        result.add("rows", std::move(rowList));
        return result;
    }

    JSON TerminalWindow::handleControlRequest(JSON const & request) {
        if (request.kind() != JSON::Kind::Object || !request.hasKey("op") || request["op"].kind() != JSON::Kind::String)
            throw std::runtime_error("Request must contain a string 'op'");

        std::string const & operation = request["op"].toString();
        if (operation == "list_tabs" || operation == "read_all") {
            JSON tabs = JSON::Array();
            for (size_t index = 0; index < sessionOrder_.size(); ++index)
                tabs.add(tabSnapshot(*sessionInfo(sessionOrder_[index]), index));
            JSON result = JSON::Object();
            result.add("tabs", std::move(tabs));
            return result;
        }

        if (!request.hasKey("id") || request["id"].kind() != JSON::Kind::Integer)
            throw std::runtime_error("Request must contain an integer 'id'");
        unsigned const index = request["id"].toUnsigned();
        if (index >= sessionOrder_.size())
            throw std::runtime_error("No tab exists with that id");

        SessionInfo * session = sessionInfo(sessionOrder_[index]);
        if (operation == "read_tab")
            return tabSnapshot(*session, index);

        if (operation == "send_text") {
            if (!request.hasKey("text") || request["text"].kind() != JSON::Kind::String)
                throw std::runtime_error("Request must contain a string 'text'");
            bool enter = false;
            if (request.hasKey("enter")) {
                if (request["enter"].kind() != JSON::Kind::Boolean)
                    throw std::runtime_error("'enter' must be a boolean");
                enter = request["enter"].toBool();
            }
            std::string const & text = request["text"].toString();
            if (!text.empty())
                session->terminal->pasteContents(text);
            if (enter)
                session->terminal->pasteContents("\r");

            JSON result = JSON::Object();
            result.add("id", JSON{static_cast<int>(index)});
            result.add("sent", JSON{true});
            result.add("bytes", JSON{static_cast<int>(text.size())});
            result.add("enter", JSON{enter});
            return result;
        }

        throw std::runtime_error("Unknown operation: " + operation);
    }
#endif

    void TerminalWindow::newSession(Config::sessions_entry const & session) {
        Config const & config = Config::Instance();
        std::unique_ptr<SessionInfo> si{new SessionInfo{session}};
        // create the pty
        PTYMaster * pty = nullptr;
        // sets the working directory of the command to the specified working directory of the session,
        Command cmd = session.command();
        cmd.setWorkingDirectory(session.workingDirectory());
#if (ARCH_WINDOWS)
        std::string executable = cmd.command();
        std::transform(executable.begin(), executable.end(), executable.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        configurePowerShellUtf8(cmd);
        std::unique_ptr<ScopedPowerShellUpdateCheck> updateCheck;
        if (executable.find("pwsh") != std::string::npos)
            updateCheck.reset(new ScopedPowerShellUpdateCheck{});
        if (session.pty() != "bypass") 
            pty = new LocalPTYMaster(cmd);
        else
            pty = new BypassPTYMaster(cmd);
        updateCheck.reset();
#else
        pty = new LocalPTYMaster{cmd};
#endif
        // and the terminal
        si->terminal = new AnsiTerminal{pty, session.palette()};
        si->terminal->setMaxHistoryRows(config.renderer.window.historyLimit());
        si->terminal->setBoldIsBright(config.sequences.boldIsBright());
        si->terminal->setDisplayBold(config.sequences.displayBold());
        si->terminal->setCursor(session.cursor());
        si->terminal->setInactiveCursorColor(session.cursor.inactiveColor());
        si->terminal->setAllowCursorChanges(config.sequences.allowCursorChanges());
        si->terminal->setAllowOSCHyperlinks(config.sequences.allowOSCHyperlinks());
        si->terminal->setDetectHyperlinks(config.sequences.detectHyperlinks());
        si->terminal->setNormalHyperlinkStyle(config.renderer.hyperlinks.normal());
        si->terminal->setActiveHyperlinkStyle(config.renderer.hyperlinks.active());
        // register the session and set it as active page
        AnsiTerminal * t = si->terminal;
#if (defined ARCH_WINDOWS && defined RENDERER_NATIVE)
        si->tabTitle = STR("Tab" << nextTabNumber_++);
#endif
        sessions_.insert(std::make_pair(t, si.release()));
        sessionOrder_.push_back(t);
        pager_->setActivePage(t);
        window_->setKeyboardFocus(t);
        // add terminal events (so that they are called only *after* the session is registered)
        t->onPTYTerminated.setHandler(&TerminalWindow::sessionPTYTerminated, this);
        t->onTitleChange.setHandler(&TerminalWindow::sessionTitleChanged, this);
        t->onClipboardSetRequest.setHandler(&TerminalWindow::terminalSetClipboard, this);
        t->onPaste.setHandler(&TerminalWindow::terminalPaste, this);
        t->onNotification.setHandler(&TerminalWindow::sessionNotification, this);
        t->onInput.setHandler(&TerminalWindow::terminalInput, this);
        t->onOutput.setHandler(&TerminalWindow::terminalOutput, this);
        t->onTppSequence.setHandler(&TerminalWindow::terminalTppSequence, this);
        t->onKeyDown.setHandler(&TerminalWindow::terminalKeyDown, this);
        t->onHyperlinkOpen.setHandler(&TerminalWindow::hyperlinkOpen, this);
        t->onHyperlinkCopy.setHandler(&TerminalWindow::hyperlinkCopy, this);
        updateTabs();
    }

    bool TerminalWindow::parseSSHCommand(std::string const & line, std::string & target) {
        std::vector<std::string> args;
        std::string token;
        char quote = 0;
        bool escaped = false;
        for (char c : line) {
            if (escaped) {
                token.push_back(c);
                escaped = false;
            } else if (c == '\\' && quote != '\'') {
                escaped = true;
            } else if (quote != 0) {
                if (c == quote)
                    quote = 0;
                else
                    token.push_back(c);
            } else if (c == '\'' || c == '"') {
                quote = c;
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                if (!token.empty()) {
                    args.push_back(std::move(token));
                    token.clear();
                }
            } else {
                token.push_back(c);
            }
        }
        if (!token.empty())
            args.push_back(std::move(token));
        if (args.empty())
            return false;
        size_t commandIndex = 0;
        if (args[0] == "&")
            commandIndex = 1;
        if (commandIndex >= args.size())
            return false;
        std::string executable = args[commandIndex];
        size_t const slash = executable.find_last_of("/\\");
        if (slash != std::string::npos)
            executable.erase(0, slash + 1);
        std::transform(executable.begin(), executable.end(), executable.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        if (executable != "ssh" && executable != "ssh.exe")
            return false;

        std::string login;
        static std::vector<std::string> const optionsWithValue{
            "-b", "-c", "-D", "-E", "-e", "-F", "-I", "-i", "-J", "-L", "-m", "-O", "-o", "-p", "-Q", "-R", "-S", "-W", "-w"
        };
        for (size_t i = commandIndex + 1; i < args.size(); ++i) {
            std::string const & arg = args[i];
            if (arg == "--") {
                if (++i < args.size())
                    target = args[i];
                break;
            }
            if (arg == "-l") {
                if (i + 1 < args.size())
                    login = args[++i];
                continue;
            }
            if (arg.size() > 1 && arg[0] == '-') {
                if (std::find(optionsWithValue.begin(), optionsWithValue.end(), arg) != optionsWithValue.end() && i + 1 < args.size())
                    ++i;
                continue;
            }
            target = arg;
            break;
        }
        if (target.empty())
            return false;
        if (!login.empty() && target.find('@') == std::string::npos)
            target = login + "@" + target;
        return target.find_first_of(" \r\n\t") == std::string::npos;
    }

    void TerminalWindow::submitInputLine(SessionInfo & session) {
        if (session.pendingSSHTarget.empty()) {
            session.pendingSSHCommand = session.inputLine;
            parseSSHCommand(session.inputLine, session.pendingSSHTarget);
            if (!session.pendingSSHTarget.empty()) {
                session.pendingSSHOutput.clear();
            }
        }
        // Discard any partial prompt redraws. The submitted input line, or a
        // complete echoed command line for scripted input, is authoritative.
        session.sshCommandOutputBuffer.clear();
        session.inputLine.clear();
        session.inputCursorPosition = 0;
        session.inputEscapeSequence.clear();
    }

    void TerminalWindow::discardPendingSSH(SessionInfo & session) {
        session.pendingSSHTarget.clear();
        session.pendingSSHCommand.clear();
        session.pendingSSHOutput.clear();
        session.sshCommandOutputBuffer.clear();
    }

    void TerminalWindow::observeTerminalInput(SessionInfo & session, std::string const & contents) {
        bool previousWasCR = false;
        auto erasePreviousCharacter = [&session]() {
            if (session.inputCursorPosition == 0)
                return;
            size_t start = session.inputCursorPosition - 1;
            while (start > 0 && (static_cast<unsigned char>(session.inputLine[start]) & 0xC0) == 0x80)
                --start;
            session.inputLine.erase(start, session.inputCursorPosition - start);
            session.inputCursorPosition = start;
        };
        auto eraseNextCharacter = [&session]() {
            if (session.inputCursorPosition >= session.inputLine.size())
                return;
            size_t end = session.inputCursorPosition + 1;
            while (end < session.inputLine.size() && (static_cast<unsigned char>(session.inputLine[end]) & 0xC0) == 0x80)
                ++end;
            session.inputLine.erase(session.inputCursorPosition, end - session.inputCursorPosition);
        };
        auto movePreviousCharacter = [&session]() {
            if (session.inputCursorPosition == 0)
                return;
            --session.inputCursorPosition;
            while (session.inputCursorPosition > 0 &&
                   (static_cast<unsigned char>(session.inputLine[session.inputCursorPosition]) & 0xC0) == 0x80)
                --session.inputCursorPosition;
        };
        auto moveNextCharacter = [&session]() {
            if (session.inputCursorPosition >= session.inputLine.size())
                return;
            ++session.inputCursorPosition;
            while (session.inputCursorPosition < session.inputLine.size() &&
                   (static_cast<unsigned char>(session.inputLine[session.inputCursorPosition]) & 0xC0) == 0x80)
                ++session.inputCursorPosition;
        };
        auto moveWordLeft = [&session]() {
            while (session.inputCursorPosition > 0 &&
                   std::isspace(static_cast<unsigned char>(session.inputLine[session.inputCursorPosition - 1])))
                --session.inputCursorPosition;
            while (session.inputCursorPosition > 0 &&
                   !std::isspace(static_cast<unsigned char>(session.inputLine[session.inputCursorPosition - 1])))
                --session.inputCursorPosition;
        };
        auto moveWordRight = [&session]() {
            while (session.inputCursorPosition < session.inputLine.size() &&
                   !std::isspace(static_cast<unsigned char>(session.inputLine[session.inputCursorPosition])))
                ++session.inputCursorPosition;
            while (session.inputCursorPosition < session.inputLine.size() &&
                   std::isspace(static_cast<unsigned char>(session.inputLine[session.inputCursorPosition])))
                ++session.inputCursorPosition;
        };
        auto handleEscapeSequence = [&session, &eraseNextCharacter, &movePreviousCharacter, &moveNextCharacter, &moveWordLeft, &moveWordRight](std::string const & sequence) {
            if (sequence.size() < 2 || sequence[0] != '\x1b')
                return;
            if (sequence[1] == '[' && sequence.size() >= 3) {
                char const command = sequence.back();
                std::string const parameters = sequence.substr(2, sequence.size() - 3);
                bool const control = parameters.find(";5") != std::string::npos;
                switch (command) {
                    case 'D':
                        if (control)
                            moveWordLeft();
                        else
                            movePreviousCharacter();
                        break;
                    case 'C':
                        if (control)
                            moveWordRight();
                        else
                            moveNextCharacter();
                        break;
                    case 'H':
                        session.inputCursorPosition = 0;
                        break;
                    case 'F':
                        session.inputCursorPosition = session.inputLine.size();
                        break;
                    case '~':
                        if (parameters == "1" || parameters == "7")
                            session.inputCursorPosition = 0;
                        else if (parameters == "4" || parameters == "8")
                            session.inputCursorPosition = session.inputLine.size();
                        else if (parameters == "3")
                            eraseNextCharacter();
                        break;
                    default:
                        break;
                }
            } else if (sequence[1] == 'O' && sequence.size() == 3) {
                switch (sequence[2]) {
                    case 'D': movePreviousCharacter(); break;
                    case 'C': moveNextCharacter(); break;
                    case 'H': session.inputCursorPosition = 0; break;
                    case 'F': session.inputCursorPosition = session.inputLine.size(); break;
                    default: break;
                }
            } else if (sequence == "\x1b" "b") {
                moveWordLeft();
            } else if (sequence == "\x1b" "f") {
                moveWordRight();
            }
        };

        for (unsigned char c : contents) {
            if (!session.inputEscapeSequence.empty()) {
                if (c == 0x1b) {
                    session.inputEscapeSequence.assign(1, '\x1b');
                    previousWasCR = false;
                    continue;
                }
                if (c == '\r' || c == '\n' || c == 0x03) {
                    session.inputEscapeSequence.clear();
                } else {
                    session.inputEscapeSequence.push_back(static_cast<char>(c));
                    if (session.inputEscapeSequence.size() == 2 && c != '[' && c != 'O') {
                        handleEscapeSequence(session.inputEscapeSequence);
                        session.inputEscapeSequence.clear();
                    } else if (session.inputEscapeSequence.size() == 3 && session.inputEscapeSequence[1] == 'O') {
                        handleEscapeSequence(session.inputEscapeSequence);
                        session.inputEscapeSequence.clear();
                    } else if (session.inputEscapeSequence.size() > 2 && session.inputEscapeSequence[1] == '[' &&
                               c >= '@' && c <= '~') {
                        handleEscapeSequence(session.inputEscapeSequence);
                        session.inputEscapeSequence.clear();
                    }
                    previousWasCR = false;
                    continue;
                }
            }
            if (c == '\r' || c == '\n') {
                if (!(c == '\n' && previousWasCR))
                    submitInputLine(session);
                previousWasCR = c == '\r';
            } else if (c == 0x03) {
                if (!session.pendingSSHTarget.empty())
                    discardPendingSSH(session);
                else {
                    session.inputLine.clear();
                    session.inputCursorPosition = 0;
                }
                previousWasCR = false;
            } else {
                previousWasCR = false;
                if (c == '\x1b') {
                    session.inputEscapeSequence.assign(1, '\x1b');
                } else if (c == '\b' || c == 0x7f) {
                    if (session.pendingSSHTarget.empty())
                        erasePreviousCharacter();
                } else if (session.pendingSSHTarget.empty()) {
                    if (c == 0x01)
                        session.inputCursorPosition = 0;
                    else if (c == 0x05)
                        session.inputCursorPosition = session.inputLine.size();
                    else if (c == 0x04)
                        eraseNextCharacter();
                    else if (c == 0x0b)
                        session.inputLine.erase(session.inputCursorPosition);
                    else if (c == 0x15) {
                        session.inputLine.erase(0, session.inputCursorPosition);
                        session.inputCursorPosition = 0;
                    } else if (c == 0x17) {
                        while (session.inputCursorPosition > 0 &&
                               std::isspace(static_cast<unsigned char>(session.inputLine[session.inputCursorPosition - 1])))
                            erasePreviousCharacter();
                        while (session.inputCursorPosition > 0 &&
                               !std::isspace(static_cast<unsigned char>(session.inputLine[session.inputCursorPosition - 1])))
                            erasePreviousCharacter();
                    } else if (c >= 0x20) {
                        session.inputLine.insert(session.inputCursorPosition, 1, static_cast<char>(c));
                        ++session.inputCursorPosition;
                    }
                }
            }
        }
    }

    void TerminalWindow::processSSHOutput(SessionInfo * session, std::string const & output) {
        if (session == nullptr)
            return;
        bool detectedFromOutput = false;
        if (session->pendingSSHTarget.empty() && session->inputLine.empty()) {
            session->sshCommandOutputBuffer += output;
            if (session->sshCommandOutputBuffer.size() > 32768)
                session->sshCommandOutputBuffer.erase(0, session->sshCommandOutputBuffer.size() - 16384);

            while (true) {
                size_t const lineEnd = session->sshCommandOutputBuffer.find_first_of("\r\n");
                if (lineEnd == std::string::npos)
                    break;
                std::string line = session->sshCommandOutputBuffer.substr(0, lineEnd);
                size_t consumed = lineEnd + 1;
                if (session->sshCommandOutputBuffer[lineEnd] == '\r' && consumed < session->sshCommandOutputBuffer.size() &&
                    session->sshCommandOutputBuffer[consumed] == '\n')
                    ++consumed;

                std::string lowerLine = line;
                std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
                bool foundCommand = false;
                for (std::string const & marker : {std::string("ssh "), std::string("ssh.exe ")}) {
                    size_t position = lowerLine.find(marker);
                    while (position != std::string::npos) {
                        if (position == 0 || std::isspace(static_cast<unsigned char>(line[position - 1])) || line[position - 1] == '>') {
                            std::string target;
                            std::string const command = line.substr(position);
                            if (parseSSHCommand(command, target)) {
                                session->pendingSSHCommand = command;
                                session->pendingSSHTarget = std::move(target);
                                session->pendingSSHOutput = session->sshCommandOutputBuffer;
                                session->sshCommandOutputBuffer.clear();
                                detectedFromOutput = true;
                                foundCommand = true;
                                break;
                            }
                        }
                        position = lowerLine.find(marker, position + marker.size());
                    }
                    if (foundCommand)
                        break;
                }
                if (foundCommand)
                    break;
                session->sshCommandOutputBuffer.erase(0, consumed);
            }
        }
        if (session->pendingSSHTarget.empty())
            return;
        if (!detectedFromOutput)
            session->pendingSSHOutput += output;
        if (session->pendingSSHOutput.size() > 32768)
            session->pendingSSHOutput.erase(0, session->pendingSSHOutput.size() - 16384);

        size_t const commandPos = session->pendingSSHOutput.find(session->pendingSSHTarget);
        size_t lineEnd = 0;
        if (commandPos != std::string::npos) {
            lineEnd = session->pendingSSHOutput.find_first_of("\r\n", commandPos + session->pendingSSHTarget.size());
            if (lineEnd == std::string::npos)
                return;
            while (lineEnd < session->pendingSSHOutput.size() &&
                   (session->pendingSSHOutput[lineEnd] == '\r' || session->pendingSSHOutput[lineEnd] == '\n'))
                ++lineEnd;
        }

        std::string lowerResponse = session->pendingSSHOutput.substr(lineEnd);
        std::transform(lowerResponse.begin(), lowerResponse.end(), lowerResponse.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        static std::vector<std::string> const failures{
            "permission denied", "could not resolve hostname", "name or service not known",
            "connection refused", "connection timed out", "operation timed out", "no route to host",
            "host key verification failed", "too many authentication failures", "ssh: connect to host",
            "connection reset by peer", "connection closed by"
        };
        if (std::any_of(failures.begin(), failures.end(), [&lowerResponse](std::string const & marker) {
            return lowerResponse.find(marker) != std::string::npos;
        })) {
            // The terminal can print these strings after an SSH connection has
            // already succeeded (for example, from a remote shell startup
            // script). Keep the submitted destination in history; clearing it
            // here made successful logins disappear from settings.json.
            session->pendingSSHTarget.clear();
            session->pendingSSHCommand.clear();
            session->pendingSSHOutput.clear();
            session->sshCommandOutputBuffer.clear();
            return;
        }

        std::string response;
        for (size_t i = lineEnd; i < session->pendingSSHOutput.size(); ++i) {
            unsigned char c = static_cast<unsigned char>(session->pendingSSHOutput[i]);
            if (c == 0x1b) {
                if (++i >= session->pendingSSHOutput.size())
                    break;
                if (session->pendingSSHOutput[i] == '[') {
                    while (++i < session->pendingSSHOutput.size())
                        if (session->pendingSSHOutput[i] >= '@' && session->pendingSSHOutput[i] <= '~')
                            break;
                } else if (session->pendingSSHOutput[i] == ']') {
                    while (++i < session->pendingSSHOutput.size() && session->pendingSSHOutput[i] != '\a') {
                        if (session->pendingSSHOutput[i] == '\x1b' && i + 1 < session->pendingSSHOutput.size() && session->pendingSSHOutput[i + 1] == '\\') {
                            ++i;
                            break;
                        }
                    }
                }
                continue;
            }
            if (c >= 0x20 || c == '\r' || c == '\n' || c == '\t')
                response.push_back(static_cast<char>(c));
        }
        while (!response.empty() && std::isspace(static_cast<unsigned char>(response.back())))
            response.pop_back();
        if (response.empty())
            return;
        size_t const lastLine = response.find_last_of("\r\n");
        std::string prompt = response.substr(lastLine == std::string::npos ? 0 : lastLine + 1);
        while (!prompt.empty() && std::isspace(static_cast<unsigned char>(prompt.front())))
            prompt.erase(prompt.begin());
        if (prompt.empty())
            return;
        std::string promptLower = prompt;
        std::transform(promptLower.begin(), promptLower.end(), promptLower.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        static std::vector<std::string> const authenticationPrompts{
            "password:", "passphrase", "continue connecting", "yes/no", "verification code", "one-time password"
        };
        if (std::any_of(authenticationPrompts.begin(), authenticationPrompts.end(), [&promptLower](std::string const & marker) {
            return promptLower.find(marker) != std::string::npos;
        }))
            return;

        std::string const host = session->pendingSSHTarget;
        session->pendingSSHTarget.clear();
        session->pendingSSHCommand.clear();
        session->pendingSSHOutput.clear();
        session->sshCommandOutputBuffer.clear();
        recordSSHHost(host);
    }

    void TerminalWindow::loadSSHHistory() {
        Config & config = Config::Instance();
        sshHosts_ = config.sshHosts();
        std::string const historyPath = JoinPath(Config::GetSettingsFolder(), "ssh-history.txt");
        std::ifstream history{historyPath};
        bool const legacyHistoryOpened = history.is_open();
        std::string host;
        bool historyChanged = false;
        while (std::getline(history, host)) {
            if (host.empty() || host.find_first_of("\r\n\t ") != std::string::npos)
                continue;
            if (std::find(sshHosts_.begin(), sshHosts_.end(), host) == sshHosts_.end()) {
                sshHosts_.push_back(std::move(host));
                historyChanged = true;
            }
        }
        if (sshHosts_.size() > 100) {
            sshHosts_.erase(sshHosts_.begin(), sshHosts_.end() - 100);
            historyChanged = true;
        }
        if (historyChanged)
            config.setSSHHosts(sshHosts_);
        history.close();
        if (legacyHistoryOpened) {
#if (defined ARCH_WINDOWS)
            std::filesystem::remove(std::filesystem::path{UTF8toUTF16(historyPath)});
#else
            std::filesystem::remove(std::filesystem::path{historyPath});
#endif
        }
    }

    void TerminalWindow::recordSSHHost(std::string const & host) {
        auto found = std::find(sshHosts_.begin(), sshHosts_.end(), host);
        if (found != sshHosts_.end())
            sshHosts_.erase(found);
        sshHosts_.push_back(host);
        if (sshHosts_.size() > 100)
            sshHosts_.erase(sshHosts_.begin());
        Config::Instance().setSSHHosts(sshHosts_);
        updateTabs();
    }

    void TerminalWindow::launchSSHHost(std::string const & host) {
        Config & config = Config::Instance();
        std::string const sessionName = STR("__terminalpp_ssh_" << nextSSHSessionId_++);
        JSON sessionJson = JSON::Object();
        sessionJson.add("name", JSON{sessionName});
        sessionJson.add("hidden", JSON{true});
        sessionJson.add("pty", JSON{"local"});
        JSON command = JSON::Array();
        command.add(JSON{"ssh"});
        command.add(JSON{host});
        sessionJson.add("command", command);
        sessionJson.add("workingDirectory", JSON{HomeDir()});
        Config::sessions_entry & session = config.sessions.addElement(sessionJson);
        newSession(session);
    }

} // namespace tpp

