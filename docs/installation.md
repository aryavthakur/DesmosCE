# Installation — experimental alpha

**This procedure is based on upstream documentation checked October 10, 2026, not an installation performed on a physical calculator.** Back up important files first. A launcher RAM reset removes unarchived data. Do not test an experimental app on your only copy of important calculator work.

## 1. Identify model and OS

The target is the **TI-84 Plus CE Python**, not the monochrome TI-84 Plus or TI-84 Plus C Silver Edition. Read the OS version in the calculator's About screen. Do not infer it from the Python branding. The Python processor is not used by DesmosCE.

## 2. Choose the documented native launcher

The [official arTIfiCE instructions](https://yvantt.github.io/arTIfiCE/) specify:

| CE OS | Upstream launcher route |
|---|---|
| 5.8.5 | arTIfiCE v3, followed by enabling AsmHook2 |
| 5.8.4 and earlier supported restricted versions | arTIfiCE v2.1 |
| 5.4 or earlier | arTIfiCE is not required |
| Newer/unlisted versions | Not verified; consult upstream before proceeding |

For 5.8.5, follow the upstream sequence exactly: backup, RAM reset, installer transfer, Inequalz setup, then trigger AppVar transfer and AsmHook2 activation. Do not transfer the trigger prematurely. For v2.1, the current official route launches TI-Basic `prgmA`; old Cabri Jr tutorials describe earlier versions. Never downgrade the OS for this project. The application does not modify firmware or install a launcher automatically.

## 3. Install runtime dependencies

Download the [CE runtime libraries v15.0](https://github.com/CE-Programming/libraries/releases/tag/v15.0). Transfer the supplied library group, or the separate `GRAPHX`, `FILEIOC`, `KEYPADC`, and `LIBLOAD` AppVars. Preserve their original names. These are native library files, not Python modules.

## 4. Transfer and launch DesmosCE

Obtain `DESMOSCE.8xp` from a successful Actions artifact or build it locally. Use [TI Connect CE](https://education.ti.com/en/products/computer-software/ti-connect-ce-sw) or a supported transfer utility. Transfer the program as an archived program. It is not loaded through the calculator's Python app.

Use the native launcher selected in step 2 to run `DESMOSCE`. With enabled AsmHook2, upstream documents normal Program-menu launching. Other supported launch routes depend on the installed launcher. A successful cross-build is not evidence that this app's launch has been tested with every route.

After transfer, disconnect the computer. Expressions, graphing, fitting and saving run locally on the calculator. No network connection, phone or computer is required for normal use.

## 5. First-run smoke test

Enter `y=x^2-6x+5`, apply it, and open `TRACE`. Expected special points include roots `(1,0)`, `(5,0)` and vertex `(3,-4)`. Enter `2k+5~17` in a fresh row and check `k=6`. Use `2nd STO` for `~` and `STO` for `=`; see [keymap](keymap.md).

**Save and exit with `2nd MODE`.** Reopen and check the session before relying on persistence. Sessions use AppVars `DCEA` and `DCEB`; keep both. A hard reset or battery loss before a successful save does not preserve new edits.

## Troubleshooting and removal

A missing-library message requires the runtime AppVars, not a Python library. A syntax/invalid-program launch error can indicate that native execution is not enabled, an incompatible launcher, or a damaged transfer. On 5.8.5, upstream says AsmHook2 must be enabled again after a RAM reset. Read the upstream instructions before resetting.

If the app freezes, try `CLEAR` to cancel a long operation. Hardware recovery may require a reset; this can lose unarchived data. Report the calculator model/revision, OS, launcher version, expression and last keys pressed. Do not report an app as stable merely because it opens once.

To remove only this app, delete `DESMOSCE` and, only when you no longer need saved work, `DCEA`/`DCEB`. Shared runtime libraries may be used by other programs. Launcher removal is separate; follow its upstream instructions.

No SAT/ACT or other exam permission is claimed. Do not bypass exam-mode restrictions or assume that ordinary program execution means exam authorization.
