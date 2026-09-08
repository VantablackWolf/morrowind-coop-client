#!/bin/bash
#
# Whole-tree audit after a port. CI/check-resolution.sh checks one file at a
# time; these two problems are invisible at that level because the files
# involved never conflicted at all.
#
#   1. STALE FILES. When upstream deletes or moves a file, the merge keeps the
#      fork's copy. It compiles, it is never referenced, and it shadows nothing
#      obvious -- until one of them shadows a real header. On the 0.51 port,
#      cmake/FindSDL2.cmake shadowed the package config that exports
#      SDL2::SDL2 and broke the generate step three layers away.
#
#   2. STRANDED HOOKS. Worse: a hook living in a file upstream MOVED goes
#      nowhere. Nothing conflicts, nothing is reported, and the hook is simply
#      absent from the build. Seven were stranded this way on the 0.51 port,
#      including ESM::CellRef::mMpNum -- the multiplayer object identity used
#      in 23 files. The build would have failed eventually; the point is that
#      nothing in the merge pointed at the cause.
#
#      Build files count. 0.51 replaced files/mygui/CMakeLists.txt with
#      files/data/CMakeLists.txt, stranding the entry that installed
#      tes3mp_logo.png. Note the limit of this check, though: the six MyGUI
#      layouts stranded by the same edit were bare lines in a list with no
#      tes3mp markers around them, so nothing here can see them go. That is
#      what CI/check-orphaned-resources.sh is for -- it asks whether anything
#      builds a file rather than who added it.
#
# Usage: CI/audit-port.sh <old-openmw-ref> <new-openmw-ref> <path-to-previous-tes3mp>
#
# Example, after CI/port-to-openmw.sh:
#   CI/audit-port.sh v047 omw51 ../TES3MP-0.8.1

set -uo pipefail
OLD="${1:?usage: $0 <old-openmw-ref> <new-openmw-ref> <path-to-previous-tes3mp>}"
NEW="${2:?}"
PREV="${3:?}"
cd "$(dirname "$0")/.."

# Paths that belong to TES3MP itself and are expected to be absent upstream.
OWN='^(apps/openmw/mwmp|apps/openmw-mp|components/openmw-mp|apps/browser|apps/master|files/tes3mp|CI/)'

echo "== Stale files (upstream deleted them; the merge kept ours)"
stale=0
comm -12 <(git ls-tree -r --name-only "$OLD" | sort) <(git ls-files | sort) \
  | grep -Ev "$OWN" \
  | comm -23 - <(git ls-tree -r --name-only "$NEW" | sort) \
  | while read -r f; do echo "   $f"; done
echo

echo "== Stranded hooks (present in the previous TES3MP, absent from this tree)"
missing=0
for f in $(cd "$PREV" && grep -rl 'Start of tes3mp' --include='*.cpp' --include='*.hpp' --include='*.h' --include='CMakeLists.txt' --include='*.cmake' . 2>/dev/null | sed 's|^\./||'); do
    was=$(grep -c 'Start of tes3mp' "$PREV/$f")
    if [ ! -f "$f" ]; then
        # File is gone from this tree. Its hooks had to land somewhere else --
        # check whether each description still appears anywhere in the port.
        while IFS= read -r desc; do
            [ -z "$desc" ] && continue
            # Compare on a PREFIX, not the whole line. Upstream runs clang-format
            # over everything it touches, so a hook's comment gets re-wrapped at a
            # different column and an exact-line search reports a migrated hook as
            # stranded. On the 0.51 port that was two of three hits: the loadcell
            # getShortDescription hooks had moved to components/esm3/ intact, and
            # only "because it was widely" vs "because it was" separated them.
            probe="${desc:0:40}"
            if ! grep -rqF -- "$probe" apps components files cmake CMakeLists.txt 2>/dev/null; then
                echo "   STRANDED: $f"
                echo "             $desc"
                missing=$((missing + 1))
            fi
        done < <(grep -A3 'Start of tes3mp' "$PREV/$f" | grep -vE 'Start of tes3mp|^--|^\s*\*/|^\s*$' | sed 's/^[[:space:]]*//' | head -"$was")
    fi
done
echo
echo "Anything listed above needs a decision: migrate the hook to wherever"
echo "upstream moved the code, or delete it deliberately and say why."
