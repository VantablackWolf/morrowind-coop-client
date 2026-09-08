#!/bin/bash
#
# Set up a 3-way merge of TES3MP onto a new OpenMW release.
#
# TES3MP is a fork, not a plugin: it patches ~730 hook blocks into ~160 upstream
# engine files. Porting to a new OpenMW is therefore a merge, and the only way
# to make it tractable is to give git a real merge base so it can auto-resolve
# everything upstream touched that TES3MP did not.
#
# Usage:
#   CI/port-to-openmw.sh <old-openmw-tag> <new-openmw-tag> [workdir]
#
# Example, porting the 0.51-based tree to 0.52:
#   CI/port-to-openmw.sh openmw-0.51.0 openmw-0.52.0 ../tes3mp-0.52-port
#
# The <old-openmw-tag> MUST be the OpenMW release this tree is currently based
# on -- see resources/version, or the OPENMW_VERSION_* values in CMakeLists.txt.
# Getting it wrong produces a merge base that is subtly incorrect and thousands
# of spurious conflicts.

set -euo pipefail

OLD_TAG="${1:?usage: $0 <old-openmw-tag> <new-openmw-tag> [workdir]}"
NEW_TAG="${2:?usage: $0 <old-openmw-tag> <new-openmw-tag> [workdir]}"
WORKDIR="${3:-../tes3mp-port-${NEW_TAG}}"
UPSTREAM="${OPENMW_REMOTE:-https://gitlab.com/OpenMW/openmw.git}"

TES3MP_SRC="$(cd "$(dirname "$0")/.." && pwd)"

echo "TES3MP source : ${TES3MP_SRC}"
echo "merge base    : ${OLD_TAG}"
echo "target        : ${NEW_TAG}"
echo "work dir      : ${WORKDIR}"
echo

# Line endings matter more than they look. A tree checked out on Windows with
# core.autocrlf=true is CRLF, while release source archives are LF; mixing them
# makes git treat EVERY file as modified on both sides and turns a ~570-hunk
# merge into a ~1800-file one. Normalise all three trees to LF before comparing.
normalise_lf() {
    find "$1" -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.c' \
        -o -name '*.txt' -o -name '*.cmake' -o -name '*.md' -o -name '*.lua' \
        -o -name '*.yaml' -o -name '*.yml' -o -name '*.qrc' -o -name '*.ui' \) \
        -not -path '*/.git/*' -print0 | xargs -0 -r sed -i 's/\r$//'
}

rm -rf "${WORKDIR}"
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

git init -q -b base .
git config core.autocrlf false
git config merge.renameLimit 999999
git config diff.renameLimit 999999

echo "==> fetching ${OLD_TAG} (merge base)"
git remote add upstream "${UPSTREAM}"
git fetch --depth 1 -q upstream "refs/tags/${OLD_TAG}"
git checkout -q FETCH_HEAD -- .
normalise_lf .
git add -A && git commit -q -m "OpenMW ${OLD_TAG} (merge base)"
git tag base-openmw

echo "==> importing TES3MP"
git checkout -q -b tes3mp
find . -mindepth 1 -maxdepth 1 ! -name .git -exec rm -rf {} +
cp -r "${TES3MP_SRC}"/. .
rm -rf .git/modules 2>/dev/null || true
normalise_lf .
git add -A && git commit -q -m "TES3MP on ${OLD_TAG}"

echo "==> fetching ${NEW_TAG}"
git checkout -q -b upstream-new base-openmw
git fetch --depth 1 -q upstream "refs/tags/${NEW_TAG}"
git checkout -q FETCH_HEAD -- .
normalise_lf .
git add -A && git commit -q -m "OpenMW ${NEW_TAG}"

echo "==> merging"
git checkout -q tes3mp
set +e
git merge upstream-new --no-commit
set -e

echo
echo "================ PORT SUMMARY ================"
git status --porcelain | awk '{print $1}' | sort | uniq -c | sort -rn
echo
echo -n "conflict hunks: "
grep -rl '<<<<<<<' --exclude-dir=.git . 2>/dev/null | xargs -r grep -c '<<<<<<<' \
    | awk -F: '{s+=$2} END {print s+0}'
echo
echo "Next steps:"
echo "  1. CI/hook-report.sh              -- see where the hooks are"
echo "  2. resolve build files first      -- nothing is verifiable until cmake configures"
echo "  3. read PORTING.md                -- resolution conventions and known traps"
