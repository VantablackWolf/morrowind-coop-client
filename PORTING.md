# Porting TES3MP to a new OpenMW release

TES3MP is a fork of the OpenMW engine, not a plugin. It patches roughly 730 hook
blocks into ~160 upstream engine files, plus ~590 files of its own. Moving to a
new OpenMW release is therefore a merge, and this document exists so the next
one costs days instead of months.

Run `CI/hook-report.sh` to see the current surface. That number is the porting
cost.

---

## The process

```bash
CI/port-to-openmw.sh openmw-0.51.0 openmw-0.52.0 ../tes3mp-0.52-port
```

That sets up a real 3-way merge — base = the OpenMW release this tree is
currently on, ours = TES3MP, theirs = the new release — so git auto-resolves
everything upstream touched that TES3MP did not. Roughly three quarters of
upstream's churn resolves itself this way; only the files TES3MP actually
patched come back as conflicts.

Then, in order:

1. **Build a pristine baseline of the NEW OpenMW first**, before touching the
   port. Twenty-five minutes of compiling buys you the ability to say "this
   failure is the port, not my toolchain" for the rest of the work. Without it
   every error is ambiguous.
2. **Resolve the build files before any source file.** Nothing about the port is
   verifiable until `cmake` configures — you cannot compile-check a single
   decision before that. It is four files and it unblocks everything.
3. **Then source conflicts**, biggest files first.
4. **Then run the round-trip tests** in `apps/openmw-mp/tests/`.

### Compile one file at a time

`CI/compile-one.sh apps/openmw/openmw-lib.vcxproj apps/openmw/mwmp/LocalPlayer.cpp`

About five seconds instead of minutes, and *more* informative than a full build:
MSVC caps reported errors at 100 per translation unit, so a heavily-broken file
shows only its first hundred in a full build and hides the rest. Adapting one
file at a time with a compile after each edit is the whole loop.

### The checks, and what each one cannot see

| Check | Finds | Blind to |
|---|---|---|
| `check-resolution.sh` | conflict markers, brace imbalance, lost hooks, restored suppressions | anything that keeps braces balanced |
| `audit-port.sh` | stale files, hooks stranded by file *moves* | hooks that stayed in the file |
| `check-hook-depth.sh` | hooks relocated into a deeper scope | a hook that moved sideways |
| `check-missing-definitions.sh` | definitions swallowed by a runaway comment | anything still compiling and linking |
| `check-hook-shadowing.sh` | a hook working on a shadow copy of the variable the function uses | shadowing that is deliberate |
| `check-orphaned-resources.sh` | assets no build file installs any more | assets installed by a wildcard |

Run all six. Each one on this list exists because the ones above it missed
something real, and each says plainly what it still cannot see.

Only `check-resolution.sh` is a gate. The rest exit 0 on purpose: they generate
leads for a human to read, and a check that fails a build on its benign hits
just teaches people to skip it.

Two of them earn their place from the same underlying event, which is worth
understanding because it will happen again. When upstream restructures a
function -- hoisting a declaration above a new `if`/`else`, say -- and the merge
drops the hook into one of the new branches, the merge also has to add a
declaration inside that branch to make the hook compile. That new declaration
shadows the hoisted one. Everything compiles: both variables are well-formed,
the types match, neither is unused, and at `/W3` MSVC says nothing. But the
branch now writes to its private copy while the function goes on to use the
outer variable, still at its initial value -- which for an `MWWorld::Ptr` means
empty, and empty means a segfault at the first dereference.

`check-hook-shadowing.sh` looks for exactly that shape. Note its criterion is
**not** "a shadow inside a hook": in both real cases the shadowing declaration
sat just *outside* the hook markers, because it is the line the merge added
rather than a line the fork ever wrote. What identifies it is that the block it
opens contains a hook.

---

## Traps, in the order they will bite

### Line endings will multiply your conflicts by three

A tree checked out on Windows with `core.autocrlf=true` is CRLF; release
archives are LF. Mix them and git sees *every* file as modified on both sides.
Observed on the 0.47 → 0.51 port: **1768 conflicted files before normalising,
571 hunks after.** `port-to-openmw.sh` normalises all three trees to LF.

### The merge base must be exact

It has to be the OpenMW release the tree is genuinely based on — check
`resources/version` or `OPENMW_VERSION_*` in `CMakeLists.txt`. A near-miss base
produces a merge that looks plausible and is quietly wrong.

Sanity check: after importing, at least ~90% of shared C/C++ files should be
byte-identical to the base. If far fewer are, the base is wrong.

### Deleted upstream modules keep shadowing the new ones

The merge keeps files upstream deleted. `cmake/FindSDL2.cmake` was removed in
0.51 so that the package's own config could export the `SDL2::SDL2` imported
target — but the merge preserved TES3MP's 0.47 copy, which shadowed the config
and defined no such target. The failure surfaced as a generate-step error in
`extern/osg-ffmpeg-videoplayer`, three layers from the cause.

After merging, list files that exist in the merge but not upstream, and justify
each one. If TES3MP did not deliberately add it, it is probably stale.

### Misaligned hunks silently duplicate whole blocks

When upstream moves a block, git may pair TES3MP's copy against unrelated
upstream text. Resolve both sides and you end up with the block twice — which
CMake reports as *"binary directory is already used to build a source
directory"*, not as a duplication. Two blocks were duplicated this way on the
0.51 port: the app `add_subdirectory` list and the WIN32/MSVC subsystem block.

After resolving a file, check for accidental duplicates before moving on.

### Guards get orphaned when upstream restructures around them

TES3MP wraps client-only dependencies in `IF (BUILD_OPENMW OR BUILD_OPENCS)` so
the dedicated server can build without them. When upstream restructures that
region, the `IF` can vanish into a resolved hunk while the `ENDIF` survives.
Two orphaned `ENDIF`s appeared this way. Always re-check `IF`/`ENDIF` balance in
a CMake file you have edited — the parser reports it far from the real line.

### Hook blocks span conflict hunks

A single `Start of tes3mp … End of tes3mp` block is routinely split across two
or three conflict hunks. A hunk can therefore look like plain 0.47-vs-0.51
divergence while actually being the middle of a `change (major)` block, and
resolving it in isolation drops half a hook or re-enables code the fork
deliberately suppressed.

Classify hunks by reading the surrounding region, not the hunk alone. Anything
that resolves conflicts mechanically must skip files containing
`change (major)` hooks entirely — those hooks usually work by commenting
upstream code out, and mechanically taking "hook plus upstream code" silently
restores behaviour TES3MP disabled on purpose. It compiles, it runs, and it is
wrong only in play.

### A hook can land inside a function body

The most expensive failure of the 0.47 → 0.51 port, five times over. When
upstream restructures a region, git can place a tes3mp block *inside* a function
rather than beside it. Braces stay balanced. Hook counts are unchanged. The file
compiles. The only symptom is an error in a **different** file saying a member
does not exist.

It hit `CellStore`'s six accessors (inside `forEach`), `ContainerStore::setResolved`
(outside its class entirely), three `ActiveSpells` hooks (inside `unloadActor`),
`CreatureStats::setSummonedCreatureActorId` (inside `updateAwareness`),
`Spells::setPowerUseTimestamp` (inside `usePower`), `CharacterController::getAttackType`,
`CellRef::setDestCell` (inside `getDestCell`), and `DragAndDrop::finish` — whose
entire hook, signature included, ended up mid-function inside `drop()`.

`CI/check-hook-depth.sh` compares each hook's brace depth against the previous
tree. Treat its output as leads, not failures.

### Suppress upstream code with line comments, never a block comment

The hook markers are themselves block comments, and block comments do not nest.
On the 0.51 port a `change (major)` hook opened `/*` to suppress
`World::rechargeItems`' recharge loop and never closed it. The comment ran on for
**330 lines**, silently deleting every function from `teleportToClosestMarker`
through `spawnRandomCreature` — `getPlayerPtr`, `updateWeather`, `goToJail`,
`confiscateStolenItems`.

The file still compiled. Those symbols would have gone missing at link time, and
the compiler's errors pointed 300 lines past the cause, at the first place it
noticed the namespace had closed.

Always `//` the suppressed lines. `CI/check-missing-definitions.sh` catches this
one directly.

### Check binaries for conflict markers

git will happily write conflict markers into a binary file. On the 0.51 port it
did exactly that to `files/data/fonts/DejaVuLGCSansMono.ttf`, which still parsed
as valid TrueType afterwards — nothing complains until the font fails to load at
runtime, long after you have stopped thinking about the merge.

After merging, scan the tree for files that contain both a conflict marker and a
NUL byte in their first few KB, and restore those from upstream rather than
editing them.

### Verify the suppression survived, every time

Most `change (major)` hooks work by commenting an upstream call out —
`enable()`, `lock()`, `deleteObject()` — because the server, not the client,
decides when that happens. Resolving such a hunk by pasting in upstream's whole
function silently restores the live call, and the result compiles, runs, and
desyncs.

Two of these slipped through on the 0.51 port before being caught. After
resolving a file, grep it for the calls the hooks are supposed to suppress and
confirm each one is still commented out.

A related mechanical trap: these hunks often begin *partway into* a class body,
after `public:` and the `execute()` signature. Pasting a complete replacement
function therefore duplicates the prefix, which shows up as a brace imbalance
somewhere far below. Check `{` / `}` balance per file after editing.

### The dangerous engine changes are the ones that still compile

Renames and deletions fail loudly and are safe. What corrupts data silently is a
field that changes type or meaning while remaining assignable. Real examples
from 0.47 → 0.51:

| Change | Would have failed how |
|---|---|
| `Potion::mData.mAutoCalc` → `mFlags` | loudly (rename) |
| `Weapon` damage `unsigned char[2]` → `std::array` | loudly |
| `Ingredient` effect/skill/attribute `int[4]` → `ESM::RefId[4]` | **silently, wrong data on the wire** |
| `ActiveEffect::mArg` `int` → `std::variant<RefId, RefNum>` | **silently** |

`apps/openmw/mwmp/EngineSurfaceCheck.hpp` pins every mirrored field against the
type this port was written for, so that class of change becomes a compile error
naming the field. **Run it first on a new OpenMW** — it is the fastest available
survey of what actually changed under you.

### Budget for the bugs that only running it can find

A clean build is roughly the halfway point, not the end. Every check in the
table above exists to shrink this list, and the list is still not empty, because
the defects that survive all of them are ones where the code is *valid* and only
its meaning changed. From the 0.47 -> 0.51 port, in the order they were found:

| Symptom | Cause |
|---|---|
| server exited instantly, no message | `main()` had no `catch`; 0.51 throws during config parsing |
| every content file "missing" | 0.51 keys file collections by extension *without* the leading dot |
| every plugin blamed on its neighbour | 0.51 prepends `builtin.omwscripts`, and the server compares by position |
| died after connecting, on drawing chat | six MyGUI layouts no build file installed any more |
| segfault on the first global script | a relocated hook shadowing the `Ptr` the function goes on to use |
| promotion packets carrying the old rank | four faction hooks placed before the rank change instead of after |
| double subtitles, double item removal | a hook left live beside the upstream copy it was meant to replace |

Not one of these produces a compiler diagnostic. Several produce no log line
either, and the two that do produce a misleading one. Plan the schedule around
that: get to a running client early and spend real time in it, because until
something has connected, logged in, and drawn a frame, "it builds" is a claim
about syntax.

Two habits paid for themselves repeatedly:

**Establish the baseline first.** Build pristine upstream OpenMW against the
same game data and confirm it plays. When the port then misbehaves you can ask
whether upstream does the same thing, and the answer is usually decisive in one
run. The `OpDisable` segfault was confirmed as ours in about a minute this way.

**Turn off the crash catcher when you want the truth.**
`OPENMW_DISABLE_CRASH_CATCHER=1` turns an access violation back into an honest
segfault. With it installed, the same fault surfaced as `Execution of script
"MarkTRStartScript" failed: SHM lock timed out` -- a message that names neither
the fault nor anything near it, and that the script interpreter then swallowed
so the game kept running with the object silently un-disabled.

---

---

## The rule that keeps the server portable

The dedicated server has **no dependency on `components/esm` at all** (verified
by `apps/openmw-mp/tests/test_standalone_protocol.cpp`, which compiles the
protocol layer with no OpenMW headers reachable). This is deliberate and it is
what makes a port a client-only problem.

It rests on one asymmetry:

> `components/openmw-mp/Base/records/` mirrors engine records using the field
> types of the OpenMW release that **defined the wire format** — currently 0.47.
> `apps/openmw/mwmp/RecordConvert*.hpp` absorbs all engine change.

So when an assertion fires or a converter stops compiling:

1. Type changed, meaning did not (`int` → `int32_t`) — update the assertion.
2. Became an `ESM::RefId` — add a `RefIdCompat` conversion in the converter.
3. Meaning changed (different units, semantics) — the converter needs real
   translation logic, and this probably warrants a protocol version bump.

**Never** resolve one by editing the mirror. The mirrors *are* the protocol;
changing a field type there breaks every deployed server and client. This is the
single most important convention in the codebase.

Two more, learned the hard way:

- Use `RefIdCompat::fromWireCreate()` for incoming ids, never
  `fromWireExisting()`. TES3MP creates records on the fly, so an incoming id
  routinely names a record this client has not built yet; looking it up instead
  of interning it yields an empty `RefId` and the packet silently does nothing.
- Where OpenMW solves the same conversion internally, copy its logic rather than
  inventing one. The skill/attribute/summon classification in
  `RecordConvertPlayer.hpp` is lifted verbatim from
  `components/esm3/activespells.cpp`, which faces the identical problem loading
  pre-RefId saves. If upstream's classification changes, ours must too.

---

## Making the next port cheaper

`CI/hook-report.sh` splits the surface three ways, and the split matters:

- **`addition` (573)** — usually one or two lines calling into `mwmp`. These
  re-apply almost for free.
- **`change (minor)` (25)** — small edits to upstream lines. Cheap.
- **`change (major)` (134)** — rewrites of upstream logic. These conflict hard
  and need real judgement *every single release*.

That last number is the lever. Every `change (major)` converted into "call one
mwmp function here, put the logic in `apps/openmw/mwmp/`" is a conflict that
stops recurring forever. The logic is identical; only its location changes.

Concretely, when you touch a `change (major)` hook during a port and find
yourself re-deriving what it meant: that is the moment to move its body into
`mwmp` and leave a single call behind. You are already paying the cost of
understanding it.

Keep hook comments exactly as they are. The `Start of tes3mp …` / `End of
tes3mp …` delimiters are what make the hooks machine-locatable, and every tool
here depends on them.
