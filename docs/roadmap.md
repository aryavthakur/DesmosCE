# Next engineering priorities

1. **Device validation and memory headroom.** Run the documented smoke tests in Cemu with a user-supplied ROM and on the TI-84 Plus CE Python. Measure stack high-water and dynamic-library placement, test FileIOC archive/garbage collection and native math edge cases. Profile before raising limits. Do not promote the build to stable on host evidence alone.
2. **Scope-aware dependency/inference pass.** Replace flat unknown discovery with lexical scopes and dependency closure, explicit duplicate-definition diagnostics and inference tests. Add UI removal/reset for inferred sliders. Keep user functions distinct from relations and multiplication.
3. **Editor fidelity and usability.** Variable-size mathematical glyphs, scalable grouping, better notices/long-result clipping, selection ranges, safer compound deletion, per-row undo, coefficient editing shortcuts, adjustable panel width and inline sliders. Preserve the tested structural model.
4. **Graph robustness and responsiveness.** Adaptive explicit subdivision with work budgets, cached implicit samples, ambiguous-cell handling, cancellable progressive rendering, fitted/list/implicit trace, robust special-point deduplication and more carefully tested tangency handling.
5. **Data and fitting.** Missing/blank table cells, computed columns, multiple tables, manual seeds/bounds, covariance diagnostics and additional reference datasets. Consider separately loadable solver/features only after profiling identifies the memory cost.
6. **Broader mathematics.** Add catalog items deliberately with tests: distributions, statistical plots, parametric/polar, matrix/complex value types and richer calculus. No menu entry or parser token should be counted as a completed feature.

The [coverage matrix](feature-matrix.md) remains the release-facing checklist. Update the evidence column and native memory report with every substantive change.
