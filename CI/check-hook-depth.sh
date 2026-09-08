#!/bin/bash
#
# Find hooks the merge relocated INTO a deeper scope than they belong in.
#
# This was the most expensive failure mode of the 0.47 -> 0.51 port, and the one
# no other check here sees. When upstream restructures a region, git can place a
# tes3mp block inside a function body rather than beside it. The result:
#
#   - braces stay balanced, so CI/check-resolution.sh passes
#   - the file still exists with all its hooks, so CI/audit-port.sh passes
#   - the hook count is unchanged, so the lost-hook check passes
#
# and the only symptom is an error in a DIFFERENT file saying the member does not
# exist. It happened four times: CellStore's six accessors landed inside
# forEach(), ContainerStore::setResolved outside its class entirely,
# ActiveSpells' three hooks inside unloadActor(), and
# CreatureStats::setSummonedCreatureActorId inside updateAwareness().
#
# The signal is brace depth. A declaration that sat at depth 1 (class body) in the
# previous tree and now sits at depth 3 has moved into a function.
#
# Usage: CI/check-hook-depth.sh <path-to-previous-tes3mp> [file...]
#   CI/check-hook-depth.sh ../TES3MP-0.8.1
#
# With no file list, every file that has hooks in both trees is checked.

set -uo pipefail
PREV="${1:?usage: $0 <path-to-previous-tes3mp> [file...]}"
shift
cd "$(dirname "$0")/.."

depths() {
    # For each "Start of tes3mp" line, print its brace depth.
    # Comments are stripped so braces inside them do not count.
    awk '
        { line = $0
          gsub(/\/\/.*/, "", line)
          if (line ~ /Start of tes3mp/) print depth
          n = gsub(/\{/, "{", line); depth += n
          n = gsub(/\}/, "}", line); depth -= n
        }
    ' "$1"
}

if [ "$#" -gt 0 ]; then
    files=("$@")
else
    mapfile -t files < <(grep -rl 'Start of tes3mp' --include='*.cpp' --include='*.hpp' --include='*.h' apps components 2>/dev/null)
fi

status=0
for f in "${files[@]}"; do
    [ -f "$PREV/$f" ] || continue

    mapfile -t was < <(depths "$PREV/$f")
    mapfile -t now < <(depths "$f")

    [ "${#was[@]}" -eq "${#now[@]}" ] || continue   # hook count differs; that is check-resolution's job

    for i in "${!was[@]}"; do
        # Two signals, both learned from real damage on the 0.51 port:
        #
        #   a hook that was at class or namespace scope (depth <= 1) and is now deeper
        #   -- that is where DECLARATIONS live, and moving one inside a function is the
        #   failure that costs the most to find; and
        #
        #   any jump of two or more levels.
        #
        # A plain +1 from depth 2 or more is usually upstream nesting the code the hook
        # wraps -- 0.51 did that to most of mwscript at once -- so it is not reported.
        jump=$(( now[i] - was[i] ))
        if { [ "${was[$i]}" -le 1 ] && [ "$jump" -gt 0 ]; } || [ "$jump" -ge 2 ]; then
            echo "$f: hook #$((i + 1)) is at brace depth ${now[$i]}, was ${was[$i]}"
            grep -n 'Start of tes3mp' "$f" | sed -n "$((i + 1))p" | sed 's/^/    now: /'
            status=1
        fi
    done
done

if [ "$status" -ne 0 ]; then
    echo
    echo "A hook that got DEEPER has probably been pulled inside a function body."
    echo "Read the surrounding code before assuming it is a false positive: a few"
    echo "are legitimate (upstream genuinely nested the code the hook wraps)."
fi

exit $status
