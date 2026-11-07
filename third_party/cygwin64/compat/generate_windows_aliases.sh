#!/usr/bin/bash
set -euo pipefail

bin=/usr/bin
compat=/compat
manifest="$compat/.generated-aliases"
next_manifest="$compat/.generated-aliases.$$"

if [[ -f "$manifest" ]]; then
    while IFS= read -r alias; do
        [[ -n "$alias" ]] && rm -f -- "$compat/$alias"
    done < "$manifest"
fi

: > "$next_manifest"
alias_count=0

while IFS= read -r -d '' link; do
    name=${link##*/}
    resolved=$(readlink -f -- "$link" 2>/dev/null) || continue

    if [[ "$resolved" == *.exe ]]; then
        target=$resolved
    elif [[ -f "${resolved}.exe" ]]; then
        target="${resolved}.exe"
    else
        continue
    fi

    [[ -f "$target" ]] || continue

    if [[ "$name" == *.exe ]]; then
        alias=$name
    else
        alias="${name}.exe"
    fi

    destination="$compat/$alias"
    if ! ln -f -- "$target" "$destination" 2>/dev/null; then
        cp -pf -- "$target" "$destination"
    fi

    printf '%s\n' "$alias" >> "$next_manifest"
    alias_count=$((alias_count + 1))
done < <(find "$bin" -mindepth 1 -maxdepth 1 -type l -print0)

mv -f -- "$next_manifest" "$manifest"
: > "$compat/.terminalpp-cygwin-compat-ready"
printf 'Generated %s Windows aliases for Cygwin executable symlinks.\n' "$alias_count"
