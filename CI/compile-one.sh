#!/bin/bash
#
# Compile a single source file out of a target, instead of the whole library.
#
# During a port most of the work is one file at a time, and a full
# "cmake --build --target openmw-lib" is minutes per attempt. MSBuild's
# SelectedFiles property compiles just the named file against the target's real
# include paths and flags, which turns that into about five seconds.
#
# It is also MORE informative than the full build: MSVC caps reported errors at
# 100 per translation unit, so in a full build a heavily-broken file shows only
# its first hundred and hides the rest. One file at a time, you see them all.
#
# Usage: CI/compile-one.sh <target-vcxproj-relative> <source file> [<source file>...]
#   CI/compile-one.sh apps/openmw/openmw-lib.vcxproj apps/openmw/mwmp/LocalPlayer.cpp
#
# BUILD_DIR defaults to ../build-port.

set -uo pipefail
BUILD_DIR="${BUILD_DIR:-$(cd "$(dirname "$0")/../.." && pwd)/build-port}"
SRC_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PROJ="${1:?usage: $0 <vcxproj> <source>...}"; shift

MSBUILD="/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe"
[ -x "$MSBUILD" ] || MSBUILD="$(command -v msbuild.exe)"

# SelectedFiles wants Windows paths, semicolon-separated.
files=""
for f in "$@"; do
    win="$(cygpath -w "$SRC_ROOT/$f" 2>/dev/null || echo "$SRC_ROOT/$f")"
    files="${files:+$files;}$win"
done

"$MSBUILD" "$BUILD_DIR/$PROJ" -p:Configuration=Release -p:SelectedFiles="$files" -v:m -nologo 2>&1 \
  | grep -E ': (error|fatal error)' \
  | sed 's/ \[C:.*//' \
  | sort -u -t'(' -k2 -n
