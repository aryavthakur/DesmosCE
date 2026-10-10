#include "dce.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
void dce_init(Model *m) {
    memset(m, 0, sizeof(*m));
    m->s.rows = 1;
    m->s.xmin = -10;
    m->s.xmax = 10;
    m->s.ymin = -10;
    m->s.ymax = 10;
    m->s.grid = 1;
    for (unsigned i = 0; i < DCE_ROWS; i++) {
        m->s.row[i].visible = 1;
        m->s.row[i].color = i % 6;
    }
    dce_compile(m);
}
void dce_compile(Model *m) {
    for (unsigned i = 0; i < DCE_ROWS; i++)
        dce_parse(&m->p[i], m->s.row[i].text);
    m->fits = 0;
}
Eval dce_context(Model *m, Real x, Real y) {
    Eval e;
    memset(&e, 0, sizeof(e));
    e.model = m;
    e.x = x;
    e.y = y;
    e.t = x;
    e.budget = 12000;
    return e;
}
int dce_function_arg(const Program *p, int lhs) {
    if (lhs < 0 || lhs >= p->used)
        return -1;
    const Node *n = &p->n[lhs];
    if (n->kind != N_CALL || (n->name[1] && n->name[1] != '_') || n->a < 0 || p->n[n->a].b >= 0)
        return -1;
    int arg = p->n[n->a].a;
    return arg >= 0 && p->n[arg].kind == N_VAR ? arg : -1;
}
bool dce_defined(const Model *m, const char *name) {
    if (!strcmp(name, "pi") || !strcmp(name, "e") || !strcmp(name, "tau") || !strcmp(name, "x_1") ||
        !strcmp(name, "y_1"))
        return true;
    for (unsigned j = 0; j < m->s.sliders; j++)
        if (!strcmp(name, m->s.slider[j].name))
            return true;
    for (unsigned i = 0; i < m->s.rows; i++) {
        const Program *p = &m->p[i];
        if (p->root < 0)
            continue;
        const Node *r = &p->n[p->root];
        if (r->kind == N_EQ && r->a >= 0) {
            const Node *a = &p->n[r->a];
            if ((a->kind == N_VAR || dce_function_arg(p, r->a) >= 0) && strcmp(a->name, "x") &&
                strcmp(a->name, "y") && !strcmp(name, a->name))
                return true;
        }
    }
    return false;
}
unsigned dce_unknowns(Model *m, bool reg, Parameter *out, unsigned cap) {
    unsigned count = 0;
    for (unsigned i = 0; i < m->s.rows; i++) {
        Program *p = &m->p[i];
        if (p->root < 0)
            continue;
        bool isreg = p->n[p->root].kind == N_REG;
        if (isreg != reg)
            continue;
        for (unsigned j = 0; j < p->used; j++) {
            Node *n = &p->n[j];
            if (n->kind != N_VAR || !strcmp(n->name, "x") || !strcmp(n->name, "y") ||
                !strcmp(n->name, "t") || !strcmp(n->name, "theta") || !strcmp(n->name, "ans") ||
                dce_defined(m, n->name))
                continue;
            bool found = false;
            for (unsigned k = 0; k < count && k < cap; k++)
                if (!strcmp(out[k].name, n->name))
                    found = true;
            if (!reg)
                for (unsigned k = 0; k < m->fits; k++)
                    if (!strcmp(m->fit[k].name, n->name))
                        found = true;
            if (!found) {
                if (count >= cap)
                    return cap + 1;
                memset(&out[count], 0, sizeof(*out));
                strcpy(out[count].name, n->name);
                out[count].value = 1;
                out[count].min = -10;
                out[count].max = 10;
                out[count].step = .1;
                count++;
            }
        }
    }
    return count;
}
void dce_add_sliders(Model *m) {
    Parameter u[DCE_PARAMS];
    unsigned n = dce_unknowns(m, false, u, DCE_PARAMS);
    if (n > DCE_PARAMS)
        n = DCE_PARAMS;
    for (unsigned i = 0; i < n && m->s.sliders < DCE_PARAMS; i++)
        m->s.slider[m->s.sliders++] = u[i];
}
bool dce_eval_text(Model *m, const char *s, Value *v) {
    Program p;
    if (!dce_parse(&p, s) || p.root < 0)
        return false;
    Eval e = dce_context(m, 0, 0);
    return dce_eval(&e, &p, p.root, v);
}
bool dce_scalar(Model *m, int row, Real x, Real y, Real *out) {
    if (row < 0 || row >= m->s.rows)
        return false;
    Program *p = &m->p[row];
    int base = p->root;
    while (base >= 0 && p->n[base].kind == N_IF && p->n[base].c < 0)
        base = p->n[base].b;
    if (base < 0)
        return false;
    Node *n = &p->n[base];
    int target = base;
    static Value v;
    static Eval e;
    e = dce_context(m, x, y);
    Binding binding = {NULL, x, NULL};
    if (n->kind == N_EQ) {
        int arg = dce_function_arg(p, n->a);
        if (arg >= 0) {
            binding.name = p->n[arg].name;
            e.bindings = &binding;
        } else if (p->n[n->a].kind != N_VAR || strcmp(p->n[n->a].name, "y"))
            return false;
        target = n->b;
    } else if (n->kind == N_REG || (n->kind >= N_LT && n->kind <= N_GE))
        return false;
    if (!dce_eval(&e, p, target, &v) || v.len != 1)
        return false;
    *out = v.v[0];
    e.y = *out;
    for (int q = p->root; q != base; q = p->n[q].b)
        if (!dce_eval(&e, p, p->n[q].a, &v) || v.len != 1 || !v.v[0])
            return false;
    return true;
}
/* Compact formatter avoids dependence on embedded printf long-double support. */
void dce_format(Real x, char *out, size_t n, unsigned digits) {
    if (!n)
        return;
    if (!isfinite(x)) {
        snprintf(out, n, "%s", isnan(x) ? "undefined" : x < 0 ? "-overflow" : "overflow");
        return;
    }
    if (x == 0) {
        snprintf(out, n, "0");
        return;
    }
    if (digits < 2)
        digits = 2;
    if (digits > 9)
        digits = 9;
    char buf[48];
    unsigned z = 0;
    bool neg = x < 0;
    if (neg)
        x = -x;
    int shift = x < 1e-300L ? -300 : 0;
    if (shift)
        x *= 1e300L;
    int ex = (int)floorl(log10l(x));
    Real f = x / powl(10, ex);
    ex += shift;
    if (f < 1) {
        f *= 10;
        ex--;
    } else if (f >= 10) {
        f /= 10;
        ex++;
    }
    uint32_t scale = 1;
    for (unsigned i = 1; i < digits; i++)
        scale *= 10;
    uint32_t rounded = (uint32_t)floorl(f * scale + .5);
    if (rounded >= 10UL * scale) {
        rounded /= 10;
        ex++;
    }
    char ds[12];
    snprintf(ds, sizeof(ds), "%lu", (unsigned long)rounded);
    if (neg)
        buf[z++] = '-';
    if (ex >= -4 && ex < (int)digits) {
        if (ex < 0) {
            buf[z++] = '0';
            buf[z++] = '.';
            for (int i = -1; i > ex; i--)
                buf[z++] = '0';
            for (unsigned i = 0; i < digits; i++)
                buf[z++] = ds[i];
        } else {
            for (int i = 0; i <= ex; i++)
                buf[z++] = ds[i];
            if ((unsigned)(ex + 1) < digits) {
                buf[z++] = '.';
                for (unsigned i = ex + 1; i < digits; i++)
                    buf[z++] = ds[i];
            }
        }
        if (memchr(buf, '.', z)) {
            while (z && buf[z - 1] == '0')
                z--;
            if (z && buf[z - 1] == '.')
                z--;
        }
    } else {
        buf[z++] = ds[0];
        buf[z++] = '.';
        for (unsigned i = 1; i < digits; i++)
            buf[z++] = ds[i];
        while (z && buf[z - 1] == '0')
            z--;
        if (buf[z - 1] == '.')
            z--;
        buf[z++] = 'e';
        snprintf(buf + z, sizeof(buf) - z, "%d", ex);
        z = strlen(buf);
    }
    buf[z] = 0;
    snprintf(out, n, "%s", buf);
}

bool dce_window_valid(const Session *s) {
    return isfinite(s->xmin) && isfinite(s->xmax) && isfinite(s->ymin) && isfinite(s->ymax) &&
           fabsl(s->xmin) <= 1e12L && fabsl(s->xmax) <= 1e12L && fabsl(s->ymin) <= 1e12L &&
           fabsl(s->ymax) <= 1e12L && s->xmax - s->xmin >= 1e-9L && s->ymax - s->ymin >= 1e-9L;
}
