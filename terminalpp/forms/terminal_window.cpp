#include "terminal_window.h"

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
}
#endif

namespace tpp {

    using namespace ui;

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
        t->onTppSequence.setHandler(&TerminalWindow::terminalTppSequence, this);
        t->onKeyDown.setHandler(&TerminalWindow::terminalKeyDown, this);
        t->onHyperlinkOpen.setHandler(&TerminalWindow::hyperlinkOpen, this);
        t->onHyperlinkCopy.setHandler(&TerminalWindow::hyperlinkCopy, this);
        updateTabs();
    }

} // namespace tpp

