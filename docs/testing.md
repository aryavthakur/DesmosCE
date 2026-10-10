# Verification and acceptance record

## Commands

```sh
make -f Makefile.host test
python3 -m pip install numpy==2.2.6 scipy==1.15.3
make -f Makefile.host reference
make -f Makefile.host preview
make
python3 tools/check_size.py
```

Host tests use AddressSanitizer and UndefinedBehaviorSanitizer. A clean GitHub Actions run repeats the host tests, independent references, actual eZ80 cross-build and static memory gate. The artifact contains its source commit, checksums and logs. Source compilation is not execution of the resulting `.8xp`.

## Automated coverage

| Test | Checks |
|---|---|
| `tests/test_core.c` | 45 numeric comparisons plus precedence, lists/statistics, functions, domains, cycles, scalar/joint/table fits, rank errors and formatting assertions |
| `tests/test_editor.c` | Structural navigation, serialization/evaluation equivalence, undo/redo and codec corruption rejection |
| `tests/test_graph.c` | Requested roots/vertex/intersection, repeated root, rejection of a 1/x root and coordinate transforms |
| `tests/test_app.c` | Logical physical-key sequence, apply/trace, table entry, invalid bounds, fit, sliders and exit request |
| `tests/test_robust.c` | 12,000 deterministic malformed-input/editor trials, nested lexical scopes, repeated semantic round-trips, subnormals and bit flips in every serialized byte |
| `tools/reference_check.py` | 18 independent NumPy/SciPy model/error cases, with tolerances asserted in source |

The randomized trials are regression stress tests, not exhaustive formal verification or a coverage-guided fuzz campaign. The key tests call logical key events; they do not verify electrical keyboard behavior. The storage test exercises serialization, not actual archive writes.

Reference models include exact/noisy polynomials of degree 1–4, exponential, shifted exponential, power, logarithmic, reciprocal, square-root and sinusoidal fits. Error cases cover duplicate x/rank deficiency, underdetermination and an invalid logarithm domain. Coefficient/prediction tolerances are recorded in the script; nonlinear parameter uniqueness is not assumed.

## Requested acceptance scenarios

| # | Scenario | Evidence and remaining gap |
|---|---|---|
| 1 | `y=x^2-6x+5` | Host roots 1/5 and vertex (3,-4) pass; real renderer preview; device interaction pending |
| 2 | `y=2x+1`, `y=-x+7` | Host intersection (2,5) passes; device trace pending |
| 3 | `2k+5~17` | Host fit k=6 and reuse in `2k` pass; fit panel preview |
| 4 | x_1=1..5, y_1=3,5,7,9,11 | Host m=2, b=1 and R-squared=1 pass; table/fit preview |
| 5 | `f(x)=(x^2+1)/(x-3)` | Fraction/power round-trip and slot navigation tested; preview inspected; LCD/key behavior pending |
| 6 | `y=2x+[1,2,3,4]` | Broadcast/list evaluation tested; four-curve real-renderer preview inspected; no pixel-by-pixel reference assertion |
| 7 | L=[1,3,3,5,8] | mean=4, median=3, sample stdev=sqrt(7) pass |
| 8 | `(x-2)^2+(y+1)^2=25` | Actual implicit-renderer host preview inspected; topology accuracy/device performance not certified |
| 9 | `y=ax^2+b` | Two sliders and stepped-value change tested through app events; preview inspected; physical response pending |
| 10 | Save, exit, reopen | Serialization round-trip/checksum rejection passes; native two-AppVar adapter implemented; end-to-end device test NOT performed |

## Preview provenance

`make -f Makefile.host preview` builds the same portable application/renderer code and emits eight 320x240 SVG scenes. The adapter substitutes drawing calls, not numerical results. The SVG uses host text rendering, so it can assess layout and graph output but not GraphX glyph clipping. Do not relabel it as Cemu or calculator screenshots.

## Hardware validation checklist

Before a supported release, use a personally owned calculator or legally obtained user ROM. Record exact model/revision, OS, launcher/runtime versions and binary checksum. Confirm startup/exit repeatedly, all ten scenarios, two-key modifiers, alpha lock, cursor movement through nested fractions, cancellation during graph/fit, invalid expressions, all capacity limits, archive saves after RAM resets, low-memory failure and preservation of the previous good save. Run a long session and measure stack/heap margins and operation timings. Do not redistribute ROM images.
