# Implemented syntax and behavior

This document describes application code, not full Desmos compatibility. All calculator-side behavior still needs device validation. The numbered [matrix](feature-matrix.md) distinguishes implemented, partial, missing and hardware-inapplicable capabilities.

## Expressions and values

Use arithmetic `+ - * / ^`, parentheses, unary signs and factorial. Power is right-associative: `2^3^2=512`; unary minus follows power: `-2^2=-4`. Implicit multiplication supports `3x`, `2(x+1)` and adjacent parenthesized groups. Single-letter identifiers concatenate as multiplication, while known function names and subscripted names are tokens. Subscript syntax is `x_1`, not arbitrary multi-letter bare identifiers.

Numbers use binary64 arithmetic. Display is rounded separately, normally to seven significant digits. Scientific notation is accepted by the parser, but there is no dedicated EE keypad shortcut. Exact rational, symbolic or arbitrary-precision results are not implemented. Domain failures return an error rather than complex output.

Definitions include `a=5`, `f(x)=x^2+3x-4`, `f(3)` and `f(g(0))`. User functions have one argument. Variable/function cycles are rejected. Multi-argument user functions and implicit inverse functions are absent. Parameter inference is intentionally limited: see [limitations](limitations.md) before using parameters hidden inside other definitions.

## Functions

- Real arithmetic: `sqrt`, `cbrt`, `nthroot(value,index)`, `abs`, `exp`, `ln`, `log(value)` and `log(value,base)`; `floor`, `ceil`, `round(value[,digits])`, `sign`, `mod`.
- Integer operations: `gcd`, `lcm`, postfix `!`, `nCr(n,r)`, `nPr(n,r)`. Combinatorics inputs are integers with `n<=170`; GCD/LCM operands are bounded integers. Overflow remains an error.
- Trigonometry: `sin`, `cos`, `tan`, `sec`, `csc`, `cot`; `asin/arcsin`, `acos/arccos`, `atan/arctan`, `arcsec`, `arccsc`, `arccot`; `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`. MODE selects radians/degrees for circular trig.
- Constants: `pi`, `tau`, `e`. Uppercase `L` is a normal user-defined name. `ans` is not implemented.

## Lists and statistics

`L=[1,3,3,5,8]` creates a list; `L[2]` is one-based indexing. Lists hold at most 32 real scalars. `[1...10]` and `[1,3...11]` create ranges. Scalar/list arithmetic broadcasts; incompatible list lengths are rejected. `y=2x+[1,2,3,4]` draws four curves.

Statistics: `mean(L)`, `median(L)`, `total(L)`, `count(L)`, `min(L)`, `max(L)`, `stdev(L)`/`stdevp(L)`, `var(L)`/`varp(L)`. Sample variants use n-1. `quantile(L,p)` uses linearly interpolated rank p(n-1); `quartile(L,k)` uses p=k/4. Quantile conventions can differ across tools; no claim of universal convention parity is made. `corr(A,B)`, `cov(A,B)`, `covp(A,B)` require paired lists. `sort`, `unique`, and `join` return lists.

No comprehension/filter syntax, nested lists, random sampling, weighted statistics, mode, histogram or box plot is implemented. Point values `(x,y)` exist, but lists of point values do not.

## Graphing

Explicit `y=f(x)` and user function definitions can be drawn. Relations such as `(x-2)^2+(y+1)^2=25`, `x=3` and inequalities use a coarse implicit renderer. `{condition:value,...}` is a scalar piecewise expression. Postfix restrictions such as `y=x^2{0<x<5}` are supported. Comparisons include `< <= > >= =`, chained ordered comparisons, `and` and `or`.

Pan, zoom, grid toggle, editable bounds, table zoom-fit, graph-focused mode, expression visibility and color assignment are present. Colors are assigned from a fixed six-color cycle; there is no style editor. Explicit sampling uses midpoint checks to avoid ordinary pole bridges, not a complete discontinuity proof. Implicit cell spacing is eight pixels and can miss small components or resolve saddle cells imperfectly.

TRACE locates numerical roots, y-intercepts, extrema and pairwise intersections of scalar explicit curves within the current x-window. It does not prove all solutions were found, does not analyze implicit/list/fitted curves generally, and does not prove an extremum is global.

## Regression and scalar solving

Examples:

```text
2k+5~17
7~3m+1
y_1~mx_1+b
y_1~ax_1^2+bx_1+c
y_1~a*exp(b*x_1)
y_1~a*x_1^b
y_1~a*ln(x_1)+b
y_1~a*sin(b*x_1+c)+d
```

Up to six directly inferred unknowns and 48 combined residuals are supported. Several `~` rows form one joint residual problem. Defined values and sliders are fixed, not newly fitted. Converged coefficients are reusable in later expressions.

Affine-in-parameter models use scaled column-pivoted Householder QR. Nonlinear models use finite-difference Jacobians and damped augmented QR, four deterministic starting points and at most 64 iterations per start. This is local numerical fitting, not CAS or a guarantee of global/unique solutions. Cancellation, bad domains, underdetermination, rank deficiency and nonconvergence have separate statuses. Failed estimates are not installed as usable fitted coefficients.

RMSE and residuals are shown in the details panel. R-squared is calculated only for a single regression whose left side is exactly `y_1` with nonconstant observations. No weighted fit, parameter bounds, confidence intervals, manual starting guesses, statistical significance tests or log-mode fitting is provided.

## Numerical calculus and geometry

`diff(x^3,x,2)` uses a central finite difference. `int(x^2,x,0,3)` uses successively refined composite Simpson integration with a bounded convergence check. `sum(k^2,k,1,5)` and `prod(k,k,1,5)` use finite integer bounds and work limits. These are function-call inputs, not full typeset calculus templates. Improper integrals, symbolic derivatives and guaranteed quadrature error bounds are absent.

`distance((0,0),(3,4))` returns 5; `midpoint((0,0),(2,4))` returns a point. Circles/conics can be entered as implicit relations, subject to renderer limitations. Polygon tools, geometric construction/dragging, parametric/polar plotting and vector/matrix operations are not implemented.

## Saving

`2nd MODE` saves a session, then returns to the OS. Expressions, table, sliders, angle mode and graph window are serialized. Fitted coefficients are recomputed from saved expressions/data. Undo history, clipboard and current cursor are not persisted. See [architecture](architecture.md) for checksum/slot behavior and [testing](testing.md) for the unverified on-device portion.
