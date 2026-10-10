#include "ui.h"
#include <math.h>
static bool value(Model *m, unsigned row, int other, int derivative, Real x, Real *y) {
    if (derivative) {
        Real h = 1e-5L * (1 + fabsl(x)), a, b;
        if (!dce_scalar(m, row, x + h, 0, &a) || !dce_scalar(m, row, x - h, 0, &b))
            return false;
        *y = (a - b) / (2 * h);
        return isfinite(*y);
    }
    if (!dce_scalar(m, row, x, 0, y))
        return false;
    if (other >= 0) {
        Real z;
        if (!dce_scalar(m, other, x, 0, &z))
            return false;
        *y -= z;
    }
    return isfinite(*y);
}
static bool root(Model *m, unsigned row, int other, int deriv, Real a, Real b, Real *out) {
    Real fa, fb;
    if (!value(m, row, other, deriv, a, &fa) || !value(m, row, other, deriv, b, &fb) || fa * fb > 0)
        return false;
    for (unsigned k = 0; k < 50; k++) {
        Real c = (a + b) / 2, fc;
        if (!value(m, row, other, deriv, c, &fc))
            return false;
        if (fabsl(fc) < 1e-9L) {
            *out = c;
            return true;
        }
        if ((fa < 0) == (fc < 0)) {
            a = c;
            fa = fc;
        } else
            b = c;
    }
    *out = (a + b) / 2;
    Real y;
    return value(m, row, other, deriv, *out, &y) && fabsl(y) < 1e-6L;
}
static void add(Interest *out, unsigned *n, unsigned cap, Real x, Real y, unsigned kind,
                unsigned row) {
    for (unsigned i = 0; i < *n; i++)
        if (fabsl(out[i].x - x) < 1e-5L && fabsl(out[i].y - y) < 1e-5L && out[i].kind == kind)
            return;
    if (*n < cap) {
        out[*n] = (Interest){x, y, (uint8_t)kind, (uint8_t)row};
        (*n)++;
    }
}
unsigned dce_analyze(Model *m, Interest *out, unsigned cap, CancelFn cancel, void *user) {
    unsigned n = 0;
    Real lo = m->s.xmin, hi = m->s.xmax;
    for (unsigned row = 0; row < m->s.rows; row++) {
        if (!m->s.row[row].visible || !graph_explicit(m, row))
            continue;
        Real y;
        if (dce_scalar(m, row, 0, 0, &y))
            add(out, &n, cap, 0, y, 1, row);
        for (int mode = -2; mode < (int)row; mode++) {
            if (mode >= 0 && (!graph_explicit(m, mode) || !m->s.row[mode].visible))
                continue;
            for (unsigned k = 0; k < 128; k++) {
                if (k % 8 == 0 && cancel && cancel(user))
                    return n;
                Real x, a = lo + (hi - lo) * k / 128, b = lo + (hi - lo) * (k + 1) / 128;
                if (root(m, row, mode >= 0 ? mode : -1, mode == -1, a, b, &x) &&
                    dce_scalar(m, row, x, 0, &y)) {
                    add(out, &n, cap, x, y, mode == -2 ? 0 : mode == -1 ? 2 : 3, row);
                    if (mode == -1 && fabsl(y) < 1e-7L)
                        add(out, &n, cap, x, 0, 0, row);
                }
            }
        }
    }
    return n;
}
