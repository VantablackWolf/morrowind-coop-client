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
