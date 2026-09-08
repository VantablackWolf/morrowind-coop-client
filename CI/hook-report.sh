#!/bin/bash
#
# Report TES3MP's hook surface in the OpenMW engine.
#
# Every hook block patched into an upstream file is a merge conflict waiting to
# happen on the next OpenMW release. This number IS the porting cost, so it is
# worth watching: if it grows, the next port gets more expensive.
#
# Usage: CI/hook-report.sh [--files] [--major]
#
#   --files   list every engine file carrying hooks, worst first
#   --major   list only "change (major)" hooks -- the expensive kind

set -euo pipefail
cd "$(dirname "$0")/.."

# TES3MP's own code is not a hook surface; only patches into upstream files count.
EXCLUDE='apps/openmw/mwmp|apps/openmw-mp|components/openmw-mp|apps/browser|apps/master'

hook_files() {
    grep -rl 'Start of tes3mp' --include='*.cpp' --include='*.hpp' --include='*.h' \
        apps components 2>/dev/null | grep -Ev "$EXCLUDE" | sort
}

total=0; files=0
for f in $(hook_files); do
    n=$(grep -c 'Start of tes3mp' "$f")
    total=$((total + n)); files=$((files + 1))
done

echo "TES3MP hook surface"
echo "==================="
echo "engine files patched : ${files}"
echo "hook blocks          : ${total}"
echo
for kind in 'addition' 'change (major)' 'change (minor)'; do
    n=$(grep -rho "Start of tes3mp ${kind}" --include='*.cpp' --include='*.hpp' --include='*.h' \
        apps components 2>/dev/null | grep -Ev "$EXCLUDE" | wc -l || true)
    printf "  %-16s %s\n" "${kind}" "${n}"
done

cat <<'NOTE'

Reading this: "addition" hooks are usually a line or two calling into mwmp and
re-apply almost for free. "change (major)" hooks rewrite upstream logic, so they
conflict hard and need real judgement every release. Driving that second number
down -- by moving logic into apps/openmw/mwmp and leaving only a thin call at
the hook site -- is the single most effective thing that makes the next port
cheaper. See PORTING.md.
NOTE

if [ "${1:-}" = "--files" ]; then
    echo
    echo "Files by hook count:"
    for f in $(hook_files); do
        printf "%4d  %s\n" "$(grep -c 'Start of tes3mp' "$f")" "$f"
    done | sort -rn
fi

if [ "${1:-}" = "--major" ]; then
    echo
    echo "change (major) hooks -- the expensive ones:"
    for f in $(hook_files); do
        n=$(grep -c 'Start of tes3mp change (major)' "$f" || true)
        [ "$n" -gt 0 ] && printf "%4d  %s\n" "$n" "$f"
    done | sort -rn
fi
