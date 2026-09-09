# Morrowind Co-op — client, server and source

TES3MP (a hard fork of OpenMW 0.47) ported onto **OpenMW 0.51.0**.

## Binaries

Published as [release assets](../../releases), not as files in this tree.

```
client/   tes3mp.exe, tes3mp-server.exe, tes3mp-browser.exe, dependencies,
          a stock structural openmw.cfg, and openmw.cfg.standalone for use
          without Mod Organizer
server/   server scripts and an empty world skeleton
*.bat     Play / Start Server / Server Browser
SETUP.txt install and troubleshooting notes
```

No mods are here and none ever will be — they belong to their authors and are
downloaded from Nexus by the installer.

## Source

This is the corresponding source for the binaries above, as GPLv3 requires.

| Branch    | What it is                                              |
|-----------|---------------------------------------------------------|
| `tes3mp`  | **The port.** TES3MP running on OpenMW 0.51.            |
| `base`    | Pristine OpenMW 0.47.0, the fork point.                  |
| `omw51`   | Pristine OpenMW 0.51.0, the target.                      |
| `main`    | This README only.                                        |

The three branches exist because porting is a three-way diff: `base` → `tes3mp`
shows what TES3MP changed, `base` → `omw51` shows what upstream moved underneath
it, and the interesting bugs live where those two overlap.

Build notes and the CI checks that find misplaced hooks are in `CI/` on the
`tes3mp` branch.

## Licence

OpenMW and TES3MP are GPLv3, and so is this. The source above corresponds to the
published binaries.
