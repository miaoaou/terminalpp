# Bundled Cygwin runtime

`cygwin64` contains the Windows Cygwin runtime bundled with terminalpp. The
Windows build copies it beside `terminalpp.exe` as `cygwin64`, and terminal
child processes receive `cygwin64/bin` at the front of their `PATH`.

The local `home`, `tmp`, and `dev` contents and the machine-specific
`/etc/hosts` file are not included. The build creates empty `home`, `tmp`, and
`dev` directories in the copied runtime; `/etc/hosts` contains only standard
localhost entries.

When replacing the bundled runtime, remove
`<build output>/cygwin64/.terminalpp-cygwin-runtime` so the next build copies
the updated files.
