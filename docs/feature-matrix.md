# DesmosCE feature coverage — 273-item implementation map

This matrix reconstructs the requested catalog groups from the development directive. A separate item-for-item copy of the named original catalog was not supplied; these numbers are an implementation mapping, not a claim to reproduce its exact original wording. Additional shortcuts and substitution behavior are recorded in the keymap/features documents.

**Evidence rule:** IMPLEMENTED AND TESTED means the stated portable behavior has automated host evidence, not physical-calculator verification. IMPLEMENTED BUT UNVERIFIED ON HARDWARE means code exists but native behavior remains unverified; it does not imply every edge case has a dedicated host test. PARTIALLY IMPLEMENTED identifies a material scope limitation. NOT IMPLEMENTED is a missing capability, not a hidden placeholder. HARDWARE-INAPPLICABLE is reserved for interaction mechanisms absent on this device, not a reason to dismiss numerical features.

No row claims a physical calculator or ROM-backed emulator test. Core = `tests/test_core.c`; Editor = `tests/test_editor.c`; Graph = `tests/test_graph.c`; App = `tests/test_app.c`; Robust = `tests/test_robust.c`; Ref = `tools/reference_check.py`; Preview = the actual app renderer via `tools/render_host.c`, visually inspected, not an automated pixel reference.

## 1–30: Graphing, equations, solving, special points

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 1 | Explicit functions | y=x^2 | graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview; Graph evaluation | Pixel samples; no speed claim |
| 2 | Multiple expressions | Plot several rows | app.c, graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | App; Preview | Ten rows |
| 3 | Linear graph | y=2x+1 | graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview | Approximate rasterization |
| 4 | Quadratic graph | y=x^2-6x+5 | graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Graph; Preview | Current window |
| 5 | Higher polynomials | y=x^4-2x | eval.c, graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Ref polynomial evaluation | Large values may overflow |
| 6 | Rational curves | y=1/x | graph.c | PARTIALLY IMPLEMENTED | Graph pole rejection | No universal pole guarantee |
| 7 | Radical curves | y=sqrt(x) | eval.c, graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Core; Preview | Real domain only |
| 8 | Exponential curves | y=exp(x) | eval.c, graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Ref | Binary64 overflow |
| 9 | Logarithmic curves | y=ln(x) | eval.c, graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Core; Ref | Positive real domain |
| 10 | Absolute-value curves | y=abs(x) | eval.c, graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path; no dedicated plot assertion | Corner analysis approximate |
| 11 | Trigonometric curves | y=sin(x) | eval.c, graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Core; Ref | Mode-dependent |
| 12 | Implicit equations | x^2+y^2=25 | graph.c | PARTIALLY IMPLEMENTED | Preview circle | Eight-pixel grid |
| 13 | Horizontal lines | y=3 | graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Scalar evaluator; preview infrastructure | Trace may duplicate extrema |
| 14 | Vertical lines | x=3 | graph.c | PARTIALLY IMPLEMENTED | Implicit code path | Coarse implicit renderer |
| 15 | Parametric plotting | (f(t),g(t)) over t | — | NOT IMPLEMENTED | None | Points are not parametric curves |
| 16 | Polar plotting | r=f(theta) | — | NOT IMPLEMENTED | None | No polar renderer |
| 17 | Point plotting | (2,3) | graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Point evaluator; code inspection | Single point only |
| 18 | Point lists | [(1,2),(3,4)] | — | NOT IMPLEMENTED | Explicit evaluator rejection | No nested point values |
| 19 | Function definitions | f(x)=x^2 | model.c, eval.c | IMPLEMENTED AND TESTED | Core; Robust | One argument |
| 20 | Function composition | f(g(0)) | eval.c | IMPLEMENTED AND TESTED | Core | Evaluation depth 12 |
| 21 | Algebraic systems | Joint unknown equations via ~ | regression.c | PARTIALLY IMPLEMENTED | Core joint fit | Numerical least squares, not CAS |
| 22 | X-intercepts | TRACE roots | analysis.c | IMPLEMENTED AND TESTED | Graph roots 1 and 5 | Scalar explicit; may miss roots |
| 23 | Y-intercepts | Evaluate at x=0 | analysis.c | IMPLEMENTED AND TESTED | Graph | Undefined domains omitted |
| 24 | Curve intersections | TRACE between two curves | analysis.c | IMPLEMENTED AND TESTED | Graph (2,5) | Scalar explicit only |
| 25 | Local extrema | TRACE derivative zeros | analysis.c | PARTIALLY IMPLEMENTED | Graph vertex (3,-4) | Finite differences; stationary points |
| 26 | Repeated roots | Tangency at zero | analysis.c | PARTIALLY IMPLEMENTED | Graph (x-2)^2 | Detected through sampled extrema |
| 27 | Asymptote handling | Avoid joining across poles | graph.c | PARTIALLY IMPLEMENTED | Graph 1/x; preview inspection | Heuristic midpoint guard |
| 28 | All real solutions | Exhaustive root set | — | NOT IMPLEMENTED | None | Numerical sampling is not proof |
| 29 | Symbolic solve | Exact algebraic solution | — | NOT IMPLEMENTED | None | No CAS |
| 30 | Coordinate labels | Show selected special point | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | App trace | Forty candidates |

## 31–54: Regression models and regression tools

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 31 | Tilde syntax | y_1~mx_1+b | parser.c, regression.c | IMPLEMENTED AND TESTED | Core; Ref | Direct inferred parameters |
| 32 | Scalar constant solving | 2k+5~17 | regression.c | IMPLEMENTED AND TESTED | Core k=6; App | Numerical result |
| 33 | Linear regression | y_1~mx_1+b | regression.c | IMPLEMENTED AND TESTED | Core; Ref | At most 32 paired points |
| 34 | Quadratic regression | y_1~ax_1^2+bx_1+c | regression.c | IMPLEMENTED AND TESTED | Core; Ref | Pivoted QR |
| 35 | Cubic regression | Degree-three polynomial | regression.c | IMPLEMENTED AND TESTED | Ref exact/noisy | Conditioning matters |
| 36 | Quartic regression | Degree-four polynomial | regression.c | IMPLEMENTED AND TESTED | Ref exact/noisy | Five parameters |
| 37 | Higher-degree regression | Arbitrary polynomial degree | regression.c | PARTIALLY IMPLEMENTED | Six-parameter bound | No general arbitrary degree |
| 38 | Exponential fit | a*exp(b*x_1) | regression.c | IMPLEMENTED AND TESTED | Ref | Local nonlinear fit |
| 39 | Shifted exponential fit | a*b^(x_1)+c | regression.c | IMPLEMENTED AND TESTED | Ref | Fixed initial seeds |
| 40 | Power fit | a*x_1^b | regression.c | IMPLEMENTED AND TESTED | Ref | Real-domain restrictions |
| 41 | Logarithmic fit | a*ln(x_1)+b | regression.c | IMPLEMENTED AND TESTED | Ref | Invalid x rejected |
| 42 | Reciprocal fit | a/x_1+b | regression.c | IMPLEMENTED AND TESTED | Ref | x cannot be zero |
| 43 | Square-root fit | a*sqrt(x_1)+b | regression.c | IMPLEMENTED AND TESTED | Ref | Real domain |
| 44 | Sinusoidal fit | a*sin(b*x_1+c)+d | regression.c | IMPLEMENTED AND TESTED | Ref prediction comparison | Local minima possible |
| 45 | Logistic fit preset | Logistic model template | — | NOT IMPLEMENTED | None | Generic expressions not certified |
| 46 | General nonlinear model | ~ with supported operations | regression.c | PARTIALLY IMPLEMENTED | Ref model subset | Six unknowns; no global guarantee |
| 47 | Multiple fit equations | Several ~ rows jointly | regression.c | IMPLEMENTED AND TESTED | Core two equations | 48 combined residuals |
| 48 | Parameter reuse | Evaluate 2k after fitting k | model.c, eval.c | IMPLEMENTED AND TESTED | Core | Only converged results installed |
| 49 | Residual inspection | Show observed-model residuals | regression.c, app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Ref SSE; Preview details | Numeric list, no residual plot |
| 50 | R-squared | Standard 1-SSE/SST | regression.c | PARTIALLY IMPLEMENTED | Core R2=1 | One y_1 regression only |
| 51 | RMSE / SSE | Fit diagnostics | regression.c, app.c | IMPLEMENTED AND TESTED | Core; Ref | RMSE displayed; SSE stored |
| 52 | Rank / underdetermination errors | Reject unidentified models | regression.c | IMPLEMENTED AND TESTED | Core; Ref | Numerical rank threshold |
| 53 | Bounds and starting guesses | User constraints and seeds | — | NOT IMPLEMENTED | None | Four hard-coded initial seeds |
| 54 | Weighted / log-mode / inference | Weights, transformed objective, intervals | — | NOT IMPLEMENTED | None | Ordinary unweighted residual objective |

## 55–70: Tables and function evaluation

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 55 | Data table | Editable x_1 / y_1 | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | App; Preview | One table |
| 56 | Cell navigation | Arrow and ENTER movement | app.c | IMPLEMENTED AND TESTED | App | Logical keys, not electrical scan |
| 57 | Numeric cells | Enter finite values | app.c | IMPLEMENTED AND TESTED | App | 32 rows |
| 58 | Expression cells | Evaluate scalar cell text | app.c, eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path; Core evaluator | No persistent cell formula |
| 59 | Add rows | Enter next x/y pair | app.c | IMPLEMENTED AND TESTED | App | Other cell defaults to zero |
| 60 | Delete data rows | 2nd DEL | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code inspection | No undo for table deletion |
| 61 | Scatter plot | Plot entered pairs | graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview table fit | Fixed point style |
| 62 | Table variable substitution | x_1 and y_1 in expressions | eval.c | IMPLEMENTED AND TESTED | Core; Ref | Lists of real scalars |
| 63 | Multiple independent tables | x_2 / y_2 and more | — | NOT IMPLEMENTED | None | Single built-in table |
| 64 | Multiple columns | More than x/y | — | NOT IMPLEMENTED | None | Two columns only |
| 65 | Computed columns | y_1=f(x_1) table column | — | NOT IMPLEMENTED | None | Use separate expression evaluation |
| 66 | Automatic function table | Generate x and f(x) rows | — | NOT IMPLEMENTED | None | Manual data entry |
| 67 | Scalar substitution | f(3), a=5 | eval.c | IMPLEMENTED AND TESTED | Core | Single-argument user functions |
| 68 | List function evaluation | f([1,2,3]) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Implemented broadcast path | 32 scalar results |
| 69 | CSV import / export | Exchange table files | — | NOT IMPLEMENTED | None | No file browser/importer |
| 70 | Missing-data handling | Blank cells excluded from fit | — | NOT IMPLEMENTED | None | Default zero is actual data |

## 71–92: Graph interface controls

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 71 | Split graph/list layout | Side-by-side editing | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview | Fixed 147-pixel split |
| 72 | Expression-focused layout | Y= toggles wide editor | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview fraction scene | Fixed font |
| 73 | Graph-focused layout | GRAPH | app.c | IMPLEMENTED AND TESTED | App; Preview | 197-pixel graph height |
| 74 | Grid | Light coordinate lines | graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview | Fixed theme |
| 75 | Axes | Dark x/y axes | graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview | Out-of-window axes omitted |
| 76 | Tick labels | Automatic 1/2/5 spacing | graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview | Bounded label count |
| 77 | Pan | Arrow keys | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | 12% window increments |
| 78 | Zoom in / out | + and - | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | Centered zoom |
| 79 | Window bounds | WINDOW editable limits | app.c | IMPLEMENTED AND TESTED | App invalid bound rejection | +/-1e12; minimum span |
| 80 | Reset view | 2nd ZOOM | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code inspection | Resets to +/-10 |
| 81 | Zoom-fit data | ZOOM | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview setup; code path | Table data only |
| 82 | Equal-axis scale lock | Square coordinate aspect | — | NOT IMPLEMENTED | None | Manually choose bounds |
| 83 | Grid toggle | MATH in WINDOW | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code inspection | Grid only |
| 84 | Visibility toggle | 2nd TRACE | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code inspection | Current expression |
| 85 | Expression colors | Six-color row cycle | app.c, graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview | Not user-editable |
| 86 | Line/point styles | Style controls | — | NOT IMPLEMENTED | None | Fixed defaults |
| 87 | Special-point selection | TRACE then LEFT/RIGHT | app.c | IMPLEMENTED AND TESTED | App; Graph | Limited scalar explicit analysis |
| 88 | Continuous cursor trace | Arbitrary x-coordinate tracing | — | NOT IMPLEMENTED | None | Special points only |
| 89 | Adjustable pane width | Drag or keyboard divider | — | NOT IMPLEMENTED | None | Fixed split |
| 90 | Touch/pinch/mouse input | Desktop/touch gestures | — | HARDWARE-INAPPLICABLE | Device has physical keypad | Pan/zoom have key alternatives |
| 91 | Live updates while typing | Recompute every edit | app.c | PARTIALLY IMPLEMENTED | App commit workflow | Update on apply, not each character |
| 92 | Cancel / progressive rendering | Interrupt long work | main.c, graph.c, regression.c | PARTIALLY IMPLEMENTED | Cancellation callbacks compiled | CLEAR; no progress indicator |

## 93–110: Expression editing

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 93 | Structured fractions | Numerator and denominator slots | editor.c | IMPLEMENTED AND TESTED | Editor; Robust; Preview | 128 editor nodes |
| 94 | Structured powers | Raised exponent slot | editor.c | IMPLEMENTED AND TESTED | Editor; Robust | Fixed-size glyphs |
| 95 | Structured radicals | Root bar and radicand | editor.c | IMPLEMENTED AND TESTED | Editor; Robust; Preview | Square root only template |
| 96 | Structured subscripts | x_1 slot | editor.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview fit; code path | Restricted name grammar |
| 97 | Left/right slot traversal | Enter/leave nested objects | editor.c | IMPLEMENTED AND TESTED | Editor; Robust | Logical-key tests |
| 98 | Vertical slot traversal | Fraction/exponent movement | editor.c | IMPLEMENTED AND TESTED | Editor | Structural, not geometric nearest cursor |
| 99 | Scalable parentheses | Size grouping to contents | — | NOT IMPLEMENTED | None | Plain character parentheses |
| 100 | Undo/redo | 2nd LEFT/RIGHT | editor.c | IMPLEMENTED AND TESTED | Editor | Ten active-row snapshots |
| 101 | Whole-expression clipboard | 2nd + / 2nd - | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code inspection | Replaces full expression |
| 102 | Selection clipboard | Copy a selected range | — | NOT IMPLEMENTED | None | No selection model |
| 103 | Duplicate row | 2nd DEL | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code inspection | Ten-row maximum |
| 104 | Delete row | 2nd CLEAR | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code inspection | No deleted-row recovery |
| 105 | Function palette | MATH inserts templates | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code inspection | 56 entries; not autocomplete |
| 106 | Autocomplete | Contextual function suggestions | — | NOT IMPLEMENTED | None | Palette alternative |
| 107 | Implicit multiplication | 3x, 2(x+1) | parser.c | IMPLEMENTED AND TESTED | Core; Robust | Single-letter algebra grammar |
| 108 | Error and overflow handling | Preserve/reject invalid input | parser.c, editor.c, app.c | IMPLEMENTED AND TESTED | Robust malformed input | Notices may clip on device |
| 109 | Reorder / folders / notes | Organize expression workspace | — | NOT IMPLEMENTED | None | Flat ten-row session |
| 110 | Saved workspace | Reopen expressions/data/settings | storage.c, main.c | PARTIALLY IMPLEMENTED | Editor; Robust codec | Native persistence untested |

## 111–121: Sliders and interactive transformations

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 111 | Infer parameters | y=ax^2+b creates a,b | model.c | PARTIALLY IMPLEMENTED | App | Flat scan; scope limitations |
| 112 | Slider value | LEFT/RIGHT changes value | app.c | IMPLEMENTED AND TESTED | App | Logical keys |
| 113 | Slider minimum | Edit minimum field | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code validation | Must contain current value |
| 114 | Slider maximum | Edit maximum field | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code validation | Maximum > minimum |
| 115 | Slider step | Positive configured increment | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | Finite positive value |
| 116 | Type parameter value | Numeric replacement | app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Shared cell evaluator | Separate slider panel |
| 117 | Graph refresh from slider | Rebuild after adjustment | app.c | IMPLEMENTED AND TESTED | App rebuild state | Device duration unmeasured |
| 118 | Horizontal/vertical shifts | y=(x-h)^2+k | eval.c, app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Shared evaluator/inference | Six sliders |
| 119 | Stretch/reflection | y=a*f(b*x) | eval.c, app.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Core composition components | Full expression must fit limits |
| 120 | Slider animation | Play/pause/loop | — | NOT IMPLEMENTED | None | Manual stepping |
| 121 | Drag movable points | Graph point controls parameters | — | NOT IMPLEMENTED | None | No keyboard point-drag alternative |

## 122–134: Inequalities and restrictions

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 122 | Less-than comparison | x<3 | parser.c, eval.c | IMPLEMENTED AND TESTED | Core chained conditions | Scalar/list numeric booleans |
| 123 | Greater-than comparison | x>3 | parser.c, eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code branch | No dedicated branch assertion |
| 124 | Non-strict comparisons | <= and >= | parser.c, eval.c | IMPLEMENTED AND TESTED | Core piecewise | Approximate input values |
| 125 | Chained comparisons | 0<x<5 | parser.c, eval.c | IMPLEMENTED AND TESTED | Core; Robust | Ordered comparison chain |
| 126 | Domain restriction | y=x^2{x>0} | eval.c, graph.c | IMPLEMENTED AND TESTED | Robust | Scalar condition only |
| 127 | Range restriction | y=x^2{y<4} | graph.c | PARTIALLY IMPLEMENTED | Code path | Uses first value for list curves |
| 128 | Piecewise values | {x<0:-x,x>=0:x} | parser.c, eval.c | IMPLEMENTED AND TESTED | Core | Scalar condition; no default shorthand |
| 129 | Multiple conditions | and / or | parser.c, eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Implemented operations | Not short-circuiting |
| 130 | Linear inequality region | y<2x+1 | graph.c | PARTIALLY IMPLEMENTED | Implicit code path | Sparse stippling |
| 131 | Nonlinear inequality region | x^2+y^2<25 | graph.c | PARTIALLY IMPLEMENTED | Implicit code path | Eight-pixel cells |
| 132 | Strict/open boundary | Dashed boundary for < or > | graph.c | PARTIALLY IMPLEMENTED | Code inspection | Coarse alternating cells |
| 133 | Compound solution regions | Robust intersection/union shading | — | NOT IMPLEMENTED | None | Separate overlapping rows only |
| 134 | Symbolic inequality solve | Exact interval solution | — | NOT IMPLEMENTED | None | Numerical plotting is not solving |

## 135–154: Statistics

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 135 | Mean | mean(L) | eval.c | IMPLEMENTED AND TESTED | Core mean=4 | 32 values |
| 136 | Median | median(L) | eval.c | IMPLEMENTED AND TESTED | Core median=3 | Sorted copy |
| 137 | Mode | mode(L) | — | NOT IMPLEMENTED | None | No builtin |
| 138 | Minimum | min(L) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | 32 values |
| 139 | Maximum | max(L) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | 32 values |
| 140 | Range | max(L)-min(L) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Composed operations | No range builtin |
| 141 | Total | total(L) | eval.c | IMPLEMENTED AND TESTED | Core range totals | Binary64 accumulation |
| 142 | Count | count(L) | eval.c | IMPLEMENTED AND TESTED | Core unique count | Nonempty scalar lists |
| 143 | Sample standard deviation | stdev(L) | eval.c | IMPLEMENTED AND TESTED | Core sqrt(7) | At least two observations |
| 144 | Population standard deviation | stdevp(L) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Implemented denominator branch | No dedicated branch assertion |
| 145 | Sample variance | var(L) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Shared Welford calculation | At least two observations |
| 146 | Population variance | varp(L) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Shared Welford calculation | Nonempty list |
| 147 | Quantile | quantile(L,p) | eval.c | IMPLEMENTED AND TESTED | Core p=.9 -> 6.8 | Interpolated p(n-1) convention |
| 148 | Quartiles | quartile(L,k) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Quantile implementation | Convention may differ |
| 149 | Interquartile range | quartile(L,3)-quartile(L,1) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Composed quantiles | No dedicated builtin |
| 150 | Correlation | corr(A,B) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Implemented paired calculation | Constant series undefined |
| 151 | Sample covariance | cov(A,B) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Implemented paired calculation | Equal-length lists |
| 152 | Population covariance | covp(A,B) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Implemented paired calculation | Equal-length lists |
| 153 | Weighted statistics | Weighted mean/variance | — | NOT IMPLEMENTED | None | No weights API |
| 154 | Inference statistics | Tests/intervals/p-values | — | NOT IMPLEMENTED | None | No inference engine |

## 155–160: Statistical visualization

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 155 | Histogram | histogram(L) | — | NOT IMPLEMENTED | None | No bin renderer |
| 156 | Box plot | boxplot(L) | — | NOT IMPLEMENTED | None | No renderer |
| 157 | Dot plot | dotplot(L) | — | NOT IMPLEMENTED | None | Scatter table is different |
| 158 | Normal probability plot | qqplot(L) or equivalent | — | NOT IMPLEMENTED | None | No distribution engine |
| 159 | Residual plot | Plot residuals against x | — | NOT IMPLEMENTED | None | Numeric residual display only |
| 160 | Statistical fit overlay | Plot fitted model with data | graph.c | PARTIALLY IMPLEMENTED | Preview linear fit | Supported direct table models |

## 161–178: Lists and list manipulation

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 161 | Literal lists | [1,2,3] | parser.c, eval.c | IMPLEMENTED AND TESTED | Core | 32 real scalars |
| 162 | Named lists | L=[1,3,5] | model.c, eval.c | IMPLEMENTED AND TESTED | Core | Single-letter/subscripted names |
| 163 | Integer ranges | [1...10] | parser.c, eval.c | IMPLEMENTED AND TESTED | Core total=55 | At most 32 elements |
| 164 | Stepped ranges | [1,3...11] | parser.c, eval.c | IMPLEMENTED AND TESTED | Core total=36 | Finite nonzero step |
| 165 | One-based indexing | L[2] | eval.c | IMPLEMENTED AND TESTED | Core | Invalid indices rejected |
| 166 | Scalar broadcasting | 2*[1,2,3] | eval.c | IMPLEMENTED AND TESTED | Core list power/broadcast paths | No nested values |
| 167 | Elementwise list arithmetic | A+B, A*B | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Implemented vector branches | Equal lengths or scalar |
| 168 | List powers | [1,2,3]^2 | eval.c | IMPLEMENTED AND TESTED | Core; Robust | Real-domain rules |
| 169 | Multiple graph family | y=2x+[1,2,3,4] | graph.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Preview four lines | Shared row color |
| 170 | Sort | sort(L) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Shared qsort path | Ascending only |
| 171 | Unique values | unique(L) | eval.c | IMPLEMENTED AND TESTED | Core count=4 | Sorted unique result |
| 172 | Join | join(A,B) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | Combined length <=32 |
| 173 | Reverse | reverse(L) | — | NOT IMPLEMENTED | None | No list reorder function |
| 174 | Shuffle / sampling | shuffle(L), sample(L) | — | NOT IMPLEMENTED | None | No RNG interface |
| 175 | Conditional filtering | L[L>2] | — | NOT IMPLEMENTED | Explicit index restrictions | Scalar numeric index only |
| 176 | Comprehensions | [f(k) for k=...] | — | NOT IMPLEMENTED | None | Sum is not comprehension |
| 177 | List slicing / index lists | L[1...3] | — | NOT IMPLEMENTED | Index must be scalar | No slice value |
| 178 | Nested lists / general map | Lists of lists/points | — | NOT IMPLEMENTED | Explicit scalar-entry restriction | Fixed scalar Value representation |

## 179–206: Arithmetic, number theory and combinatorics

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 179 | Addition | 2+3 | eval.c | IMPLEMENTED AND TESTED | Core | Binary64 |
| 180 | Subtraction | 5-2 | eval.c | IMPLEMENTED AND TESTED | Core | Binary64 |
| 181 | Multiplication | 3*4 | eval.c | IMPLEMENTED AND TESTED | Core | Binary64 |
| 182 | Division | 1/4 | eval.c | IMPLEMENTED AND TESTED | Core; Editor | Zero division rejected |
| 183 | Operator precedence | 2+3*4 | parser.c | IMPLEMENTED AND TESTED | Core=14 | Bounded AST |
| 184 | Powers | 2^3^2 | parser.c, eval.c | IMPLEMENTED AND TESTED | Core=512 | Real powers only |
| 185 | Negative exponents | 2^-2 | parser.c, eval.c | IMPLEMENTED AND TESTED | Core=.25 | Zero base domain |
| 186 | Unary signs | -2^2 | parser.c | IMPLEMENTED AND TESTED | Core=-4 | Follows documented precedence |
| 187 | Square root | sqrt(81) | eval.c | IMPLEMENTED AND TESTED | Core=9 | Negative input rejected |
| 188 | Cube root | cbrt(-8) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Native/host library call | No dedicated assertion |
| 189 | Nth root | nthroot(value,index) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | Negative radicand requires odd integer index |
| 190 | Absolute value | abs(-3) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | Real scalars/lists |
| 191 | Natural exponential | exp(x) | eval.c | IMPLEMENTED AND TESTED | Ref | Overflow rejected |
| 192 | Natural logarithm | ln(e^3) | eval.c | IMPLEMENTED AND TESTED | Core=3 | Positive real domain |
| 193 | Base-10 logarithm | log(100) | eval.c | IMPLEMENTED AND TESTED | Core=2 | Positive real domain |
| 194 | Arbitrary-base logarithm | log(value,base) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | Not log-subscript syntax |
| 195 | Floor | floor(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | Binary64 input |
| 196 | Ceiling | ceil(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | Binary64 input |
| 197 | Rounding | round(x[,digits]) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | Digit magnitude <=12 |
| 198 | Sign | sign(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | -1,0,1 |
| 199 | Modulo | mod(x,y) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | Floor-based convention |
| 200 | GCD | gcd(24,36) | eval.c | IMPLEMENTED AND TESTED | Core=12 | Bounded integer inputs |
| 201 | LCM | lcm(12,18) | eval.c | IMPLEMENTED AND TESTED | Core=36 | Binary64 integer exactness bound |
| 202 | Factorial | 6! | eval.c | IMPLEMENTED AND TESTED | Core=720 | Integer 0..170 |
| 203 | Combinations | nCr(6,2) | eval.c | IMPLEMENTED AND TESTED | Core=15 | Integer n<=170 |
| 204 | Permutations | nPr(n,r) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Shared combinatorics branch | Integer n<=170 |
| 205 | Constants/scientific notation | pi, e, tau, 1e-6 | parser.c, model.c | IMPLEMENTED AND TESTED | Core; Robust formatting | No dedicated EE key |
| 206 | Exact fractions / rationalize | Exact symbolic fraction output | — | NOT IMPLEMENTED | None | Structured fraction input is approximate |

## 207–222: Trigonometry

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 207 | Sine | sin(pi/2) | eval.c | IMPLEMENTED AND TESTED | Core=1 | Angle mode |
| 208 | Cosine | cos(x) | eval.c | IMPLEMENTED AND TESTED | Robust identity round-trip | Angle mode |
| 209 | Tangent | tan(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Library call | Near-pole numeric sensitivity |
| 210 | Cosecant | csc(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Reciprocal sine branch | Zeros undefined |
| 211 | Secant | sec(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Reciprocal cosine branch | Zeros undefined |
| 212 | Cotangent | cot(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Reciprocal tangent branch | Zeros undefined |
| 213 | Arcsine | asin(x), arcsin(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code branch | Input [-1,1] |
| 214 | Arccosine | acos(x), arccos(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code branch | Input [-1,1] |
| 215 | Arctangent | atan(x), arctan(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code branch | Principal value |
| 216 | Arccosecant | arccsc(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | asin(1/x) branch | Principal convention |
| 217 | Arcsecant | arcsec(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | acos(1/x) branch | Principal convention |
| 218 | Arccotangent | arccot(x) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | atan2(1,x) branch | Convention documented by code |
| 219 | Degrees/radians | MODE | app.c, eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Implemented conversion | No per-expression unit flag |
| 220 | Hyperbolic functions | sinh, cosh, tanh | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Library branches | Overflow/domain limits |
| 221 | Inverse hyperbolic | asinh, acosh, atanh | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Library branches | Real domain |
| 222 | Trig solving / identities | Exact trig solution families | — | NOT IMPLEMENTED | None | Numerical graph/fit alternatives only |

## 223–234: Geometry

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 223 | Coordinate points | (x,y) | parser.c, eval.c | IMPLEMENTED AND TESTED | Core distance inputs | Two scalar components |
| 224 | Distance | distance((0,0),(3,4)) | eval.c | IMPLEMENTED AND TESTED | Core=5 | Euclidean 2D only |
| 225 | Midpoint | midpoint((0,0),(2,4)) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Code path | Returns point Value |
| 226 | Slope | (y2-y1)/(x2-x1) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Arithmetic components | Manual expression; no builtin |
| 227 | Circle | (x-h)^2+(y-k)^2=r^2 | graph.c | PARTIALLY IMPLEMENTED | Preview | Coarse implicit renderer |
| 228 | Ellipse | x^2/a^2+y^2/b^2=1 | graph.c | PARTIALLY IMPLEMENTED | Shared implicit path | No dedicated geometric test |
| 229 | Hyperbola | x^2/a^2-y^2/b^2=1 | graph.c | PARTIALLY IMPLEMENTED | Shared implicit path | Asymptote/cell limitations |
| 230 | General conics | Quadratic implicit relation | graph.c | PARTIALLY IMPLEMENTED | Shared implicit path | No symbolic classification |
| 231 | Line segments | Line plus domain restriction | graph.c | PARTIALLY IMPLEMENTED | Robust restrictions | No segment object/endpoints UI |
| 232 | Polygon | polygon(points) | — | NOT IMPLEMENTED | None | No polygon renderer |
| 233 | Area/perimeter tools | Geometry object measurements | — | NOT IMPLEMENTED | None | Manual formulas possible |
| 234 | Geometric transformations | Rotate/reflect/construct objects | — | NOT IMPLEMENTED | None | Formula-level changes only |

## 235–247: Probability distributions

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 235 | Distribution objects | normaldist(...) | — | NOT IMPLEMENTED | None | No distribution Value type |
| 236 | Normal density | Normal PDF | — | NOT IMPLEMENTED | None | A recognized token is not implementation |
| 237 | Normal cumulative | Normal CDF | — | NOT IMPLEMENTED | None | No distribution routine |
| 238 | Inverse normal | Quantile/inverse CDF | — | NOT IMPLEMENTED | None | List quantile is different |
| 239 | Binomial probability | Binomial PMF | — | NOT IMPLEMENTED | None | nCr alone is not a distribution API |
| 240 | Binomial cumulative | Binomial CDF | — | NOT IMPLEMENTED | None | No routine |
| 241 | Poisson distribution | PMF/CDF | — | NOT IMPLEMENTED | None | No routine |
| 242 | Uniform distribution | PDF/CDF | — | NOT IMPLEMENTED | None | No distribution object |
| 243 | Student t | PDF/CDF/quantile | — | NOT IMPLEMENTED | None | No special-function engine |
| 244 | Chi-square | PDF/CDF/quantile | — | NOT IMPLEMENTED | None | No special-function engine |
| 245 | Geometric/exponential distributions | Probability models | — | NOT IMPLEMENTED | None | exp() is only arithmetic |
| 246 | Random samples | random/sample from distributions | — | NOT IMPLEMENTED | None | No user RNG |
| 247 | Probability shading | Tail/interval plot | — | NOT IMPLEMENTED | None | Inequality plotting is different |

## 248–273: Calculus, matrices, complex numbers and advanced functions

| # | Feature | Expected syntax / behavior | Module | Status | Tests / evidence | Hardware / scope limits |
|---|---|---|---|---|---|---|
| 248 | Numerical derivative | diff(x^3,x,2) | eval.c | IMPLEMENTED AND TESTED | Core=12 | Central finite difference |
| 249 | Derivative graph | y=diff(f(t),t,x) | eval.c, graph.c | PARTIALLY IMPLEMENTED | Shared evaluation path | No derivative-graph reference test |
| 250 | Higher derivatives | Nested derivative operations | eval.c | PARTIALLY IMPLEMENTED | Nested binding support | No accuracy guarantee; depth limits |
| 251 | Definite integral | int(x^2,x,0,3) | eval.c | IMPLEMENTED AND TESTED | Core=9 | Bounded Simpson refinement |
| 252 | Improper/singular integrals | Infinite or singular bounds | — | NOT IMPLEMENTED | None | No robust improper quadrature |
| 253 | Symbolic differentiation | Exact f-prime expression | — | NOT IMPLEMENTED | None | No symbolic algebra |
| 254 | Symbolic integration | Antiderivative | — | NOT IMPLEMENTED | None | No CAS |
| 255 | Finite summation | sum(k^2,k,1,5) | eval.c | IMPLEMENTED AND TESTED | Core=55; Robust nested scopes | Bounded integer range/work |
| 256 | Finite product | prod(k,k,1,5) | eval.c | IMPLEMENTED BUT UNVERIFIED ON HARDWARE | Shared bounded-loop branch | No dedicated numeric assertion |
| 257 | Limits | Symbolic/numerical limit object | — | NOT IMPLEMENTED | None | Graph inspection is not a limit solver |
| 258 | Tangent/normal tools | Automatic tangent line | — | NOT IMPLEMENTED | None | Manual derivative formula possible |
| 259 | Matrix entry | Rectangular matrix values | — | NOT IMPLEMENTED | None | No matrix Value type |
| 260 | Matrix arithmetic | +, product, scalar operations | — | NOT IMPLEMENTED | None | Solver scratch matrices are internal |
| 261 | Determinant | det(A) | — | NOT IMPLEMENTED | None | No public matrix API |
| 262 | Matrix inverse | A^-1 | — | NOT IMPLEMENTED | None | Scalar powers only |
| 263 | Transpose | transpose(A) | — | NOT IMPLEMENTED | None | No public matrix API |
| 264 | Row reduction | rref(A) | — | NOT IMPLEMENTED | None | QR solver does not expose RREF |
| 265 | Eigenvalues/eigenvectors | eig(A) | — | NOT IMPLEMENTED | None | No matrix API |
| 266 | Complex literals/arithmetic | a+b*i | — | NOT IMPLEMENTED | None | i is an ordinary variable name |
| 267 | Complex components | real, imag, conjugate, arg | — | NOT IMPLEMENTED | None | Real-only Value |
| 268 | Complex graphing | Complex plane/functions | — | NOT IMPLEMENTED | None | Real x/y graph only |
| 269 | Recursive sequences | a_n from prior terms | — | NOT IMPLEMENTED | Cycle rejection tested | Cycles are errors, not recurrences |
| 270 | Prime/factorization utilities | isprime, factor | — | NOT IMPLEMENTED | None | GCD/LCM only |
| 271 | Special functions | gamma, beta, erf | — | NOT IMPLEMENTED | None | No public routines |
| 272 | 3D/vector calculus | 3D plots, gradients, surfaces | — | NOT IMPLEMENTED | None | Not labelled impossible on hardware |
| 273 | Actions/tickers/programming | Timed actions and expression programs | — | NOT IMPLEMENTED | None | Fixed calculator application state machine |

## Additional UI and substitution requirements

The keymap documents ALPHA/alpha-lock, inverse-trig shortcuts, comparison/bracket insertion, `~`, x_1/y_1 templates, row selection, whole-expression clipboard, visibility, fit-result access and save/exit. Parser substitution supports definitions and single-argument function calls (rows 19, 20, 48, 62, 67 and 68). Automatic answer substitution (`ans`), arbitrary argument signatures, rich text, notes, folders, search, custom styling, animations and cloud sharing are not implemented. No touchscreen behavior is claimed. See limitations.md for scope/inference and precision caveats.
