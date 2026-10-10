# DesmosCE — experimental native alpha

An independent, offline, Desmos-inspired graphing calculator written in C for the **TI-84 Plus CE Python**. This is native eZ80 code, not a browser, Python script, remote display, or port of Desmos JavaScript.

**Verification boundary:** the application cross-compiles with CE C/C++ Toolchain v15.0. Its math, editor, graph analysis, integration workflows, and serialization have host tests. **No physical calculator or ROM-backed emulator execution has been performed.** Treat the `.8xp` as an experimental development build, not a stable or exam-authorized release. Back up your calculator before testing.

## What is here

- A white split-view expression list and graph, plus expression-focused, graph-focused, table, slider, and regression-result screens.
- A real structural editor for fractions, powers, roots, and subscripts. Arrow keys move between mathematical slots; undo/redo, whole-expression copy/paste, row duplication and deletion are included.
- Explicit curves, multiple curves from list arithmetic, coarse implicit equations, inequalities, restrictions, points, and table scatter plots.
- Numerical roots, intercepts, extrema, and intersections for scalar explicit curves.
- Desmos-style `~` fitting: `2k+5~17` finds `k=6`; `y_1~mx_1+b` fits table data. Converged coefficients can be reused in other expressions.
- Column-pivoted Householder QR for affine-in-parameter models; bounded, multi-start Levenberg–Marquardt with augmented QR for nonlinear models. Underdetermined and rank-deficient problems are reported instead of silently inventing coefficients.
- One 32-row `x_1`/`y_1` table, six sliders, lists, statistics, trigonometry, and bounded numerical calculus.
- Versioned binary64 session serialization with CRC32 and alternating AppVar save slots. **The codec is host-tested; actual on-device saving is not yet verified.**

This is a substantial subset, **not Desmos feature parity**. Read the [273-item coverage matrix](docs/feature-matrix.md) and [limitations](docs/limitations.md) before relying on a feature.

## Obtain the build

Open this repository's **Actions → Test and build DesmosCE → successful run → DesmosCE-experimental-alpha** artifact. The ZIP includes `DESMOSCE.8xp`, the exact application source, build/test logs, memory estimates, host-renderer SVG previews, checksums, and third-party source/notices. Artifacts expire after 30 days; download the archive or rebuild it.

The compiled file is `bin/DESMOSCE.8xp` when building locally. Transfer instructions and launcher versions are in [installation.md](docs/installation.md). A computer is used for building/transferring files, not normal calculator operation. Runtime libraries are separate upstream downloads.

## Build and verify

Install [CE Toolchain v15.0](https://github.com/CE-Programming/toolchain/releases/tag/v15.0), then add its `bin` directory to `PATH`:

```sh
make
# Produces bin/DESMOSCE.8xp, bin/DESMOSCE.obj and bin/DESMOSCE.map
python3 tools/check_size.py
```

On a conventional Linux host with a C compiler and Python:

```sh
make -f Makefile.host test
python3 -m pip install numpy==2.2.6 scipy==1.15.3
make -f Makefile.host reference
make -f Makefile.host preview
```

`test` enables AddressSanitizer and UndefinedBehaviorSanitizer. `reference` compares against independent NumPy/SciPy calculations. `preview` calls the actual application renderer with an SVG drawing adapter. **Those SVGs are not emulator screenshots and do not verify calculator font rasterization or performance.**

The initial local cross-build is 64,063 bytes compressed, with 130,886 bytes of linked text and 49,834 bytes of BSS. Read [performance.md](docs/performance.md): compressed size does not describe execution RAM, and the static layout margin is narrow. CI writes current measurements to `memory-report.json`.

## First use

`Y=` switches split/expression views. Enter a formula and press `ENTER` to apply it and select the next row. `GRAPH` applies the current expression and opens the graph. `MATH` opens the template/function palette. `STAT` opens the data table. **`2nd` then `MODE` saves and exits.**

For `~`, press `2nd` then `STO→`. For `=`, press `STO→`. Standard ALPHA letters are lowercase; `2nd ALPHA` toggles alpha lock. The full [keymap](docs/keymap.md) is important: physical-keyboard navigation differs from a touchscreen.

### Examples

```text
y=x^2-6x+5
f(x)=(x^2+1)/(x-3)
2k+5~17
y_1~mx_1+b
y=2x+[1,2,3,4]
L=[1,3,3,5,8]
mean(L)
(x-2)^2+(y+1)^2=25
y=ax^2+b
```

Use separate expressions for each line. Graph updates happen on apply, not after every keystroke. Unknown parameter inference has documented scope limitations; use direct inline regression models.

## Documentation

[Features and syntax](docs/features.md) · [Keymap](docs/keymap.md) · [Installation](docs/installation.md) · [Compatibility](docs/compatibility.md) · [Architecture](docs/architecture.md) · [Tests and acceptance results](docs/testing.md) · [Performance/memory](docs/performance.md) · [Limitations](docs/limitations.md) · [Coverage matrix](docs/feature-matrix.md) · [Next engineering priorities](docs/roadmap.md)

Original application code is MIT-licensed. See [third-party notices](THIRD_PARTY_NOTICES.md) for CE libraries and relinking information. No Desmos source, TI ROM, firmware image, or proprietary assets are included. DesmosCE is not affiliated with Desmos or Texas Instruments. General math and practice only; no claim of SAT, ACT, classroom, or other exam authorization.
