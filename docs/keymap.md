# Physical keymap

Press modifier keys first, then the following key. A modifier is normally one-shot. `2nd ALPHA` toggles alpha lock. Letters follow the calculator's ALPHA legends but are inserted lowercase; `APPS`/`PRGM` produce `b`/`c` under ALPHA.

## Global navigation

| Key | Action |
|---|---|
| Y= | Apply current expression; toggle split and expression-focused views |
| GRAPH | Apply current expression; graph-focused view |
| STAT | Data table, columns x_1 and y_1 |
| WINDOW | Editable graph bounds |
| TRACE | Find/cycle scalar explicit special points |
| MODE | Toggle radians/degrees |
| ZOOM | Fit window to entered table data |
| MATH | Template/function palette while editing |
| 2nd MODE | Apply edits, save session and exit |
| 2nd WINDOW | Slider panel |
| 2nd ENTER | Regression details and residuals |
| 2nd ZOOM | Reset both axes to [-10,10] |
| 2nd STAT | Replace current expression with a linear-fit template |
| 2nd TRACE | Toggle current expression visibility |

## Equation editing

| Key | Action |
|---|---|
| ENTER | Apply, then next/new expression; at most ten rows |
| LEFT / RIGHT | Navigate within and between mathematical slots |
| UP / DOWN | Move between fraction or exponent/subscript slots |
| DEL | Delete preceding structural item; at a slot boundary navigate outward |
| CLEAR | Clear current expression |
| 2nd UP / DOWN | Apply and select previous/next expression |
| 2nd LEFT / RIGHT | Undo / redo, ten snapshots per active expression |
| 2nd DEL | Duplicate current expression |
| 2nd CLEAR | Delete current expression row |
| 2nd + / 2nd - | Copy / replace from whole-expression clipboard |
| STO | Equals sign `=` |
| 2nd STO | Regression operator `~` |
| X,T,theta,n | `x` |
| 2nd X,T,theta,n | Uppercase `L` for a list |
| Divide | Fraction around preceding atom/group; enter denominator |
| Power | Power around preceding atom/group; enter exponent |
| x-squared | Insert exponent 2 and leave the exponent |
| Reciprocal | Insert exponent -1 and leave the exponent |
| 2nd x-squared | Square-root template |
| 2nd Power | Subscript template |
| SIN / COS / TAN | Corresponding function call |
| 2nd SIN / COS / TAN | Inverse function call |
| LOG / LN | Base-10 / natural logarithm call |
| 2nd ( / 2nd ) | `[` / `]` |
| 2nd multiply / 2nd divide | `<` / `>` |
| 2nd decimal point | `:` |

`MATH` offers `~`, `=`, fraction/root/power/subscript templates, x_1/y_1, brackets/braces, comparisons, constants, functions and fit templates. UP/DOWN selects; LEFT/RIGHT jumps eight entries; ENTER inserts; CLEAR returns. Fractions, exponents and subscripts remain structural until RIGHT moves back to the surrounding expression. MATH x_1/y_1 inserts a complete subscript and leaves its slot automatically.

**Example `2k+5~17`:** type 2; ALPHA then left parenthesis for k; + 5; 2nd STO; 1 7; ENTER. The result is computed rather than pasted from a template. `2nd ENTER` opens fit details. To type `y`, use ALPHA then 1.

**Example fraction:** enter `(x^2+1)` first, move outside the exponent with RIGHT, then divide and enter `(x-3)`. The preceding parenthesized group becomes the numerator. Use UP/DOWN to revisit numerator/denominator.

## Graph, table and settings

In graph view, arrows pan and + / - zoom. During special-point tracing, LEFT/RIGHT cycles points; CLEAR leaves tracing. CLEAR is also checked during bounded graph/fit loops to request cancellation.

In the table, arrows select a cell. Start typing to replace its contents, then ENTER commits and advances column/row. `2nd DEL` deletes the selected data row. CLEAR cancels the current cell edit. New rows default the other numeric cell to zero; missing-data cells are not supported.

In window settings, UP/DOWN selects a bound; typing and ENTER replaces it. Each commit must keep maximum greater than minimum. MATH toggles grid visibility.

In sliders, UP/DOWN selects a parameter and LEFT/RIGHT changes its value by the configured step. MATH cycles value/minimum/maximum/step. Typing and ENTER replaces that field. There is no slider animation or touch dragging.

In regression details, UP/DOWN scrolls residuals and CLEAR returns. A result labelled local numerical convergence is not a proof of uniqueness.
