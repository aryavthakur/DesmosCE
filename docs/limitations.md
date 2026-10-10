# Known limitations — read before relying on results

## Release status

This is an experimental alpha with a real native binary and passing host tests. It has not run on a physical calculator or a ROM-backed emulator. In particular, launch, font clipping, continuous keypad polling, native math-library edge cases, save/archive behavior, memory high-water and long-session stability remain unverified.

## Interface differences from Desmos

The display uses a fixed six-pixel GraphX font with structural positioning, not Desmos's full mathematical typography. Fractions, roots, powers and subscripts are supported; scalable parentheses, general calculus templates and matrices are not. Expressions apply on ENTER/GRAPH/navigation, rather than producing a new fit on each character. Split-panel width is fixed. Sliders have a separate screen rather than inline controls. Color markers are small squares. There is no touchscreen dragging, range selection, autocomplete, search, row reordering, rich notes, folders, custom styles or full context menus.

The fixed editor pool may run out before the 191-character parser limit. Imported text is preserved read-only in this situation; it is not silently shortened. Only one active row has undo history. Copy/paste replaces a whole expression, not selected substrings. Long results or notices can be clipped by the small screen.

## Math and inference

Results are approximate binary64 real numbers. No symbolic CAS, exact fractions, automatic rationalization, arbitrary precision, matrices, complex arithmetic, distribution objects or statistical hypothesis testing exists. Several names may tokenize but still return an unsupported-function error; token recognition does not mean implementation.

Unknown discovery scans variable nodes in direct regression rows, not a scope-aware transitive dependency graph. Parameters hidden in other definitions may not be inferred. Bound function/summation names may produce extraneous sliders. A parameter already defined or assigned a slider is fixed for fitting. Prefer fresh rows with direct inline fit models. Sliders cannot currently be deleted individually through the UI.

Piecewise/restriction conditions must evaluate to a scalar. Boolean operations are not short-circuiting. There are no nested lists, lists of points, general list filters/comprehensions, or multi-argument user functions. Quantiles use an explicitly documented interpolation convention that may differ from another calculator.

## Numerical algorithms

Nonlinear fits are local and start from four fixed seeds. A converged answer is not proof of uniqueness or best global fit. Poorly scaled, oscillatory, discontinuous or nearly unidentifiable problems may fail or return a different local solution. There are no fit parameter bounds, weights, manual seeds or confidence intervals. Limits are six parameters and 48 residuals.

TRACE scans the current x-window and may miss roots or nearby intersections. It does not establish global extrema. Constant curves can yield many candidates, truncated at the 40-point limit. Implicit/list/fitted curves do not have the full scalar explicit trace workflow.

Explicit midpoint guards prevent common asymptote bridges but can omit valid steep curves or miss a discontinuity. The coarse implicit grid can miss tiny components and mishandle ambiguous topology. Strict-inequality boundaries are only coarsely dashed. Parametric/polar plots, polygons, filled area-between-curves and statistical visualizations are absent.

Numerical differentiation is finite-difference based. Integration is bounded composite Simpson refinement, not robust improper/singular quadrature. A successful convergence comparison is not a rigorous error certificate.

## Data and persistence

Only one 32-row two-column table exists. There is no CSV import/export, computed table column, multiple independent tables, or missing-data representation. The other cell in a new row initially equals zero; incomplete rows affect fits until edited. Tables must be populated fully before trusting regression.

Saving occurs on `2nd MODE`, not continuously. Unsaved edits can be lost. The alternating CRC-protected codec is tested, but native FileIOC, archive garbage collection and interrupted writes need real hardware testing. Backup both DCEA and DCEB. Clipboard and undo/cursor state do not persist.

## Capacity and compatibility

The code budget is tight; see [performance](performance.md). A compressed 64 KB file is not a 64 KB RAM requirement. This app targets the CE eZ80, not other TI models. Launcher instructions must match the OS, and no firmware modification is automated.

None of these limits are represented as unsupported claims of full Desmos parity or exam approval. Missing features are tracked in [feature-matrix.md](feature-matrix.md).
