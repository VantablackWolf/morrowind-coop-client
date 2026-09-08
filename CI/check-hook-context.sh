#!/bin/bash
#
# Find hooks whose surrounding code changed -- the signature of a hook that the
# merge dropped in the wrong place.
#
# This is the check the 0.51 port needed most and did not have. Almost every
# runtime defect found by playing the game was a hook that survived intact,
# compiled cleanly, and simply ended up next to the wrong statement:
#
#   scene.cpp        the cell-load report landed inside a lambda that had
#                    already returned. Unreachable. The server never learned
#                    the player had loaded any cell, so nothing in the world
#                    could be activated.
#   messagebox.cpp   mHasServerOrigin was set one statement too early, before
#                    the object it writes to is constructed. Null dereference
#                    on the first interactive message box of a session.
#   miscextensions   OpDisable's hook moved into a new branch and took a
#                    shadowing declaration with it.
#   statsextensions  four faction hooks ended up before the rank change
#                    instead of after, so every promotion packet carried the
#                    old rank.
#   enchantingdialog, recharge, repair
#                    success and failure sound hooks both stacked in the
#                    success branch.
#
# None of these produce a diagnostic. Braces stay balanced, hook counts are
# unchanged, and CI/check-resolution.sh passes. What does change is the code
# immediately around the hook, and that is what this compares.
#
# For every hook in the previous TES3MP tree it records the nearest real
# statement before and after the block, then finds the same hook here by its
# description text and compares. Upstream reformats and renames constantly, so
# the comparison is fuzzy: only genuinely different neighbours are reported,
# not rewordings of the same call.
#
# Exits 0 always -- a lead generator. Some hooks move for good reason, and a
# check that fails the build on those would just get skipped.
#
# Usage: CI/check-hook-context.sh <path-to-previous-tes3mp> [paths...]

set -uo pipefail
cd "$(dirname "$0")/.."
PREV="${1:?usage: $0 <path-to-previous-tes3mp> [paths...]}"
shift
# NB: ("${@:-apps components}") would collapse the default into ONE array
# element, "apps components", which is not a directory -- the scan then walks
# nothing and reports a clean tree. Set the default as a real two-element array.
if [ $# -eq 0 ]; then paths=(apps components); else paths=("$@"); fi

PREV="$PREV" python - "${paths[@]}" <<'PY'
import os, re, sys, difflib

prev_root = os.environ['PREV']
paths = sys.argv[1:]

START = 'Start of tes3mp'
END = 'End of tes3mp'

# Lines that are not "the statement next to the hook": comments, braces on
# their own, blanks, and the hook markers themselves.
NOISE = re.compile(r'^\s*(//|/\*|\*|\*/|\{|\}|\)|$)')


def code_lines(lines):
    """Index -> is this line a real statement?"""
    out = []
    in_comment = False
    for line in lines:
        stripped = line.strip()
        real = True
        if in_comment:
            real = False
            if '*/' in stripped:
                in_comment = False
        elif stripped.startswith('/*'):
            real = False
            if '*/' not in stripped:
                in_comment = True
        elif NOISE.match(line):
            real = False
        out.append(real)
    return out


def hooks(path):
    """(description, previous statement, next statement) for each hook block."""
    try:
        lines = open(path, encoding='utf8', errors='replace').read().splitlines()
    except OSError:
        return []
    if not any(START in l for l in lines):
        return []

    real = code_lines(lines)
    found = []
    i = 0
    while i < len(lines):
        if START not in lines[i]:
            i += 1
            continue
        start = i
        # The description is the first non-empty line after the marker.
        desc = ''
        for j in range(i + 1, min(i + 6, len(lines))):
            t = lines[j].strip()
            if t and not t.startswith(('*/', '/*')):
                desc = t
                break
        # Find the matching End marker.
        end = start
        for j in range(start + 1, len(lines)):
            if END in lines[j]:
                end = j
                break
        else:
            end = start

        before = ''
        for j in range(start - 1, max(-1, start - 40), -1):
            if real[j]:
                before = lines[j].strip()
                break
        after = ''
        for j in range(end + 1, min(len(lines), end + 40)):
            if real[j]:
                after = lines[j].strip()
                break

        if desc:
            found.append((desc, before, after))
        i = end + 1
    return found


def norm(s):
    """Compare on identifiers, not punctuation or spelling of access."""
    s = re.sub(r'\s+', '', s)
    s = s.replace('->', '.').replace('::', '.')
    return s


def similar(a, b):
    if not a and not b:
        return 1.0
    if not a or not b:
        return 0.0
    return difflib.SequenceMatcher(None, norm(a), norm(b)).ratio()


# Index the previous tree by description, so a hook is still found after the
# file it lives in has been moved or renamed upstream.
prev_index = {}
for dirpath, dirnames, filenames in os.walk(prev_root):
    dirnames[:] = [d for d in dirnames if d != '.git']
    for fn in filenames:
        if fn.endswith(('.cpp', '.hpp', '.h')):
            for desc, before, after in hooks(os.path.join(dirpath, fn)):
                prev_index.setdefault(desc[:60], []).append((before, after))

leads = []
checked = 0
for root in paths:
    for dirpath, dirnames, filenames in os.walk(root):
        dirnames[:] = [d for d in dirnames if d != '.git']
        for fn in sorted(filenames):
            if not fn.endswith(('.cpp', '.hpp', '.h')):
                continue
            path = os.path.join(dirpath, fn).replace(os.sep, '/')
            for desc, before, after in hooks(path):
                key = desc[:60]
                if key not in prev_index:
                    continue
                # A description may appear more than once; the hook is fine if
                # it matches ANY of them.
                best = 0.0
                best_pair = None
                for pb, pa in prev_index[key]:
                    score = (similar(before, pb) + similar(after, pa)) / 2
                    if score > best:
                        best, best_pair = score, (pb, pa)
                checked += 1
                if best < 0.55 and best_pair is not None:
                    leads.append((best, path, desc, best_pair, before, after))

# Ranked, worst first. Upstream renames enough that a flat list of every
# difference is unreadable and therefore unread; the hooks whose surroundings
# changed MOST are where a relocation hides.
leads.sort(key=lambda x: x[0])
LIMIT = int(os.environ.get('HOOK_CONTEXT_LIMIT', '25'))
for best, path, desc, pair, before, after in leads[:LIMIT]:
    print("   %s   [similarity %.2f]" % (path, best))
    print("     hook: %s" % desc[:70])
    print("     was:  ...%s / %s..." % (pair[0][:58], pair[1][:58]))
    print("     now:  ...%s / %s..." % (before[:58], after[:58]))
    print()
hits = len(leads)
if hits > LIMIT:
    print("   ... and %d more, least suspicious first. HOOK_CONTEXT_LIMIT=0 shows all." % (hits - LIMIT))
    print()

print("%d of %d matched hooks sit next to different code than before." % (hits, checked))
print("Each is a LEAD. Upstream restructures constantly and a hook often has to")
print("move with it. What matters is whether it still runs at the right moment:")
print("before or after the statement it is meant to observe, and on every path")
print("that statement runs on.")
PY
exit 0
