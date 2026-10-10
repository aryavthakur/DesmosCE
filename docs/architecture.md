# Architecture

## Data path

Physical key -> logical key (`main.c`) -> application state (`app.c`) -> structural editor (`editor.c`) -> bounded expression text -> AST (`parser.c`) -> evaluator/model -> graph or regression -> drawing callbacks -> GraphX. The portable modules do not include calculator headers. The host SVG adapter implements the same drawing interface for layout inspection.

## Module boundaries

| File | Responsibility |
|---|---|
| `include/dce.h` | Real type, AST, values, sessions, model and numerical interfaces |
| `include/ui.h` | Draw callbacks, editor nodes, application modes and logical keys |
| `src/parser.c` | Tokenization, precedence, bounded recursive descent and diagnostic text |
| `src/eval.c` | Scalar/list operations, functions, lexical bindings and domain checks |
| `src/model.c` | Definitions, inference, compile/rebuild, scalar evaluation and number formatting |
| `src/regression.c` | Residual assembly, conservative affine classification, pivoted QR and LM |
| `src/editor.c` | Parent-linked structural slots, import/export, navigation, history and layout |
| `src/graph.c` | Coordinate transforms, grid, explicit and implicit rendering |
| `src/analysis.c` | Finite-difference extrema, bracketing roots and intersections |
| `src/storage.c` | Versioned binary format, validation and CRC32 |
| `src/app.c` | Screen state machine, keymap, tables, sliders and result presentation |
| `src/main.c` | GraphX, physical keyboard, FileIOC AppVars and OS entry/exit |
| `src/format_native.c` | Small integer/string formatter; Real formatting is separate |
| `tools/render_host.c` | Actual app renderer with SVG callbacks, not emulation |

## Memory and precision

The application is single-threaded and deliberately non-reentrant. Large solver matrices, draw temporaries and model storage use static buffers. AST child links are signed 8-bit indices with -1 as absent and 128 slots. Number/name storage is a union. These choices reduced RAM without sacrificing binary64 arithmetic.

Native `Real` is CE `long double`; host `Real` is `double`. `storage.c` asserts an eight-byte representation and encodes explicit little-endian binary64 values rather than dumping structs. Display rounding never overwrites stored values. Native printf floating-point support is not linked; `dce_format` handles finite values, scientific notation, subnormals and overflow text.

Source: [CE hardware types](https://ce-programming.github.io/toolchain/static/hardware.html). The selected linker layout and measured margins are in [performance](performance.md).

## Expressions and evaluation

The parser recognizes single-letter algebra names, known function names and subscript names. Power is right associative; comparison chaining creates conjunctions. AST construction is bounded and does not recover by silently dropping invalid input.

Evaluation tracks active definition rows to reject cycles, a depth cap, an operation budget and lexical bindings for user function/calculus variables. Lists carry a length and point flag. Lists broadcast scalar arithmetic; there is no general dynamic value heap.

Inference is a separate, less sophisticated pass than evaluation: it scans direct variable nodes rather than computing a full scope-aware dependency closure. This can create extraneous sliders for bound names or fail to infer parameters hidden in definitions. Direct inline regression syntax is the supported reliable path for this alpha.

## Editor

Editor nodes are sequence, character, fraction, power, subscript or root. Each compound node owns one or two sequence slots. Parent/sibling links provide cursor traversal without reparsing on every arrow press. Fraction/power insertion captures the preceding atom or parenthesized group. Rendering measures ascent/descent recursively, then positions fraction bars, radicals and raised/lowered slots.

Serialization produces bounded parser text. Import and repeated round-trips are tested against expression values. An imported expression that exceeds structural capacity is preserved in a read-only source field rather than truncated. History is ten serialized snapshots for the active row; switching rows starts a new per-row editing history.

## Numerical fitting

Residuals are assembled from all `~` rows. A conservative symbolic degree pass recognizes models affine in inferred parameters; these use direct scaled pivoted QR. Nonlinear models use finite-difference Jacobians and the augmented system `[J/scale; sqrt(lambda) I]`, avoiding explicit normal-equation construction. Acceptance decreases the squared-residual objective and reduces damping; failed steps restore parameters and increase damping. A fresh undamped rank check precedes installation of fitted values.

The implementation bounds all work; it does not establish global convergence. Parameter count, observation count, domain and rank errors remain explicit. Residual weighting and inequality-constrained fitting are intentionally absent.

## Rendering and responsiveness

The graph is cached as the previous draw buffer while equation text changes. Applying an expression rebuilds the model/fit/sliders and requests a graph redraw. Explicit plotting evaluates samples and midpoint curvature; implicit plotting traverses eight-pixel cells. CLEAR is polled between batches. This avoids per-keystroke full fits, but it is not the continuous live-update behavior of desktop Desmos.

## Persistent state

The wire format begins with `DCE1`, total length, generation, and CRC32 (its own field treated as zero), followed by counts, flags, bounds, rows, data and sliders. Decoding validates counts, finite numbers, string terminators, colors and bounds into a temporary session before replacing the live session.

Native saving increments generation and writes the alternate DCEA/DCEB AppVar, closes it, reopens and verifies, then archives it after GraphX has ended. Startup chooses the newest valid slot. This retains a prior good slot if a new write fails. A hardware power-loss/garbage-collection test is still necessary; the host codec tests are not a substitute.

## Extension rule

Add tests before raising limits or adding value types. Keep numerical operations portable and hardware calls inside the adapter. Rerun both host references and native memory gates after any change: desktop performance or a small compressed file does not imply CE feasibility.
