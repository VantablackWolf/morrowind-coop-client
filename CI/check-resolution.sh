#!/bin/bash
#
# Sanity-check a file after resolving merge conflicts in it.
#
# Two mistakes cost real time on the 0.51 port, both of which compile cleanly:
#
#   1. Resolving a "change (major)" hunk by pasting upstream's whole function
#      back in restores the call tes3mp deliberately suppressed. The client then
#      acts unilaterally instead of waiting for the server, which shows up only
#      as desync during play.
#
#   2. These hunks often start partway into a class body, after "public:" and
#      the execute() signature, so pasting a complete replacement duplicates the
#      prefix and leaves the file a brace open -- reported far from the cause.
#
# Usage: CI/check-resolution.sh <file> [<file>...]

set -uo pipefail
status=0

for f in "$@"; do
    [ -f "$f" ] || { echo "$f: missing"; status=1; continue; }
    echo "== $f"

    if grep -q '<<<<<<<\|>>>>>>>' "$f"; then
        echo "   FAIL: conflict markers still present"
        status=1
    fi

    # Brace balance, ignoring // comments. Crude, but it catches duplicated
    # function prefixes, which is what it is for.
    bal=$(sed 's://.*::' "$f" | tr -cd '{}' | awk '{
        n=0
        for (i=1; i<=length($0); i++) n += (substr($0,i,1)=="{") ? 1 : -1
        print n
    }')
    if [ "${bal:-0}" != "0" ]; then
        echo "   FAIL: brace balance ${bal} (expected 0) -- likely a duplicated function prefix"
        status=1
    fi

    # Every call a change (major) hook commented out must still be commented out.
    # Look for the same call appearing live elsewhere in the file.
    while IFS= read -r line; do
        call=$(printf '%s' "$line" | sed 's:^[[:space:]]*//[[:space:]]*::; s:[[:space:]]*$::')
        [ -n "$call" ] || continue
        case "$call" in
            \**|/\**|Start*|End*|"") continue ;;
        esac
        live=$(grep -F -- "$call" "$f" | grep -v '^[[:space:]]*//' | wc -l)
        if [ "$live" -gt 0 ]; then
            echo "   WARN: suppressed call appears live elsewhere: ${call}"
            echo "         (fine if upstream legitimately calls it in another opcode -- check)"
        fi
    done < <(awk '/Start of tes3mp change \(major\)/,/End of tes3mp change \(major\)/' "$f" \
             | grep '^[[:space:]]*//[[:space:]]*[A-Za-z_]' )

    hooks=$(grep -c 'Start of tes3mp' "$f" || true)
    echo "   ok: braces balanced, ${hooks} hook blocks present"
done

exit $status
