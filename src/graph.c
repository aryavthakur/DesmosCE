#include "ui.h"
#include <math.h>
#include <string.h>
static int clamp_pixel(Real x) {
    return (int)fmaxl(-4096, fminl(4096, x));
}
int graph_px(const Session *s, Real x, int l, int w) {
    return clamp_pixel(l + (x - s->xmin) / (s->xmax - s->xmin) * (w - 1));
}
int graph_py(const Session *s, Real y, int t, int h) {
    return clamp_pixel(t + (s->ymax - y) / (s->ymax - s->ymin) * (h - 1));
}
Real graph_x(const Session *s, int p, int l, int w) {
    return s->xmin + (Real)(p - l) / (w - 1) * (s->xmax - s->xmin);
}
Real graph_y(const Session *s, int p, int t, int h) {
    return s->ymax - (Real)(p - t) / (h - 1) * (s->ymax - s->ymin);
}
static int base_root(const Program *p) {
    int r = p->root;
    while (r >= 0 && p->n[r].kind == N_IF && p->n[r].c < 0)
        r = p->n[r].b;
    return r;
}
static bool contains_y(const Program *p, int r) {
    if (r < 0)
        return false;
    const Node *n = &p->n[r];
    if (n->kind == N_VAR && !strcmp(n->name, "y"))
        return true;
    return contains_y(p, n->a) || contains_y(p, n->b) || contains_y(p, n->c);
}
bool graph_explicit(Model *m, unsigned row) {
    Program *p = &m->p[row];
    int r = base_root(p);
    if (r < 0)
        return false;
    Node *n = &p->n[r];
    if (n->kind == N_EQ) {
        Node *lhs = &p->n[n->a];
        return ((lhs->kind == N_VAR && !strcmp(lhs->name, "y")) ||
                dce_function_arg(p, n->a) >= 0) &&
               !contains_y(p, n->b);
    }
    if (n->kind == N_REG) {
        if (!m->fits)
            return false;
        for (unsigned i = 0; i < p->used; i++)
            if (p->n[i].kind == N_VAR &&
                (!strcmp(p->n[i].name, "x_1") || !strcmp(p->n[i].name, "y_1")))
                return true;
        return false;
    }
    if (n->kind >= N_LT && n->kind <= N_GE)
        return false;
    if (n->kind == N_POINT)
        return false;
    return !contains_y(p, r);
}
static bool explicit_value(Model *m, unsigned row, Real x, Value *v) {
    Program *p = &m->p[row];
    int base = base_root(p);
    if (base < 0)
        return false;
    Node *n = &p->n[base];
    static Eval e;
    e = dce_context(m, x, 0);
    Binding binding = {NULL, x, NULL};
    int target = base;
    Parameter bindings[1];
    if (n->kind == N_REG) {
        memset(bindings, 0, sizeof(bindings));
        strcpy(bindings[0].name, "x_1");
        bindings[0].value = x;
        e.params = bindings;
        e.count = 1;
        target = n->b;
    } else if (n->kind == N_EQ) {
        target = n->b;
        Node *lhs = &p->n[n->a];
        if (lhs->kind == N_CALL && lhs->a >= 0) {
            int arg = p->n[lhs->a].a;
            if (p->n[arg].kind == N_VAR) {
                binding.name = p->n[arg].name;
                e.bindings = &binding;
            }
        }
    }
    if (!dce_eval(&e, p, target, v))
        return false;
    /* Top-level restrictions are checked using the computed y value. */
    for (int r = p->root; r != base; r = p->n[r].b) {
        static Value c;
        e.y = v->v[0];
        if (!dce_eval(&e, p, p->n[r].a, &c) || c.len != 1 || !c.v[0])
            return false;
    }
    return !v->point;
}
static bool implicit_value(Model *m, unsigned row, Real x, Real y, Real *v, int *relation) {
    Program *p = &m->p[row];
    int r = base_root(p);
    if (r < 0)
        return false;
    static Eval e;
    e = dce_context(m, x, y);
    for (int q = p->root; q != r; q = p->n[q].b) {
        static Value c;
        if (!dce_eval(&e, p, p->n[q].a, &c) || c.len != 1 || !c.v[0])
            return false;
    }
    Node *n = &p->n[r];
    if (n->kind != N_EQ && (n->kind < N_LT || n->kind > N_GE))
        return false;
    static Value a, b;
    if (!dce_eval(&e, p, n->a, &a) || !dce_eval(&e, p, n->b, &b) || a.len != 1 || b.len != 1)
        return false;
    *relation = n->kind;
    *v = a.v[0] - b.v[0];
    return isfinite(*v);
}
static Real tick(Real span) {
    Real p = powl(10, floorl(log10l(span / 5))), r = span / (5 * p);
    return p * (r <= 1 ? 1 : r <= 2 ? 2 : r <= 5 ? 5 : 10);
}
static void explicit_curve(Model *m, Draw *d, unsigned row, int l, int t, int w, int h,
                           CancelFn cancel, void *user) {
    static Value prev, cur, mid;
    bool old = false;
    unsigned color = C_RED + m->s.row[row].color;
    for (int pixel = 0; pixel < w; pixel++) {
        if (pixel % 8 == 0 && cancel && cancel(user))
            return;
        Real x = graph_x(&m->s, l + pixel, l, w);
        bool ok = explicit_value(m, row, x, &cur);
        if (ok && old && cur.len == prev.len) {
            Real xm = graph_x(&m->s, l + pixel - 1, l, w) + (m->s.xmax - m->s.xmin) / (2 * (w - 1));
            bool halfway = explicit_value(m, row, xm, &mid) && mid.len == cur.len;
            for (unsigned k = 0; k < cur.len; k++) {
                Real y0 = prev.v[k],
                     y1 = cur.v[k]; /* Midpoint curvature guard prevents ordinary pole bridges. */
                if (!halfway || fabsl(mid.v[k] - (y0 + y1) / 2) > (m->s.ymax - m->s.ymin) * .04L)
                    continue;
                int a = graph_py(&m->s, y0, t, h), b = graph_py(&m->s, y1, t, h);
                if ((a < t && b < t) || (a >= t + h && b >= t + h))
                    continue;
                d->line(d->user, l + pixel - 1, a, l + pixel, b, color);
            }
        }
        if (ok)
            prev = cur;
        old = ok;
    }
}
static void implicit_curve(Model *m, Draw *d, unsigned row, int l, int t, int w, int h,
                           CancelFn cancel, void *user) {
    const int step = 8;
    unsigned color = C_RED + m->s.row[row].color;
    for (int yy = t; yy < t + h - 1; yy += step) {
        if (cancel && cancel(user))
            return;
        int ey = yy + step < t + h ? yy + step : t + h - 1;
        for (int xx = l; xx < l + w - 1; xx += step) {
            int ex = xx + step < l + w ? xx + step : l + w - 1, px[4] = {xx, ex, ex, xx},
                py[4] = {yy, yy, ey, ey};
            Real f[4];
            int kind = 0;
            bool ok = true;
            for (unsigned k = 0; k < 4; k++)
                if (!implicit_value(m, row, graph_x(&m->s, px[k], l, w),
                                    graph_y(&m->s, py[k], t, h), &f[k], &kind))
                    ok = false;
            if (!ok)
                continue;
            if (kind != N_EQ) {
                Real center;
                int rel;
                bool inside = implicit_value(m, row, graph_x(&m->s, (xx + ex) / 2, l, w),
                                             graph_y(&m->s, (yy + ey) / 2, t, h), &center, &rel) &&
                              ((kind == N_LT || kind == N_LE) ? center < 0 : center > 0);
                if (inside) {
                    for (int y = yy + 1; y < ey; y += 3)
                        for (int x = xx + 1; x < ex; x += 3)
                            d->rect(d->user, x, y, 1, 1, color);
                }
            }
            int crossx[4], crossy[4];
            unsigned count = 0;
            for (unsigned k = 0; k < 4; k++) {
                unsigned j = (k + 1) % 4;
                if ((f[k] < 0) == (f[j] < 0) && f[k] != 0 && f[j] != 0)
                    continue;
                if (f[k] == f[j])
                    continue;
                Real u = f[k] / (f[k] - f[j]);
                if (u < 0 || u > 1)
                    continue;
                Real wx = graph_x(&m->s, px[k], l, w) +
                          u * (graph_x(&m->s, px[j], l, w) - graph_x(&m->s, px[k], l, w)),
                     wy = graph_y(&m->s, py[k], t, h) +
                          u * (graph_y(&m->s, py[j], t, h) - graph_y(&m->s, py[k], t, h)),
                     check;
                int rel;
                /* Reject sign changes whose candidate residual gets worse (e.g. 1/x=0). */
                if (!implicit_value(m, row, wx, wy, &check, &rel) ||
                    fabsl(check) > fmaxl(fabsl(f[k]), fabsl(f[j])))
                    continue;
                crossx[count] = px[k] + (int)(u * (px[j] - px[k]));
                crossy[count] = py[k] + (int)(u * (py[j] - py[k]));
                count++;
            }
            if ((kind == N_LT || kind == N_GT) && ((xx + yy) / step) % 2)
                continue;
            for (unsigned k = 1; k < count; k += 2)
                d->line(d->user, crossx[k - 1], crossy[k - 1], crossx[k], crossy[k], color);
        }
    }
}
void graph_draw(Model *m, Draw *d, int l, int t, int w, int h, CancelFn cancel, void *user) {
    Session *s = &m->s;
    if (!dce_window_valid(s))
        return;
    d->clip(d->user, l, t, w, h);
    d->rect(d->user, l, t, w, h, C_WHITE);
    Real dx = tick(s->xmax - s->xmin), dy = tick(s->ymax - s->ymin);
    int ax = graph_px(s, 0, l, w), ay = graph_py(s, 0, t, h);
    char text[24];
    for (unsigned i = 0; i < 16; i++) {
        Real x = (ceill(s->xmin / dx) + i) * dx;
        if (x > s->xmax)
            break;
        int p = graph_px(s, x, l, w);
        if (s->grid)
            d->line(d->user, p, t, p, t + h - 1, C_GRID);
        if (x != 0) {
            dce_format(x, text, sizeof(text), 4);
            d->text(d->user, p + 2, ay >= t && ay < t + h - 10 ? ay + 3 : t + h - 10, text,
                    C_MUTED);
        }
    }
    for (unsigned i = 0; i < 16; i++) {
        Real y = (ceill(s->ymin / dy) + i) * dy;
        if (y > s->ymax)
            break;
        int p = graph_py(s, y, t, h);
        if (s->grid)
            d->line(d->user, l, p, l + w - 1, p, C_GRID);
        if (y != 0) {
            dce_format(y, text, sizeof(text), 4);
            d->text(d->user, ax >= l && ax < l + w - 30 ? ax + 3 : l + 3, p + 2, text, C_MUTED);
        }
    }
    if (ax >= l && ax < l + w)
        d->line(d->user, ax, t, ax, t + h - 1, C_TEXT);
    if (ay >= t && ay < t + h)
        d->line(d->user, l, ay, l + w - 1, ay, C_TEXT);
    for (unsigned i = 0; i < s->rows; i++)
        if (s->row[i].visible && m->p[i].root >= 0) {
            Program *p = &m->p[i];
            int r = base_root(p);
            Node *n = &p->n[r];
            if (n->kind == N_EQ && p->n[n->a].kind == N_VAR && strcmp(p->n[n->a].name, "y") &&
                strcmp(p->n[n->a].name, "x"))
                continue;
            if (n->kind == N_POINT) {
                Value v;
                Eval e = dce_context(m, 0, 0);
                if (dce_eval(&e, p, r, &v) && v.len == 2)
                    d->rect(d->user, graph_px(s, v.v[0], l, w) - 2, graph_py(s, v.v[1], t, h) - 2,
                            5, 5, C_RED + s->row[i].color);
                continue;
            }
            if (graph_explicit(m, i))
                explicit_curve(m, d, i, l, t, w, h, cancel, user);
            else
                implicit_curve(m, d, i, l, t, w, h, cancel, user);
            if (cancel && cancel(user))
                break;
        }
    for (unsigned i = 0; i < s->points; i++)
        d->rect(d->user, graph_px(s, s->x[i], l, w) - 2, graph_py(s, s->y[i], t, h) - 2, 4, 4,
                C_BLUE);
    d->clip(d->user, 0, 0, 320, 240);
}
