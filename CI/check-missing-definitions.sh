#!/bin/bash
#
# Find function definitions that exist upstream but have vanished from the port.
#
# On the 0.47 -> 0.51 port a "change (major)" hook opened a block comment to
# suppress an upstream call and never closed it. The comment ran on for 330 lines
# and swallowed every function definition after it -- teleportToClosestMarker,
# getPlayerPtr, updateWeather, goToJail, spawnRandomCreature and more.
#
# Nothing caught it. Braces balanced, because they were inside a comment. Hook
# counts were unchanged. It compiled. The symbols would have gone missing at link
# time, and the compiler's own errors pointed 300 lines past the cause, at the
# first place it noticed the namespace had closed.
#
# A block comment cannot nest inside the tes3mp hook markers, which are
# themselves block comments -- so suppressed code must be line-commented. This
# check exists because that is easy to get wrong and invisible when you do.
#
# Usage: CI/check-missing-definitions.sh <path-to-upstream-openmw> [file...]
#   CI/check-missing-definitions.sh ../openmw-openmw-51

set -uo pipefail
UP="${1:?usage: $0 <path-to-upstream-openmw> [file...]}"
shift
cd "$(dirname "$0")/.."

# Comments MUST be stripped first. The whole point of this check is a definition
# that is still textually present but sits inside a runaway comment -- grep alone
# sees it and reports nothing, which is the trap being checked for.
strip_comments() {
    awk '
        BEGIN { inblock = 0 }
        {
            line = $0; out = ""; i = 1; n = length(line)
            while (i <= n) {
                c = substr(line, i, 1); d = substr(line, i, 2)
                if (inblock) {
                    if (d == "*/") { inblock = 0; i += 2 } else { i++ }
                } else if (d == "/*") {
                    inblock = 1; i += 2
                } else if (d == "//") {
                    break
                } else {
                    out = out c; i++
                }
            }
            print out
        }
    ' "$1"
}

# Anchored to the start of a line and preceded by a return type, so this matches
# DEFINITIONS and not the far more numerous call sites. Deliberately crude: it
# only has to be consistent between the two trees, not to parse C++.
defs() {
    strip_comments "$1" \
      | grep -oE '^[[:space:]]*[A-Za-z_][A-Za-z0-9_:<>&*, ]*[[:space:]&*]+[A-Za-z_][A-Za-z0-9_]*::[~A-Za-z_][A-Za-z0-9_]*[[:space:]]*[(]' \
      | grep -oE '[A-Za-z_][A-Za-z0-9_]*::[~A-Za-z_][A-Za-z0-9_]*[[:space:]]*[(]$' \
      | tr -d ' ' | sort -u
}

if [ "$#" -gt 0 ]; then
    files=("$@")
else
    mapfile -t files < <(cd "$UP" && find apps components -name '*.cpp' 2>/dev/null)
fi

status=0
for f in "${files[@]}"; do
    [ -f "$UP/$f" ] || continue
    [ -f "$f" ] || continue

    missing=$(comm -23 <(defs "$UP/$f") <(defs "$f"))
    if [ -n "$missing" ]; then
        echo "== $f"
        printf '   missing: %s\n' $missing
        status=1
    fi
done

if [ "$status" -ne 0 ]; then
    cat <<'NOTE'

A definition present upstream and absent here is either a deliberate tes3mp
removal -- in which case say so in a hook comment -- or something swallowed it.
Check for an unterminated block comment above the first missing one.
NOTE
fi

exit $status
