#include "dce.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
static bool error(Eval *e, const char *s) {
    if (!e->error[0]) {
        strncpy(e->error, s, sizeof(e->error) - 1);
    }
    return false;
}
static bool scalar(Value *v, Real x) {
    v->len = 1;
    v->point = 0;
    v->v[0] = x;
    return isfinite(x);
}
static bool variable(Eval *e, const char *name, Value *v) {
    for (const Binding *b = e->bindings; b; b = b->parent)
        if (!strcmp(name, b->name))
            return scalar(v, b->value);
    for (unsigned i = 0; i < e->count; i++)
        if (!strcmp(name, e->params[i].name))
            return scalar(v, e->params[i].value);
    Model *m = e->model;
    for (unsigned i = 0; i < m->s.sliders; i++)
        if (!strcmp(name, m->s.slider[i].name))
            return scalar(v, m->s.slider[i].value);
    for (unsigned i = 0; i < m->fits; i++)
        if (!strcmp(name, m->fit[i].name))
            return scalar(v, m->fit[i].value);
    if (!strcmp(name, "x"))
        return scalar(v, e->x);
    if (!strcmp(name, "y"))
        return scalar(v, e->y);
    if (!strcmp(name, "t") || !strcmp(name, "theta"))
        return scalar(v, e->t);
    if (!strcmp(name, "pi"))
        return scalar(v, DCE_PI);
    if (!strcmp(name, "tau"))
        return scalar(v, 2 * DCE_PI);
    if (!strcmp(name, "e"))
        return scalar(v, DCE_E);
    if (!strcmp(name, "x_1") || !strcmp(name, "y_1")) {
        v->len = m->s.points;
        v->point = 0;
        if (!v->len)
            return error(e, "Table is empty");
        memcpy(v->v, !strcmp(name, "x_1") ? m->s.x : m->s.y, v->len * sizeof(Real));
        return true;
    }
    for (unsigned i = 0; i < m->s.rows; i++) {
        Program *p = &m->p[i];
        if (p->root < 0)
            continue;
        Node *r = &p->n[p->root];
        if (r->kind != N_EQ || r->a < 0)
            continue;
        Node *a = &p->n[r->a];
        if (a->kind == N_VAR && !strcmp(name, a->name)) {
            if (e->active & (1u << i))
                return error(e, "Circular definition");
            e->active |= 1u << i;
            bool ok = dce_eval(e, p, r->b, v);
            e->active &= ~(1u << i);
            return ok;
        }
    }
    return error(e, "Undefined variable");
}
static int cmp(const void *a, const void *b) {
    Real x = *(const Real *)a, y = *(const Real *)b;
    return x < y ? -1 : x > y;
}
static Real quantile(Value a, Real p) {
    qsort(a.v, a.len, sizeof(Real), cmp);
    Real t = p * (a.len - 1);
    unsigned i = (unsigned)floorl(t);
    return a.v[i] + (i + 1 < a.len ? (t - i) * (a.v[i + 1] - a.v[i]) : 0);
}
static bool unary(Eval *e, const char *s, Real x, Real *y) {
    Real rad = e->model->s.degrees ? DCE_PI / 180 : 1;
#define F(name, code)                                                                              \
    if (!strcmp(s, name)) {                                                                        \
        *y = (Real)(code);                                                                         \
        return isfinite(*y);                                                                       \
    }
    F("sqrt", sqrtl(x))
    F("cbrt", cbrtl(x))
    F("abs", fabsl(x))
    F("exp", expl(x))
    F("ln", logl(x))
    F("log", log10l(x))
    F("sin", sinl(x * rad))
    F("cos", cosl(x * rad))
    F("tan", tanl(x * rad))
    F("sec", 1 / cosl(x * rad))
    F("csc", 1 / sinl(x * rad))
    F("cot", 1 / tanl(x * rad))
    F("asin", asinl(x) / rad)
    F("arcsin", asinl(x) / rad)
    F("acos", acosl(x) / rad)
    F("arccos", acosl(x) / rad)
    F("atan", atanl(x) / rad)
    F("arctan", atanl(x) / rad)
    F("arcsec", acosl(1 / x) / rad)
    F("arccsc", asinl(1 / x) / rad)
    F("arccot", atan2l(1, x) / rad)
    F("sinh", sinhl(x))
    F("cosh", coshl(x))
    F("tanh", tanhl(x))
    F("asinh", asinhl(x))
    F("acosh", acoshl(x))
    F("atanh", atanhl(x))
    F("floor", floorl(x))
    F("ceil", ceill(x))
    F("round", roundl(x))
    F("sign", (x > 0) - (x < 0))
#undef F
    return false;
}
static bool bound_eval(Eval *e, const Program *p, int n, const char *name, Real x, Real *r) {
    Binding b = {name, x, e->bindings};
    e->bindings = &b;
    Value v;
    bool ok = dce_eval(e, p, n, &v) && v.len == 1;
    e->bindings = b.parent;
    if (ok)
        *r = v.v[0];
    return ok;
}
static bool function(Eval *e, const Program *p, const Node *n, Value *v) {
    int ids[4];
    unsigned argc = 0;
    for (int a = n->a; a >= 0; a = p->n[a].b) {
        if (argc == 4)
            return error(e, "At most four arguments");
        ids[argc++] = p->n[a].a;
    }
    const char *s = n->name;
    if ((!strcmp(s, "sum") || !strcmp(s, "prod") || !strcmp(s, "int") || !strcmp(s, "diff")) &&
        argc >= 3) {
        if (p->n[ids[1]].kind != N_VAR)
            return error(e, "Expected bound variable");
        Value a, b;
        if (!dce_eval(e, p, ids[2], &a) || a.len != 1)
            return false;
        if (!strcmp(s, "diff")) {
            Real h = cbrtl(2.220446049250313e-16L) * (1 + fabsl(a.v[0])), f, g;
            if (!bound_eval(e, p, ids[0], p->n[ids[1]].name, a.v[0] + h, &f) ||
                !bound_eval(e, p, ids[0], p->n[ids[1]].name, a.v[0] - h, &g))
                return false;
            return scalar(v, (f - g) / (2 * h));
        }
        if (argc != 4 || !dce_eval(e, p, ids[3], &b) || b.len != 1)
            return error(e, "Expected four arguments");
        Real lo = a.v[0], hi = b.v[0];
        if (!strcmp(s,
                    "int")) { /* Composite Simpson with successive mesh refinement; bounded work. */
            Real prev = NAN;
            for (unsigned panels = 8; panels <= 256; panels *= 2) {
                Real h = (hi - lo) / panels, total = 0;
                for (unsigned k = 0; k <= panels; k++) {
                    Real f;
                    if (!bound_eval(e, p, ids[0], p->n[ids[1]].name, lo + k * h, &f))
                        return false;
                    total += f * (k == 0 || k == panels ? 1 : k % 2 ? 4 : 2);
                }
                total *= h / 3;
                if (isfinite(prev) && fabsl(total - prev) < 1e-8L * (1 + fabsl(total)))
                    return scalar(v, total);
                prev = total;
            }
            return error(e, "Integral did not converge");
        }
        if (lo != floorl(lo) || hi != floorl(hi) || hi - lo > 512)
            return error(e, "Sum/product bounds invalid");
        Real total = !strcmp(s, "prod") ? 1 : 0;
        for (Real k = lo; k <= hi; k++) {
            Real f;
            if (!bound_eval(e, p, ids[0], p->n[ids[1]].name, k, &f))
                return false;
            if (!strcmp(s, "prod"))
                total *= f;
            else
                total += f;
        }
        return scalar(v, total);
    }
    Value a, b;
    memset(&b, 0, sizeof(b));
    if (argc < 1 || !dce_eval(e, p, ids[0], &a))
        return false;
    if (argc > 1 && !dce_eval(e, p, ids[1], &b))
        return false;
    if (argc == 1) {
        Real sum = 0, mean = 0, m2 = 0;
        for (unsigned i = 0; i < a.len; i++) {
            sum += a.v[i];
            Real d = a.v[i] - mean;
            mean += d / (i + 1);
            m2 += d * (a.v[i] - mean);
        }
        if (!strcmp(s, "total"))
            return scalar(v, sum);
        if (!strcmp(s, "mean"))
            return scalar(v, mean);
        if (!strcmp(s, "count"))
            return scalar(v, a.len);
        if (!strcmp(s, "median"))
            return scalar(v, quantile(a, .5));
        if (!strcmp(s, "min"))
            return scalar(v, quantile(a, 0));
        if (!strcmp(s, "max"))
            return scalar(v, quantile(a, 1));
        if (!strcmp(s, "stdev") || !strcmp(s, "stdevp") || !strcmp(s, "var") ||
            !strcmp(s, "varp")) {
            bool pop = s[strlen(s) - 1] == 'p';
            if (a.len <= !pop)
                return error(e, "Need two observations");
            Real z = m2 / (a.len - !pop);
            return scalar(v, !strncmp(s, "stdev", 5) ? sqrtl(z) : z);
        }
        if (!strcmp(s, "sort") || !strcmp(s, "unique")) {
            *v = a;
            qsort(v->v, v->len, sizeof(Real), cmp);
            if (!strcmp(s, "unique")) {
                v->len = 0;
                for (unsigned i = 0; i < a.len; i++)
                    if (!i || v->v[i] != v->v[i - 1])
                        v->v[v->len++] = v->v[i];
            }
            return true;
        }
    }
    if (argc == 2) {
        if (!strcmp(s, "quantile") || !strcmp(s, "quartile")) {
            if (b.len != 1)
                return error(e, "Expected scalar quantile");
            Real z = b.v[0] / (!strcmp(s, "quartile") ? 4 : 1);
            if (z < 0 || z > 1)
                return error(e, "Quantile outside [0,1]");
            return scalar(v, quantile(a, z));
        }
        if (!strcmp(s, "join")) {
            if (a.len + b.len > DCE_LIST)
                return error(e, "List limit (32)");
            *v = a;
            memcpy(v->v + a.len, b.v, b.len * sizeof(Real));
            v->len += b.len;
            return true;
        }
        if (!strcmp(s, "distance") || !strcmp(s, "midpoint")) {
            if (a.len != 2 || b.len != 2)
                return error(e, "Expected two points");
            if (!strcmp(s, "distance"))
                return scalar(v, hypotl(a.v[0] - b.v[0], a.v[1] - b.v[1]));
            v->len = 2;
            v->point = 1;
            v->v[0] = (a.v[0] + b.v[0]) / 2;
            v->v[1] = (a.v[1] + b.v[1]) / 2;
            return true;
        }
        if (!strcmp(s, "corr") || !strcmp(s, "cov") || !strcmp(s, "covp")) {
            if (a.len != b.len || a.len < 2)
                return error(e, "Paired lists required");
            Real ax = 0, ay = 0, xx = 0, yy = 0, xy = 0;
            for (unsigned i = 0; i < a.len; i++) {
                ax += a.v[i] / a.len;
                ay += b.v[i] / a.len;
            }
            for (unsigned i = 0; i < a.len; i++) {
                Real dx = a.v[i] - ax, dy = b.v[i] - ay;
                xx += dx * dx;
                yy += dy * dy;
                xy += dx * dy;
            }
            if (!strcmp(s, "corr"))
                return scalar(v, xy / sqrtl(xx * yy));
            return scalar(v, xy / (a.len - (!strcmp(s, "cov") ? 1 : 0)));
        }
    }
    /* A defined f(x) takes one argument. Numeric a(x+1) is multiplication. */
    for (unsigned i = 0; i < e->model->s.rows; i++) {
        Program *p2 = &e->model->p[i];
        if (p2->root < 0)
            continue;
        Node *r = &p2->n[p2->root];
        if (r->kind != N_EQ || r->a < 0)
            continue;
        Node *lhs = &p2->n[r->a];
        if ((lhs->kind != N_CALL && lhs->kind != N_VAR) || strcmp(lhs->name, s) || argc != 1)
            continue;
        if (lhs->kind == N_CALL && dce_function_arg(p2, r->a) >= 0) {
            int arg = dce_function_arg(p2, r->a);
            if (p2->n[arg].kind != N_VAR)
                return error(e, "Invalid function parameter");
            if (e->active & (1u << i))
                return error(e, "Recursive/circular function");
            e->active |= 1u << i;
            v->len = a.len;
            v->point = 0;
            bool ok = true;
            for (unsigned j = 0; j < a.len; j++)
                if (!bound_eval(e, p2, r->b, p2->n[arg].name, a.v[j], &v->v[j])) {
                    ok = false;
                    break;
                }
            e->active &= ~(1u << i);
            return ok;
        }
        if (lhs->kind == N_VAR) {
            if (!variable(e, s, &b) || b.len != 1)
                return false;
            *v = a;
            for (unsigned j = 0; j < v->len; j++)
                v->v[j] *= b.v[0];
            return true;
        }
    }
    if (argc > 2)
        return error(e, "Wrong argument count");
    unsigned len = argc == 2 && b.len > a.len ? b.len : a.len;
    if (argc == 2 && a.len != b.len && a.len != 1 && b.len != 1)
        return error(e, "Mismatched list lengths");
    v->len = len;
    v->point = 0;
    for (unsigned i = 0; i < len; i++) {
        Real x = a.v[a.len == 1 ? 0 : i], y = argc == 2 ? b.v[b.len == 1 ? 0 : i] : 0, z = NAN;
        if (argc == 1) {
            if (!unary(e, s, x, &z))
                return error(e, "Unknown function or domain");
        } else if (!strcmp(s, "log"))
            z = logl(x) / logl(y);
        else if (!strcmp(s, "nthroot")) {
            if (y == 0)
                return error(e, "Root index cannot be zero");
            z = x < 0 && y == floorl(y) && fmodl(y, 2) != 0 ? -powl(-x, 1 / y) : powl(x, 1 / y);
        } else if (!strcmp(s, "round")) {
            if (fabsl(y) > 12)
                return error(e, "Rounding precision limit");
            Real f = powl(10, y);
            z = roundl(x * f) / f;
        } else if (!strcmp(s, "mod"))
            z = x - y * floorl(x / y);
        else if (!strcmp(s, "min"))
            z = fminl(x, y);
        else if (!strcmp(s, "max"))
            z = fmaxl(x, y);
        else if (!strcmp(s, "gcd") || !strcmp(s, "lcm")) {
            if (x != floorl(x) || y != floorl(y) || fabsl(x) > 1e12L || fabsl(y) > 1e12L)
                return error(e, "Expected bounded integers");
            Real aa = fabsl(x), bb = fabsl(y);
            while (bb) {
                Real t = fmodl(aa, bb);
                aa = bb;
                bb = t;
            }
            z = !strcmp(s, "gcd") ? aa : aa ? fabsl(x / aa * y) : 0;
        } else if (!strcmp(s, "nCr") || !strcmp(s, "nPr")) {
            if (x < 0 || y < 0 || x != floorl(x) || y != floorl(y) || y > x || x > 170)
                return error(e, "Combinatorics domain");
            z = 1;
            for (unsigned k = 0; k < (unsigned)y; k++)
                z *= !strcmp(s, "nCr") ? (x - k) / (k + 1) : (x - k);
        } else
            return error(e, "Unknown function");
        if (!isfinite(z))
            return error(e, "Outside real domain");
        v->v[i] = z;
    }
    return true;
}
bool dce_eval(Eval *e, const Program *p, int id, Value *v) {
    if (id < 0 || id >= p->used)
        return error(e, "Invalid expression");
    if (!e->budget--)
        return error(e, "Evaluation work limit");
    if (e->depth >= 12)
        return error(e, "Evaluation nesting limit");
    e->depth++;
    const Node *n = &p->n[id];
    Value a, b;
    bool ok = false;
    v->len = 0;
    v->point = 0;
    switch (n->kind) {
    case N_NUM:
        ok = scalar(v, n->number);
        break;
    case N_VAR:
        ok = variable(e, n->name, v);
        break;
    case N_CALL:
        ok = function(e, p, n, v);
        break;
    case N_LIST: {
        ok = true;
        for (int j = n->a; j >= 0; j = p->n[j].b) {
            if (v->len == DCE_LIST || !dce_eval(e, p, p->n[j].a, &a) || a.len != 1) {
                ok = error(e, "List limit or non-scalar entry");
                break;
            }
            v->v[v->len++] = a.v[0];
        }
        break;
    }
    case N_RANGE: {
        Value c;
        if (!dce_eval(e, p, n->a, &a) || !dce_eval(e, p, n->b, &b) || a.len != 1 || b.len != 1)
            break;
        Real start = a.v[0], end = b.v[0], step = end >= start ? 1 : -1;
        if (n->c >= 0) {
            if (!dce_eval(e, p, n->c, &c) || c.len != 1)
                break;
            step = c.v[0] - start;
        }
        if (step == 0 || (end - start) / step < 0) {
            error(e, "Invalid list range");
            break;
        }
        Real count = floorl((end - start) / step + 1e-10L) + 1;
        if (count > DCE_LIST) {
            error(e, "List limit (32)");
            break;
        }
        v->len = (uint8_t)count;
        for (unsigned i = 0; i < v->len; i++)
            v->v[i] = start + i * step;
        ok = true;
        break;
    }
    case N_IF:
        if (dce_eval(e, p, n->a, &a) && a.len == 1) {
            if (a.v[0])
                ok = dce_eval(e, p, n->b, v);
            else if (n->c >= 0)
                ok = dce_eval(e, p, n->c, v);
            else
                error(e, "Outside restriction");
        }
        break;
    case N_NEG:
    case N_FACT:
        if (dce_eval(e, p, n->a, v)) {
            ok = true;
            for (unsigned i = 0; i < v->len; i++) {
                Real z = v->v[i];
                if (n->kind == N_NEG)
                    v->v[i] = -z;
                else if (z < 0 || z > 170 || floorl(z) != z) {
                    ok = error(e, "Factorial domain");
                    break;
                } else {
                    Real f = 1;
                    for (unsigned j = 2; j <= (unsigned)z; j++)
                        f *= j;
                    v->v[i] = f;
                }
            }
        }
        break;
    default:
        if (!dce_eval(e, p, n->a, &a) || !dce_eval(e, p, n->b, &b))
            break;
        if (n->kind == N_POINT) {
            if (a.len == 1 && b.len == 1) {
                v->len = 2;
                v->point = 1;
                v->v[0] = a.v[0];
                v->v[1] = b.v[0];
                ok = true;
            } else
                error(e, "Point-list pairs not yet supported");
            break;
        }
        if (n->kind == N_INDEX) {
            if (b.len != 1 || b.v[0] != floorl(b.v[0]) || b.v[0] < 1 || b.v[0] > a.len) {
                error(e, "List index outside bounds");
                break;
            }
            ok = scalar(v, a.v[(unsigned)b.v[0] - 1]);
            break;
        }
        if (a.len != b.len && a.len != 1 && b.len != 1) {
            error(e, "Mismatched list lengths");
            break;
        }
        v->len = a.len > b.len ? a.len : b.len;
        ok = true;
        for (unsigned i = 0; i < v->len; i++) {
            Real x = a.v[a.len == 1 ? 0 : i], y = b.v[b.len == 1 ? 0 : i], z = NAN;
            switch (n->kind) {
            case N_ADD:
                z = x + y;
                break;
            case N_SUB:
            case N_REG:
                z = x - y;
                break;
            case N_MUL:
                z = x * y;
                break;
            case N_DIV:
                z = x / y;
                break;
            case N_POW:
                z = powl(x, y);
                break;
            case N_EQ:
                z = x == y;
                break;
            case N_LT:
                z = x < y;
                break;
            case N_LE:
                z = x <= y;
                break;
            case N_GT:
                z = x > y;
                break;
            case N_GE:
                z = x >= y;
                break;
            case N_AND:
                z = x != 0 && y != 0;
                break;
            case N_OR:
                z = x != 0 || y != 0;
                break;
            default:
                break;
            }
            if (!isfinite(z)) {
                ok = error(e, "Outside real domain");
                break;
            }
            v->v[i] = z;
        }
    }
    e->depth--;
    return ok;
}
