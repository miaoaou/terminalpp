#pragma once

#include "ui/widgets/window.h"
#include "ui/widgets/pager.h"
#include "ui/widgets/panel.h"
#include "ui/widgets/label.h"
#include "ui/widgets/dialog.h"
#include "ui-terminal/ansi_terminal.h"
#include "tpp-lib/local_pty.h"
#include "tpp-lib/bypass_pty.h"
#include "tpp-lib/remote_files.h"

#include "../config.h"
#include "../window.h"

#include "about_box.h"

#include <algorithm>
#include <cctype>

namespace tpp {

    /** New Version Dialog.
     */
    class NewVersionDialog : public ui::Dialog::Cancel {
    public:
        explicit NewVersionDialog(std::string const & message):
            ui::Dialog::Cancel{"New Version"},
            contents_{new ui::Label{message}} {
            setBody(contents_);
        }

    private:
        Label * contents_;
    };

    /** Paste confirmation dialog.
     */
    class PasteDialog : public ui::Dialog::YesNoCancel {
    public:

        explicit PasteDialog(std::string const & contents):
            ui::Dialog::YesNoCancel{"Are you sure you want to paste?"},
            contents_{new ui::Label{contents}} {
            setBody(contents_);
        }

        std::string const & contents() const {
            return contents_->text();
        }

    protected:

        void keyDown(KeyEvent::Payload & event) override {
            if (*event == SHORTCUT_PASTE) {
                dismiss(btnYes());
                return;
            }
            Dialog::YesNoCancel::keyDown(event);
        }

    private:
        Label * contents_;
    };

    /** Clipboard copy confirmation dialog.
     */
    class CopyDialog : public ui::Dialog::YesNoCancel {
    public:
        explicit CopyDialog(std::string const & contents):
            ui::Dialog::YesNoCancel{"Do you want to set clipboard to the following?"},
            contents_{new ui::Label{contents}} {
            setBody(contents_);
        }

        std::string const & contents() const {
            return contents_->text();
        }

    protected:

        void keyDown(KeyEvent::Payload & event) override {
            if (*event == SHORTCUT_COPY) {
                dismiss(btnYes());
                return;
            }
            Dialog::YesNoCancel::keyDown(event);
        }

    private:
        Label * contents_;
    };

    /** The terminal window.


     */
    class TerminalWindow : public ui::Window {
    public:

        explicit TerminalWindow(tpp::Window * window):
            window_{window},
            main_{new Panel{}},
            pager_{new Pager{}} {

            window_->onClose.setHandler(&TerminalWindow::windowCloseRequest, this);
            window_->onKeyDown.setHandler(&TerminalWindow::windowKeyDown, this);

            main_->setLayout(new Layout::Column{VerticalAlign::Top});
#if (defined ARCH_WINDOWS && defined RENDERER_NATIVE)
            main_->setBackground(Color::Black);
#endif
            main_->attach(pager_);
            setContents(main_);

            Config const & config = Config::Instance();
            remoteFiles_ = new RemoteFiles(config.remoteFiles.dir());

            pager_->onPageChange.setHandler(&TerminalWindow::activeSessionChanged, this);

            window_->setRoot(this);
            loadSSHHistory();
            versionChecker_ = std::thread{[this](){
                std::string channel = Config::Instance().version.checkChannel();
                if (channel.empty())
                    return;
                std::string newVersion = Application::Instance()->checkLatestVersion(channel);
                if (!newVersion.empty()) {
                    schedule([this, newVersion]() {
                        NewVersionDialog * d = new NewVersionDialog{STR("New version " << newVersion << " is available")};
                        showModal(d);
                    });
                }
            }};

            setFocusable(true);

            //setName("TerminalWindow");
        }

        ~TerminalWindow() override {
            versionChecker_.join();
            delete remoteFiles_;
        }

        void newSession(Config::sessions_entry const & session);


    private:
        class SessionInfo {
        public:
            std::string name;
            std::string title;
            std::string tabTitle;
            AnsiTerminal * terminal = nullptr;
            bool terminateOnKeyPress = false;
            std::string inputLine;
            std::string pendingSSHCommand;
            std::string pendingSSHTarget;
            std::string pendingSSHOutput;
            bool pendingSSHWasAlreadySaved = false;
            /** If true, the current session has an active notification.
             */
            bool notification = false;
            PasteDialog * pendingPaste = nullptr;

            explicit SessionInfo(Config::sessions_entry const & session):
                name{session.name()},
                title{session.name()} {
            }

            ~SessionInfo() {
                delete terminal;
            }
        };

        /** The window has been requested to close.

            TODO check that there are no active sessions and perhaps ask if there are whether really to exit.
         */
        void windowCloseRequest(tpp::Window::CloseEvent::Payload & e) {
            MARK_AS_UNUSED(e);
        }

        static bool parseSSHCommand(std::string const & line, std::string & target);
        static void eraseLastInputCharacter(std::string & line);
        void submitInputLine(SessionInfo & session);
        void observeTerminalInput(SessionInfo & session, std::string const & contents);
        void processSSHOutput(SessionInfo * session, std::string const & output);
        void discardPendingSSH(SessionInfo & session);
        void loadSSHHistory();
        void recordSSHHost(std::string const & host);
        void launchSSHHost(std::string const & host);

        /** Global hotkeys handling.
         */
        void windowKeyDown(tpp::Window::KeyEvent::Payload & e) {
            // a keydown also clears any active notification in the current session, and if this is the last active notification, also the notification icon
            if (activeSession_ != nullptr) {
                if (activeSession_->notification) {
                    activeSession_->notification = false;
                    ASSERT(activeNotifications_ > 0);
                    if (--activeNotifications_ == 0)
                        window_->setIcon(tpp::Window::Icon::Default);
                }
            }
            if (*e == SHORTCUT_FULLSCREEN) {
                window_->setFullscreen(! window_->fullscreen());
            } else if (*e == SHORTCUT_SETTINGS) {
                Application::Instance()->openLocalFile(Config::GetSettingsFile(), /* edit = */ true);
            } else if (*e == SHORTCUT_NEW_TAB) {
                createNewTab();
            } else if (*e == SHORTCUT_CLOSE_TAB && activeSession_ != nullptr) {
                closeSession(activeSession_);
            } else if (*e == SHORTCUT_NEXT_TAB) {
                cycleTab(1);
            } else if (*e == SHORTCUT_PREVIOUS_TAB) {
                cycleTab(-1);
            } else if (*e == SHORTCUT_ZOOM_IN || *e == SHORTCUT_ZOOM_IN_ALT) {
                if (window_->zoom() < 10) 
                    window_->setZoom(window_->zoom() * 1.25);
            } else if (*e == SHORTCUT_ZOOM_OUT || *e == SHORTCUT_ZOOM_OUT_ALT) {
                if (window_->zoom() > 1)
                    window_->setZoom(std::max(1.0, window_->zoom() / 1.25));
            } else if (*e == SHORTCUT_ABOUT && ! window_->isModal()) {
                showModal(new AboutBox{});
            } else {
                return;
            }
            e.stop();
        }

        void updateTabs() {
#if (defined ARCH_WINDOWS && defined RENDERER_NATIVE)
            std::vector<tpp::Window::TabBarItem> tabs;
            tabs.reserve(sessionOrder_.size());
            for (AnsiTerminal * terminal : sessionOrder_) {
                SessionInfo * session = sessionInfo(terminal);
                tpp::Window::TabBarItem tab;
                tab.title = session->tabTitle;
                tab.active = session == activeSession_;
                tab.activate = [this, terminal]() {
                    pager_->setActivePage(terminal);
                    window_->setKeyboardFocus(terminal);
                };
                tab.close = [this, terminal]() {
                    auto found = sessions_.find(terminal);
                    if (found != sessions_.end())
                        closeSession(found->second);
                };
                tabs.push_back(std::move(tab));
            }
            Config & config = Config::Instance();
            auto findSession = [&config](std::function<bool(Config::sessions_entry const &)> matches) -> std::string {
                for (size_t i = 0; i < config.sessions.size(); ++i) {
                    Config::sessions_entry const & session = config.sessions[i];
                    if (!session.hidden() && matches(session))
                        return session.name();
                }
                return {};
            };
            auto executableIs = [](Config::sessions_entry const & session, char const * executable) {
                std::string command = session.command().command();
                std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
                return command.find(executable) != std::string::npos;
            };
            std::string powershell = findSession([&](Config::sessions_entry const & session) {
                return executableIs(session, "pwsh") || executableIs(session, "powershell");
            });
            std::string commandPrompt = findSession([&](Config::sessions_entry const & session) {
                return executableIs(session, "cmd.exe");
            });
            std::string wsl;
            for (size_t i = 0; i < config.sessions.size(); ++i) {
                Config::sessions_entry const & session = config.sessions[i];
                if (session.name() == config.defaultSession() && executableIs(session, "wsl.exe")) {
                    wsl = session.name();
                    break;
                }
            }
            if (wsl.empty()) {
                for (size_t i = 0; i < config.sessions.size(); ++i) {
                    Config::sessions_entry const & session = config.sessions[i];
                    if (executableIs(session, "wsl.exe")) {
                        wsl = session.name();
                        break;
                    }
                }
            }
            if (wsl.empty()) {
                // Keep the WSL menu option usable even when no WSL profile was
                // discovered or configured. `wsl.exe` opens the default distro.
                std::string generatedName = "__terminalpp_wsl_default";
                for (size_t suffix = 1; ; ++suffix) {
                    bool nameTaken = false;
                    for (size_t i = 0; i < config.sessions.size(); ++i)
                        nameTaken = nameTaken || config.sessions[i].name() == generatedName;
                    if (!nameTaken)
                        break;
                    generatedName = STR("__terminalpp_wsl_default_" << suffix);
                }
                JSON wslSession = JSON::Object();
                wslSession.add("name", JSON{generatedName});
                wslSession.add("hidden", JSON{true});
                wslSession.add("pty", JSON{"local"});
                wslSession.add("command", JSON::Parse("[\"wsl.exe\"]"));
                wslSession.add("workingDirectory", JSON{HomeDir()});
                config.sessions.addElement(wslSession);
                wsl = generatedName;
            }
            auto launchSession = [this](std::string name) {
                if (!name.empty())
                    newSession(Config::Instance().sessionByName(name));
            };
            std::vector<tpp::Window::TabBarMenuItem> sshItems;
            for (std::string const & host : sshHosts_)
                sshItems.push_back({host, [this, host]() { launchSSHHost(host); }, true});
            if (sshItems.empty())
                sshItems.push_back({"No saved SSH connections", {}, false});
            std::vector<tpp::Window::TabBarMenuItem> dropDownItems{
                {"Windows PowerShell", [launchSession, powershell]() { launchSession(powershell); }, !powershell.empty()},
                {"Command Prompt", [launchSession, commandPrompt]() { launchSession(commandPrompt); }, !commandPrompt.empty()},
                {"WSL", [launchSession, wsl]() { launchSession(wsl); }, true},
                {"SSH", {}, true, std::move(sshItems)}
            };
            window_->setTabBar(std::move(tabs), [this]() { createNewTab(); }, [this]() { createNewTab(); }, std::move(dropDownItems));
            window_->setFileDropAction([this](std::vector<std::string> const & paths) {
                insertDroppedFiles(paths);
            });
#endif
        }

        void insertDroppedFiles(std::vector<std::string> const & paths) {
            if (activeSession_ == nullptr || activeSession_->terminal == nullptr || paths.empty())
                return;

            std::string command = Config::Instance().sessionByName(activeSession_->name).command().command();
            std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            bool const isPowerShell = command.find("pwsh") != std::string::npos || command.find("powershell") != std::string::npos;
            bool const isWsl = command.find("wsl.exe") != std::string::npos || command == "wsl";

            auto quotePath = [isPowerShell, isWsl](std::string path) {
                if (isWsl && path.size() >= 3 && std::isalpha(static_cast<unsigned char>(path[0])) &&
                    path[1] == ':' && (path[2] == '\\' || path[2] == '/')) {
                    std::string linuxPath{"/mnt/"};
                    linuxPath.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(path[0]))));
                    linuxPath.push_back('/');
                    for (size_t i = 3; i < path.size(); ++i)
                        linuxPath.push_back(path[i] == '\\' ? '/' : path[i]);
                    path = std::move(linuxPath);
                }

                std::string quoted{"'"};
                if (isPowerShell) {
                    for (char c : path) {
                        quoted.push_back(c);
                        if (c == '\'')
                            quoted.push_back('\'');
                    }
                    quoted.push_back('\'');
                    return quoted;
                }
                if (isWsl) {
                    for (char c : path) {
                        if (c == '\'')
                            quoted += "'\\''";
                        else
                            quoted.push_back(c);
                    }
                    quoted.push_back('\'');
                    return quoted;
                }
                return "\"" + path + "\"";
            };

            std::string contents;
            for (std::string const & path : paths) {
                if (!contents.empty())
                    contents.push_back(' ');
                contents += quotePath(path);
            }
            activeSession_->terminal->pasteContents(contents);
        }

        void createNewTab() {
            Config const & config = Config::Instance();
            std::string const sessionName = activeSession_ == nullptr ? config.defaultSession() : activeSession_->name;
            newSession(config.sessionByName(sessionName));
        }

        void selectTab(size_t index) {
            if (index >= sessionOrder_.size())
                return;
            AnsiTerminal * terminal = sessionOrder_[index];
            pager_->setActivePage(terminal);
            window_->setKeyboardFocus(terminal);
        }

        void closeTab(size_t index) {
            if (index >= sessionOrder_.size())
                return;
            closeSession(sessionInfo(sessionOrder_[index]));
        }

        void cycleTab(int direction) {
            if (sessionOrder_.size() < 2 || activeSession_ == nullptr)
                return;
            auto current = std::find(sessionOrder_.begin(), sessionOrder_.end(), activeSession_->terminal);
            if (current == sessionOrder_.end())
                return;
            int index = static_cast<int>(current - sessionOrder_.begin());
            int const count = static_cast<int>(sessionOrder_.size());
            index = (index + direction + count) % count;
            selectTab(static_cast<size_t>(index));
        }

        SessionInfo * sessionInfo(Widget * terminal) {
            AnsiTerminal * t = dynamic_cast<AnsiTerminal*>(terminal);
            ASSERT(t != nullptr);
            auto i = sessions_.find(t);
            ASSERT(i != sessions_.end());
            return (i->second);
        }

        void closeSession(SessionInfo * session) {
            UI_THREAD_ONLY;
            if (session->pendingPaste != nullptr)
                session->pendingPaste->dismiss(session->pendingPaste->btnCancel());
            sessionOrder_.erase(std::remove(sessionOrder_.begin(), sessionOrder_.end(), session->terminal), sessionOrder_.end());
            sessions_.erase(session->terminal);
            pager_->removePage(session->terminal);
            delete session;
            // if this was the last session, close the window
            if (sessions_.empty()) {
                updateTabs();
                window_->requestClose();
                return;
            }
            // otherwise focus the active session instead
            // TODO do we want this always, or should we actually check if the remove session was focused first?
            window_->setKeyboardFocus(pager_->activePage());
            updateTabs();
        }

        void sessionTitleChanged(ui::StringEvent::Payload & e) {
            SessionInfo * si = sessionInfo(e.sender());
            si->title = *e;
            updateTabs();
            if (activeSession_ == si)
                window_->setTitle(*e);
        }

        void hyperlinkOpen(ui::StringEvent::Payload & e) {
            Application::Instance()->openUrl(*e);
        }

        void hyperlinkCopy(ui::StringEvent::Payload & e) {
            Application::Instance()->setClipboard(*e);
        }

        /** Changes the icon when terminal sends notification.

            Changes the icon, marks the notification flag for the terminal's session and increments the window notifications counter.
         */
        void sessionNotification(ui::VoidEvent::Payload & e) {
            SessionInfo * si = sessionInfo(e.sender());
            // only increment the counter if current session does not have active notification enabled already
            if (si->notification == false) {
                si->notification = true;
                if (++activeNotifications_ == 1)
                    window_->setIcon(tpp::Window::Icon::Notification);
            }
        }

        void keyDown(KeyEvent::Payload & e) override {
            if (activeSession_->terminateOnKeyPress && ! e->isModifierKey())
                closeSession(activeSession_);
            else
                Widget::keyDown(e);
        }

        void activeSessionChanged(ui::Event<Widget*>::Payload & e) {
            activeSession_ = *e == nullptr ? nullptr : sessionInfo(*e);
            updateTabs();
            // set own background to the session's terminal background so that it propagates to the window's background
            if (activeSession_ != nullptr) {
                setBackground(activeSession_->terminal->palette().defaultBackground());
                window_->setTitle(activeSession_->title);
            }
        }

        void sessionPTYTerminated(ExitCodeEvent::Payload & e) {
            SessionInfo * si = sessionInfo(e.sender());
            window_->setIcon(tpp::Window::Icon::Notification);
            si->title = STR("Terminated, exit code " << *e);
            updateTabs();
            if (activeSession_ == si)
                window_->setTitle(si->title);
            Config const & config = Config::Instance();
            if (! config.renderer.window.waitAfterPtyTerminated()) {
                closeSession(si);
            } else {
                if (activeSession_ == si)
                    window_->setKeyboardFocus(this);
                si->terminateOnKeyPress = true;
            }
        }

        void terminalSetClipboard(StringEvent::Payload & e) {
            switch (Config::Instance().sequences.allowClipboardUpdate()) {
                case config::AllowClipboardUpdate::Deny:
                    break;
                case config::AllowClipboardUpdate::Allow:
                    setClipboard(*e);
                    break;
                case config::AllowClipboardUpdate::Ask: {
                    CopyDialog * d = new CopyDialog(*e);
                    d->onDismiss.setHandler([d, this](ui::Event<Widget*>::Payload & e) {
                        if (*e == d->btnYes())
                            setClipboard(d->contents());
                    });
                    showModal(d);
                    break;
                }
            }
        }

        void terminalPaste(StringEvent::Payload & e) {
            SessionInfo * si = sessionInfo(e.sender());
            switch (Config::Instance().sequences.confirmPaste()) {
                case config::ConfirmPaste::Never:
                    si->terminal->pasteContents(*e);
                    return;
                case config::ConfirmPaste::Multiline:
                    if ((*e).find('\n') == std::string::npos) {
                        si->terminal->pasteContents(*e);
                        return;
                    }
                    [[fallthrough]]; // fallthrough to always
                case config::ConfirmPaste::Always:
                    break;
            }
            if (si->pendingPaste != nullptr)
                si->pendingPaste->cancel();
            si->pendingPaste = new PasteDialog(*e);
            si->pendingPaste->onDismiss.setHandler([si](ui::Event<Widget*>::Payload & e) {
                if (*e == si->pendingPaste->btnYes()) {
                    si->terminal->pasteContents(si->pendingPaste->contents());
                }
                si->pendingPaste = nullptr;
            });
            showModal(si->pendingPaste);
        }

        void terminalKeyDown(KeyEvent::Payload & e) {
            SessionInfo * si = sessionInfo(e.sender());
            if (*e == SHORTCUT_PASTE) {
                si->terminal->requestClipboardPaste();
            } else if (*e == SHORTCUT_SCROLL_UP) {
                si->terminal->scrollBy(Point{0, -3});
            } else if (*e == SHORTCUT_SCROLL_DOWN) {
                si->terminal->scrollBy(Point{0, 3});
            } else if (*e == SHORTCUT_SCROLL_PAGE_UP) {
                si->terminal->scrollBy(Point{0, -si->terminal->height()});
            } else if (*e == SHORTCUT_SCROLL_PAGE_DOWN) {
                si->terminal->scrollBy(Point{0, si->terminal->height()});
            } else if (*e == SHORTCUT_SCROLL_TOP) {
                si->terminal->scrollBy(Point{0, INT_MIN});
            } else if (*e == SHORTCUT_SCROLL_BOTTOM) {
                si->terminal->scrollBy(Point{0, INT_MAX});
            } else {
                return;
            }
            e.stop();
        }

        void terminalInput(StringEvent::Payload & e) {
            SessionInfo * si = sessionInfo(e.sender());
            observeTerminalInput(*si, *e);
        }

        void terminalOutput(StringEvent::Payload & e) {
            AnsiTerminal * terminal = dynamic_cast<AnsiTerminal*>(e.sender());
            if (terminal == nullptr)
                return;
            std::string output = *e;
            window_->schedule([this, terminal, output = std::move(output)]() {
                auto found = sessions_.find(terminal);
                if (found != sessions_.end())
                    processSSHOutput(found->second, output);
            }, terminal);
        }

        void terminalTppSequence(TppSequenceEvent::Payload & event) {
            SessionInfo * si = sessionInfo(event.sender());
            try {
                switch (event->kind) {
                    case tpp::Sequence::Kind::GetCapabilities:
                        si->terminal->pty()->send(tpp::Sequence::Capabilities{1});
                        break;
                    case tpp::Sequence::Kind::OpenFileTransfer: {
                        Sequence::OpenFileTransfer req(event->payloadStart, event->payloadEnd);
                        si->terminal->pty()->send(remoteFiles_->openFileTransfer(req));
                        break;
                    }
                    case tpp::Sequence::Kind::Data: {
                        Sequence::Data data{event->payloadStart, event->payloadEnd};
                        remoteFiles_->transfer(data);
                        // make sure the UI thread remains responsive
                        window_->yieldToUIThread();
                        break;
                    }
                    case tpp::Sequence::Kind::GetTransferStatus: {
                        Sequence::GetTransferStatus req{event->payloadStart, event->payloadEnd};
                        si->terminal->pty()->send(remoteFiles_->getTransferStatus(req));
                        break;
                    }
                    case tpp::Sequence::Kind::ViewRemoteFile: {
                        Sequence::ViewRemoteFile req{event->payloadStart, event->payloadEnd};
                        RemoteFiles::File * f = remoteFiles_->get(req.id());
                        if (f == nullptr) {
                            si->terminal->pty()->send(Sequence::Nack{req, "No such file"});
                        } else if (! f->ready()) {
                            si->terminal->pty()->send(Sequence::Nack(req, "File not transferred"));
                        } else {
                            // send the ack first in case there are local issues with the opening
                            si->terminal->pty()->send(Sequence::Ack{req, req.id()});
                            Application::Instance()->openLocalFile(f->localPath(), false);
                        }
                        break;
                    }
                    default:
                        LOG() << "Unknown sequence";
                        break;
                }
            } catch (std::exception const & e) {
                showError(e.what());
            }
        }


        tpp::Window * window_;

        ui::Panel * main_;
        ui::Pager * pager_;

#if (defined ARCH_WINDOWS && defined RENDERER_NATIVE)
        unsigned nextTabNumber_ = 1;
#endif

        std::unordered_map<AnsiTerminal *, SessionInfo *> sessions_;
        std::vector<AnsiTerminal *> sessionOrder_;
        std::vector<std::string> sshHosts_;
        unsigned nextSSHSessionId_ = 1;

        SessionInfo * activeSession_ = nullptr;
        unsigned activeNotifications_ = 0;

        RemoteFiles * remoteFiles_;

        std::thread versionChecker_;

    };

} // namespace tpp
