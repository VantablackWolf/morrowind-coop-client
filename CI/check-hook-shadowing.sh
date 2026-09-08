#!/bin/bash
#
# Find variables re-declared around a tes3mp hook that shadow one already
# declared in an enclosing scope of the same function.
#
# This is the merge's nastiest failure mode. When upstream restructures a
# function -- hoisting a declaration above a new if/else, say -- and the merge
# drops the hook into one of the new branches, the declaration the hook needs
# gets re-created inside that branch. Everything still compiles: both variables
# are well-formed, the types match, neither is unused. But the branch now
# operates on its private copy and the outer variable keeps its initial value,
# which for an MWWorld::Ptr means empty.
#
# Two of these shipped in the 0.51 port before this check existed:
#
#   summoning.cpp    -- the spawn hook was nested inside "if (anim)" and
#                       redeclared `placed`, shadowing the real creature.
#   miscextensions.cpp OpDisable
#                    -- 0.51 split the opcode into explicit/implicit branches
#                       and hoisted `MWWorld::Ptr ptr` above them. The hook
#                       landed in the explicit branch behind its own `ptr`, so
#                       the trailing disable(ptr) got an empty Ptr and
#                       segfaulted on the first global script of a new game.
#
# Neither produced a compiler warning at /W3, and neither is visible in a
# per-file conflict review: the hook body is character-for-character what it was
# in 0.8.1. Only its scope changed.
#
# Note the criterion is NOT "a shadow inside a hook". In both real cases the
# shadowing declaration sits just OUTSIDE the hook markers -- it is the line the
# merge added to make the relocated hook compile. What identifies it is that the
# block it opens contains a hook.
#
# Exits 0 always -- like check-hook-depth.sh this is a lead generator. Shadowing
# is legal and occasionally deliberate; the point is to put the handful of
# candidates in front of a human.
#
# Usage: CI/check-hook-shadowing.sh [paths...]   (default: apps components)

set -uo pipefail
cd "$(dirname "$0")/.."
paths=("${@:-apps components}")

python - "${paths[@]}" <<'PY'
import os, re, sys

# "Type name = ..." / "Type name(...)" -- deliberately conservative. Requires a
# capitalised or namespaced type so plain assignments and control flow are not
# mistaken for declarations.
DECL = re.compile(
    r'^\s*(?:const\s+)?((?:[A-Za-z_]\w*::)*[A-Z]\w*(?:\s*<[^;{}]*>)?)\s*[*&]?\s*'
    # ";" matters as much as "=" here: the variable a relocated hook shadows is
    # very often the bare "MWWorld::Ptr ptr;" that upstream hoisted above the
    # branches precisely so both could assign to it.
    r'([a-z_]\w*)\s*(?:=[^=]|\(|;)')

KEYWORDS = {'if', 'for', 'while', 'switch', 'return', 'catch', 'else', 'do'}


def scan(lines):
    """Shadowing declarations whose block also contains a tes3mp hook."""
    depth = 0
    scope = {}   # depth -> {name: line}
    pending = []  # shadows whose block has not closed yet
    found = []

    for n, line in enumerate(lines, 1):
        code = re.sub(r'//.*', '', line)

        if 'Start of tes3mp' in line:
            # Every still-open shadow provably contains a hook.
            for p in pending:
                p['hook'] = True

        m = DECL.match(code)
        if m and code.split()[0] not in KEYWORDS:
            name = m.group(2)
            if name not in KEYWORDS:
                outer = None
                for d in range(depth):
                    if name in scope.get(d, {}):
                        outer = scope[d][name]
                if outer is not None:
                    pending.append({'line': n, 'type': m.group(1), 'name': name,
                                    'outer': outer, 'depth': depth, 'hook': False})
                scope.setdefault(depth, {})[name] = n

        for ch in code:
            if ch == '{':
                depth += 1
            elif ch == '}':
                scope.pop(depth, None)
                depth = max(0, depth - 1)
                # Shadows declared at a depth we have now left are resolved.
                for p in [q for q in pending if q['depth'] > depth]:
                    if p['hook']:
                        found.append(p)
                    pending.remove(p)
                if depth == 0:
                    scope.clear()
                    del pending[:]
    return found


hits = 0
for root in sys.argv[1:]:
    for dirpath, _, files in os.walk(root):
        for fn in sorted(files):
            if not fn.endswith(('.cpp', '.hpp', '.h')):
                continue
            path = os.path.join(dirpath, fn).replace(os.sep, '/')
            try:
                lines = open(path, encoding='utf8', errors='replace').read().splitlines()
            except OSError:
                continue
            if not any('Start of tes3mp' in l for l in lines):
                continue
            for p in scan(lines):
                print("   %s:%d" % (path, p['line']))
                print("     '%s %s' shadows the one declared at line %d,"
                      % (p['type'], p['name'], p['outer']))
                print("     and a tes3mp hook sits inside the block it opens.")
                hits += 1

print()
print("%d shadowed declaration(s) around tes3mp hooks." % hits)
print("Each needs a look: does the block mean to work on its own copy, or on the")
print("variable the surrounding function goes on to use?")
PY
exit 0
