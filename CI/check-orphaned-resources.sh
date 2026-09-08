#!/bin/bash
#
# Find files under files/ that no build file mentions.
#
# The port's asset equivalent of a stranded hook, and it cost a working client
# once already. 0.51 replaced files/mygui/CMakeLists.txt with
# files/data/CMakeLists.txt. The merge moved TES3MP's six MyGUI layouts to the
# new directory but the entries listing them had nowhere to land, so they sat on
# disk referenced by nothing. The client built, linked, connected, logged in,
# and then died on "Resource 'mygui/tes3mp_chat.skin.xml' not found" the first
# time it drew the chat window.
#
# Note why CI/audit-port.sh could not have caught this even with its file
# filter widened to CMake files: it looks for tes3mp hook markers, and in 0.8.1
# only ONE of those seven entries (tes3mp_logo.png) was inside a marked block.
# The six layouts were bare lines in a list. Nothing marked them as ours, so
# nothing could notice they had gone missing. What identifies them is not who
# added them but that nothing builds them.
#
# The question this asks is deliberately dumber and therefore more general:
# here is a file in the source tree; does any CMakeLists.txt or .cmake file
# anywhere mention it? A resource nothing references is either dead weight or a
# casualty of the merge, and both are worth a look.
#
# Exits 0 always. Some hits are legitimate -- documentation, files consumed by
# a wildcard, platform assets for a platform you are not building. This is a
# lead generator, like check-hook-depth.sh and check-hook-shadowing.sh.
#
# Usage: CI/check-orphaned-resources.sh [dir]   (default: files)

set -uo pipefail
cd "$(dirname "$0")/.."
ROOT="${1:-files}"

python - "$ROOT" <<'PY'
import os, sys

root = sys.argv[1]

# Everything any build file says, as one haystack. Cheap and good enough: build
# files name resources by path or by basename, and both survive this.
haystack = []
for dirpath, dirnames, filenames in os.walk('.'):
    dirnames[:] = [d for d in dirnames if d not in ('.git', 'build', 'build-port')]
    for fn in filenames:
        if fn == 'CMakeLists.txt' or fn.endswith(('.cmake', '.qrc')):
            try:
                haystack.append(open(os.path.join(dirpath, fn),
                                     encoding='utf8', errors='replace').read())
            except OSError:
                pass
blob = '\n'.join(haystack)

# Extensions worth asking about. Source and build files are not resources, and
# docs are noise.
INTERESTING = ('.layout', '.xml', '.skin.xml', '.png', '.dds', '.omwfx', '.glsl',
               '.shader', '.yaml', '.json', '.cfg', '.ui', '.rc', '.ico', '.qrc',
               '.omwscripts', '.lua', '.fnt', '.omwfont', '.ttf')

orphans = []
for dirpath, dirnames, filenames in os.walk(root):
    dirnames[:] = [d for d in dirnames if d != '.git']
    for fn in sorted(filenames):
        if not fn.endswith(INTERESTING):
            continue
        path = os.path.join(dirpath, fn).replace(os.sep, '/')
        rel = path[len(root) + 1:] if path.startswith(root + '/') else path
        # A build file may name it by full path, by path relative to files/, or
        # by basename. Any of those counts as referenced.
        if fn in blob or rel in blob or path in blob:
            continue
        orphans.append(path)

for p in orphans:
    print("   %s" % p)

print()
print("%d file(s) under %s/ that no CMakeLists.txt or .cmake mentions." % (len(orphans), root))
print("Each needs a look: is it installed by a wildcard, is it dead, or did the")
print("line that used to list it go missing when upstream restructured the build?")
PY
exit 0
