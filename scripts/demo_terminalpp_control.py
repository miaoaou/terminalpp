"""Demo for reading terminal++ tabs and sending text to a tab."""

import argparse
import subprocess

import terminalpp_control as terminal


def run_demo(args, parser):
    if args.send_tab is not None:
        if args.text is None:
            parser.error("--send-tab requires --text")
        tabs = terminal.read_all_tabs()
        try:
            tab_id = int(args.send_tab)
        except ValueError:
            requested_title = args.send_tab.casefold()
            match = next((
                tab for tab in tabs
                if tab["title"].casefold() == requested_title
                or tab["terminal_title"].casefold() == requested_title
            ), None)
            if match is None:
                available = ", ".join("{} ({})".format(tab["title"], tab["id"]) for tab in tabs)
                parser.error("unknown tab title {!r}; available tabs: {}".format(args.send_tab, available or "none"))
            tab_id = match["id"]
        result = terminal.send_text(tab_id, args.text, enter=args.enter)
        print("Sent {} UTF-8 bytes to tab {} (Enter: {}).".format(
            result["bytes"], result["id"], result["enter"]
        ))
        return

    if args.text is not None or args.enter:
        parser.error("--text and --enter require --send-tab")

    tabs = terminal.read_all_tabs()
    if not tabs:
        print("No open tabs were returned by terminal++.")
        return

    for tab in tabs:
        marker = " [active]" if tab["active"] else ""
        print("\n=== Tab {}: {}{} ===".format(tab["id"], tab["title"], marker))
        content = tab["text"].rstrip("\n")
        print(content if content else "(empty)")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--send-tab", help="send text to a zero-based tab id or title such as Tab1")
    parser.add_argument("--text", help="text to send")
    parser.add_argument("--enter", action="store_true", help="send Enter after the text")
    args = parser.parse_args()
    try:
        run_demo(args, parser)
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        parser.exit(1, "terminal++ control error: {}\n".format(error))


if __name__ == "__main__":
    main()
