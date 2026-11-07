# Bundled Cygwin runtime

`cygwin64` contains the Windows Cygwin runtime bundled with terminalpp. The
Windows build copies it beside `terminalpp.exe` as `cygwin64`; terminal child
processes receive Windows-compatible aliases from `cygwin64/compat` before
`cygwin64/bin` in their `PATH`.

The build restores Cygwin's `SYSTEM` attribute on symlink files, which Git does
not preserve, then generates `.exe` aliases for executable symlinks in `bin`.
This lets PowerShell and Command Prompt invoke those commands while leaving
the original Cygwin symlinks intact.

The local `home`, `tmp`, and `dev` contents and the machine-specific
`/etc/hosts` file are not included. The build creates empty `home`, `tmp`, and
`dev` directories in the copied runtime; `/etc/hosts` contains only standard
localhost entries.

The Windows build uses Robocopy to sync the large runtime incrementally and
regenerates the compatibility aliases when building terminalpp.
