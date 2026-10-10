# Performance and memory budget

## Measured initial native build

CE C/C++ Toolchain **v15.0**, optimized C11, link-time optimization, `-Oz -fno-inline-functions`, archived ZX7-compressed program. The command is `make`; CI pins the compiler download by SHA-256. Current build measurements are emitted by `tools/check_size.py`.

| Measurement | Initial value |
|---|---:|
| `DESMOSCE.8xp` transfer file | 64,063 bytes |
| Linked text/read-only code | 130,886 bytes |
| Linked initialized data | 0 bytes |
| BSS | 49,834 bytes |
| Conservative v15 required runtime-file byte total | 15,511 bytes |
| Code region from 0xD1A87F to 0xD40000 | 153,473 bytes |
| Estimated code margin before reserve | 7,076 bytes |
| Extra loader/OS bookkeeping reserve used in CI | 6,144 bytes |

**These numbers are a static estimate, not a proof of safe execution.** The runtime allowance uses complete upstream GRAPHX/FILEIOC/KEYPADC/LIBLOAD `.8xv` sizes as a conservative approximation. Actual loaded-library placement, relocation, OS memory state, fragmentation and stack high-water have not been measured on a device. This narrow margin is why the artifact is experimental.

The default BSS/heap range is 0xD052C6 to 0xD13FD8; the configured stack continues to 0xD1A87E. Current BSS leaves roughly ten kilobytes of that BSS/heap range. CI limits BSS to 55,000 bytes and rejects text plus dependency estimate plus reserve above the code-region size. Evaluator recursion is capped at 12, but a formal whole-program stack bound has not been established.

Source: [upstream makefile configuration](https://ce-programming.github.io/toolchain/static/makefile-options.html), [hardware overview](https://ce-programming.github.io/toolchain/static/hardware.html), and the v15 linker defaults. Read `bin/DESMOSCE.map` and the JSON report for each build.

## Capacity limits

| Resource | Limit |
|---|---:|
| Expression rows | 10 |
| Source text per expression | 191 characters plus terminator |
| AST nodes per row | 128 |
| Editor nodes | 128 total, including slot nodes |
| Undo snapshots | 10 for active expression |
| Table points / list elements | 32 |
| Inferred sliders / fit parameters | 6 each |
| Joint regression residuals | 48 |
| Nonlinear starts and iterations | 4 starts, 64 iterations each |
| Evaluation recursion / node budget | 12 / 12,000 per context |
| Trace candidates | 40 |
| Implicit cell spacing | 8 pixels |
| Explicit samples | About one per graph x-pixel plus midpoint checks |
| Simpson panels | 8 through 256, with convergence test |
| Serialized maximum session | 2,769 bytes |

## Optimizations actually made

The initial native image exceeded a useful memory budget. AST unions/compact indices, static scratch reuse, bounded capacities, LTO, disabled inlining, and a narrow integer/string formatter reduced code and BSS. Binary64 math was retained. Unimplemented probability functions are explicitly marked missing rather than silently represented as working placeholders.

Whole-graph recomputation is delayed until apply/settings change; editing redraws its panel while retaining the previous graph. Drawing is double-buffered. Long loops expose cancellation points. No hardware frequency, battery, RAM wait-state or firmware patch is applied by the app.

## Measurements not performed

No physical key latency, graph frames per second, native fit duration, stack high-water, heap fragmentation, battery-life or emulator timing measurements exist. Host timings would not be representative and are not used to claim calculator speed. Implicit plots and nonlinear fitting may be slow. Collect these measurements before changing the label from experimental alpha to supported release.
