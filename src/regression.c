#include "dce.h"
#include <math.h>
#include <string.h>
/* Householder QR with column pivoting: never form J^T J. a is row-major. */
bool dce_qr(Real *a, Real *b, unsigned m, unsigned n, Real *x, unsigned *rank) {
    static unsigned perm[DCE_PARAMS];
    static Real sol[DCE_PARAMS];
    Real scale = 0;
    *rank = 0;
    if (!n || n > DCE_PARAMS || m < n)
        return false;
    for (unsigned j = 0; j < n; j++) {
        perm[j] = j;
        Real norm = 0;
        for (unsigned i = 0; i < m; i++)
            norm = hypotl(norm, a[i * n + j]);
        if (norm > scale)
            scale = norm;
    }
    for (unsigned k = 0; k < n; k++) {
        unsigned best = k;
        Real norm = 0;
        for (unsigned j = k; j < n; j++) {
            Real z = 0;
            for (unsigned i = k; i < m; i++)
                z = hypotl(z, a[i * n + j]);
            if (z > norm) {
                norm = z;
                best = j;
            }
        }
        if (norm <= 1e-9L * fmaxl(scale, 1e-12L))
            break;
        if (best != k) {
            unsigned t = perm[k];
            perm[k] = perm[best];
            perm[best] = t;
            for (unsigned i = 0; i < m; i++) {
                Real z = a[i * n + k];
                a[i * n + k] = a[i * n + best];
                a[i * n + best] = z;
            }
        }
        Real alpha = -copysignl(norm, a[k * n + k]), v0 = a[k * n + k] - alpha;
        Real den = v0 * v0;
        for (unsigned i = k + 1; i < m; i++)
            den += a[i * n + k] * a[i * n + k];
        if (!den)
            break;
        for (unsigned j = k + 1; j < n; j++) {
            Real dot = v0 * a[k * n + j];
            for (unsigned i = k + 1; i < m; i++)
                dot += a[i * n + k] * a[i * n + j];
            dot *= 2 / den;
            a[k * n + j] -= dot * v0;
            for (unsigned i = k + 1; i < m; i++)
                a[i * n + j] -= dot * a[i * n + k];
        }
        Real dot = v0 * b[k];
        for (unsigned i = k + 1; i < m; i++)
            dot += a[i * n + k] * b[i];
        dot *= 2 / den;
        b[k] -= dot * v0;
        for (unsigned i = k + 1; i < m; i++)
            b[i] -= dot * a[i * n + k];
        a[k * n + k] = alpha;
        for (unsigned i = k + 1; i < m; i++)
            a[i * n + k] = 0;
        (*rank)++;
    }
    if (*rank < n)
        return false;
    for (int i = (int)n - 1; i >= 0; i--) {
        Real z = b[i];
        for (unsigned j = i + 1; j < n; j++)
            z -= a[i * n + j] * sol[j];
        sol[i] = z / a[i * n + i];
    }
    for (unsigned j = 0; j < n; j++)
        x[perm[j]] = sol[j];
    return true;
}
static bool residual(Model *m, Parameter *params, unsigned count, Real *r, unsigned *len) {
    *len = 0;
    for (unsigned i = 0; i < m->s.rows; i++) {
        Program *p = &m->p[i];
        if (p->root < 0 || p->n[p->root].kind != N_REG)
            continue;
        Eval e = dce_context(m, 0, 0);
        e.params = params;
        e.count = count;
        Value v;
        if (!dce_eval(&e, p, p->root, &v) || *len + v.len > DCE_RESID)
            return false;
        memcpy(r + *len, v.v, v.len * sizeof(Real));
        *len += v.len;
    }
    return *len > 0;
}
static Real norm2(const Real *r, unsigned n) {
    Real s = 0;
    for (unsigned i = 0; i < n; i++)
        s += r[i] * r[i];
    return s;
}
static bool jacobian(Model *m, Parameter *p, unsigned n, const Real *base, unsigned rows, Real *j) {
    static Real plus[DCE_RESID], minus[DCE_RESID];
    for (unsigned col = 0; col < n; col++) {
        Real x = p[col].value, h = 6e-6L * (1 + fabsl(x));
        unsigned np = 0, nm = 0;
        p[col].value = x + h;
        bool a = residual(m, p, n, plus, &np);
        p[col].value = x - h;
        bool b = residual(m, p, n, minus, &nm);
        p[col].value = x;
        if (!a && !b)
            return false;
        for (unsigned row = 0; row < rows; row++) {
            if (a && np != rows)
                return false;
            if (b && nm != rows)
                return false;
            j[row * n + col] = a && b ? (plus[row] - minus[row]) / (2 * h)
                               : a    ? (plus[row] - base[row]) / h
                                      : (base[row] - minus[row]) / h;
        }
    }
    return true;
}
/* Conservative symbolic degree in the inferred parameters. Nonlinear or
 * unsupported constructs go through LM; no finite-probe linearity guesses. */
static int degree(Model *m, const Program *p, int id, Parameter *parameters, unsigned count,
                  unsigned depth) {
    if (id < 0)
        return 0;
    if (depth > 16)
        return -1;
    const Node *n = &p->n[id];
    if (n->kind == N_NUM)
        return 0;
    if (n->kind == N_VAR) {
        for (unsigned i = 0; i < count; i++)
            if (!strcmp(n->name, parameters[i].name))
                return 1;
        for (unsigned i = 0; i < m->s.rows; i++) {
            Program *q = &m->p[i];
            if (q->root < 0)
                continue;
            Node *r = &q->n[q->root];
            if (r->kind == N_EQ && q->n[r->a].kind == N_VAR && !strcmp(n->name, q->n[r->a].name))
                return degree(m, q, r->b, parameters, count, depth + 1);
        }
        return 0;
    }
    int a = degree(m, p, n->a, parameters, count, depth + 1),
        b = degree(m, p, n->b, parameters, count, depth + 1);
    if (a < 0 || b < 0)
        return -1;
    switch (n->kind) {
    case N_NEG:
        return a;
    case N_ADD:
    case N_SUB:
    case N_REG:
    case N_ARG:
    case N_LIST:
        return a > b ? a : b;
    case N_MUL:
        return a + b <= 1 ? a + b : -1;
    case N_DIV:
        return b ? -1 : a;
    case N_POW:
        if (!a && !b)
            return 0;
        if (p->n[n->b].kind == N_NUM && p->n[n->b].number == 1)
            return a;
        return -1;
    case N_CALL:
        return strlen(n->name) > 1 && a == 0 ? 0 : -1;
    default:
        return !a && !b && degree(m, p, n->c, parameters, count, depth + 1) == 0 ? 0 : -1;
    }
}
void dce_fit(Model *m, FitResult *out, CancelFn cancel, void *user) {
    /* Static scratch keeps large Jacobians off the CE stack. Not reentrant. */
    static Real j[DCE_RESID * DCE_PARAMS], a[(DCE_RESID + DCE_PARAMS) * DCE_PARAMS],
        rhs[DCE_RESID + DCE_PARAMS];
    static Real r[DCE_RESID], trial[DCE_RESID], delta[DCE_PARAMS], scales[DCE_PARAMS];
    static Parameter p[DCE_PARAMS], best[DCE_PARAMS];
    memset(out, 0, sizeof(*out));
    out->status = FIT_NO_DATA;
    m->fits = 0;
    unsigned n = dce_unknowns(m, true, p, DCE_PARAMS);
    if (n > DCE_PARAMS) {
        out->status = FIT_UNDER;
        return;
    }
    out->count = n;
    if (!n) {
        unsigned nr;
        if (residual(m, p, 0, r, &nr)) {
            out->observations = nr;
            out->sse = norm2(r, nr);
            out->rmse = sqrtl(out->sse / nr);
            out->status = out->sse < 1e-12 ? FIT_OK : FIT_NO_DATA;
        }
        return;
    }
    Real bests = INFINITY;
    bool converged = false;
    unsigned bestrows = 0;
    bool affine = true;
    for (unsigned i = 0; i < m->s.rows; i++) {
        Program *pr = &m->p[i];
        if (pr->root >= 0 && pr->n[pr->root].kind == N_REG && degree(m, pr, pr->root, p, n, 0) < 0)
            affine = false;
    }
    if (affine) {
        for (unsigned col = 0; col < n; col++)
            p[col].value = 0;
        unsigned rows;
        if (!residual(m, p, n, r, &rows)) {
            out->status = FIT_DOMAIN;
            return;
        }
        if (rows < n) {
            out->status = FIT_UNDER;
            return;
        }
        for (unsigned col = 0; col < n; col++) {
            if (cancel && cancel(user)) {
                out->status = FIT_CANCEL;
                return;
            }
            p[col].value = 1;
            unsigned nr;
            bool ok = residual(m, p, n, trial, &nr);
            p[col].value = 0;
            if (!ok || nr != rows) {
                out->status = FIT_DOMAIN;
                return;
            }
            Real z = 0;
            for (unsigned row = 0; row < rows; row++) {
                j[row * n + col] = trial[row] - r[row];
                z = hypotl(z, j[row * n + col]);
            }
            scales[col] = fmaxl(z, 1e-12L);
        }
        for (unsigned row = 0; row < rows; row++) {
            rhs[row] = -r[row];
            for (unsigned col = 0; col < n; col++)
                a[row * n + col] = j[row * n + col] / scales[col];
        }
        unsigned rank;
        if (!dce_qr(a, rhs, rows, n, delta, &rank)) {
            out->status = FIT_RANK;
            return;
        }
        for (unsigned col = 0; col < n; col++)
            p[col].value = delta[col] / scales[col];
        if (!residual(m, p, n, r, &rows)) {
            out->status = FIT_DOMAIN;
            return;
        }
        bests = norm2(r, rows);
        bestrows = rows;
        memcpy(best, p, n * sizeof(*p));
        converged = true;
        out->iterations = 1;
        goto finalize;
    }
    for (unsigned start = 0; start < 4; start++) {
        for (unsigned k = 0; k < n; k++)
            p[k].value = start == 0 ? 1 : start == 1 ? .1L : start == 2 ? 2 : -1;
        unsigned rows = 0;
        if (!residual(m, p, n, r, &rows)) {
            out->status = FIT_DOMAIN;
            continue;
        }
        if (rows < n) {
            out->status = FIT_UNDER;
            return;
        }
        Real s = norm2(r, rows), lambda = 1e-3L;
        bool done = false;
        for (unsigned iter = 0; iter < 64; iter++) {
            out->iterations++;
            if (cancel && cancel(user)) {
                out->status = FIT_CANCEL;
                return;
            }
            if (!jacobian(m, p, n, r, rows, j)) {
                out->status = FIT_DOMAIN;
                break;
            }
            for (unsigned col = 0; col < n; col++) {
                Real z = 0;
                for (unsigned row = 0; row < rows; row++)
                    z = hypotl(z, j[row * n + col]);
                scales[col] = fmaxl(z, 1e-12L);
            }
            memset(a, 0, (rows + n) * n * sizeof(Real));
            for (unsigned row = 0; row < rows; row++) {
                rhs[row] = -r[row];
                for (unsigned col = 0; col < n; col++)
                    a[row * n + col] = j[row * n + col] / scales[col];
            }
            for (unsigned col = 0; col < n; col++) {
                a[(rows + col) * n + col] = sqrtl(lambda);
                rhs[rows + col] = 0;
            }
            unsigned rank;
            if (!dce_qr(a, rhs, rows + n, n, delta, &rank))
                break;
            Real step = 0;
            for (unsigned col = 0; col < n; col++) {
                delta[col] /= scales[col];
                p[col].value += delta[col];
                step = fmaxl(step, fabsl(delta[col]) / (1 + fabsl(p[col].value)));
            }
            unsigned nr = 0;
            bool valid = residual(m, p, n, trial, &nr) && nr == rows;
            Real ss = valid ? norm2(trial, rows) : INFINITY;
            if (valid && ss <= s) {
                Real change = s - ss;
                s = ss;
                memcpy(r, trial, rows * sizeof(Real));
                lambda = fmaxl(lambda / 3, 1e-12L);
                if (s < 1e-18L || step < 1e-8L || change < 1e-12L * (1 + s)) {
                    done = true;
                    break;
                }
            } else {
                for (unsigned col = 0; col < n; col++)
                    p[col].value -= delta[col];
                lambda *= 10;
                if (lambda > 1e14L)
                    break;
            }
        }
        if (s < bests) {
            bests = s;
            memcpy(best, p, n * sizeof(*p));
            bestrows = rows;
            converged = done;
        }
        if (done && s < 1e-18L)
            break;
    }
finalize:
    if (!isfinite(bests)) {
        out->status = FIT_DOMAIN;
        return;
    }
    memcpy(p, best, n * sizeof(*p));
    unsigned rows;
    if (!residual(m, p, n, r, &rows) || rows != bestrows || !jacobian(m, p, n, r, rows, j)) {
        out->status = FIT_DOMAIN;
        return;
    }
    for (unsigned col = 0; col < n; col++) {
        Real z = 0;
        for (unsigned row = 0; row < rows; row++)
            z = hypotl(z, j[row * n + col]);
        scales[col] = fmaxl(z, 1e-12L);
    }
    for (unsigned row = 0; row < rows; row++) {
        rhs[row] = 0;
        for (unsigned col = 0; col < n; col++)
            a[row * n + col] = j[row * n + col] / scales[col];
    }
    unsigned rank;
    dce_qr(a, rhs, rows, n, delta, &rank);
    out->status = rank < n ? FIT_RANK : converged ? FIT_OK : FIT_LIMIT;
    out->observations = rows;
    out->sse = bests;
    out->rmse = sqrtl(bests / rows);
    memcpy(out->p, p, n * sizeof(*p));
    memcpy(out->residual, r, rows * sizeof(Real));
    /* R^2 only for one regression whose left side is exactly y_1. */
    unsigned regs = 0;
    bool eligible = false;
    for (unsigned i = 0; i < m->s.rows; i++) {
        Program *pr = &m->p[i];
        if (pr->root >= 0 && pr->n[pr->root].kind == N_REG) {
            regs++;
            Node *lhs = &pr->n[pr->n[pr->root].a];
            eligible = lhs->kind == N_VAR && !strcmp(lhs->name, "y_1");
        }
    }
    if (regs == 1 && eligible && m->s.points > 1) {
        Real avg = 0, total = 0;
        for (unsigned i = 0; i < m->s.points; i++)
            avg += m->s.y[i] / m->s.points;
        for (unsigned i = 0; i < m->s.points; i++) {
            Real d = m->s.y[i] - avg;
            total += d * d;
        }
        if (total > 0) {
            out->has_r2 = true;
            out->r2 = 1 - bests / total;
        }
    }
    if (out->status == FIT_OK) {
        m->fits = n;
        memcpy(m->fit, p, n * sizeof(*p));
    }
}
const char *dce_fit_status(FitStatus s) {
    static const char *const t[] = {"Converged (local numeric fit)",
                                    "No unknowns/data to fit",
                                    "Underdetermined: need constraints",
                                    "Rank deficient / not identifiable",
                                    "Invalid domain or too much data",
                                    "Not converged; estimates only",
                                    "Cancelled"};
    return t[s];
}
