#include "dce.h"
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
/* Bounded recursive descent. Single-letter names concatenate as multiplication;
   documented function names and subscripted names are single tokens. */
enum { T_END = 256, T_NUM, T_ID, T_LE, T_GE, T_DOTS };
typedef struct {
    const char *s;
    int tok;
    Real num;
    char id[DCE_NAME];
    Program *p;
    unsigned depth;
} Parser;
static const char *const words[] = {
    "arcsin",   "arccos",  "arctan", "arccsc",   "arcsec",   "arccot",    "asinh",     "acosh",
    "atanh",    "asin",    "acos",   "atan",     "sinh",     "cosh",      "tanh",      "sqrt",
    "cbrt",     "nthroot", "sin",    "cos",      "tan",      "csc",       "sec",       "cot",
    "abs",      "exp",     "log",    "ln",       "floor",    "ceil",      "round",     "sign",
    "mod",      "gcd",     "lcm",    "nCr",      "nPr",      "mean",      "median",    "total",
    "count",    "min",     "max",    "stdevp",   "stdev",    "varp",      "var",       "quantile",
    "quartile", "sort",    "unique", "join",     "corr",     "covp",      "cov",       "sum",
    "prod",     "int",     "diff",   "distance", "midpoint", "normalpdf", "normalcdf", "pi",
    "theta",    "tau",     "ans",    "and",      "or",       NULL};
size_t dce_name_length(const char *s) {
    size_t n = 1;
    for (unsigned i = 0; words[i]; i++) {
        size_t k = strlen(words[i]);
        if (k > n && !strncmp(s, words[i], k))
            n = k;
    }
    if (s[n] == '_') {
        n++;
        while (isalnum((unsigned char)s[n]))
            n++;
    }
    return n;
}
static void fail(Parser *q, const char *s) {
    if (!q->p->error[0]) {
        strncpy(q->p->error, s, sizeof(q->p->error) - 1);
    }
}
static void next(Parser *q) {
    while (isspace((unsigned char)*q->s))
        q->s++;
    if (isdigit((unsigned char)*q->s) || (*q->s == '.' && isdigit((unsigned char)q->s[1]))) {
        char *end;
        q->num = (Real)strtold(q->s, &end);
        if (end > q->s && end[-1] == '.' && end[0] == '.' && end[1] == '.')
            end--;
        q->s = end;
        q->tok = T_NUM;
        return;
    }
    if (isalpha((unsigned char)*q->s)) {
        size_t n = dce_name_length(q->s);
        if (n >= DCE_NAME) {
            fail(q, "Name too long");
            n = DCE_NAME - 1;
        }
        memcpy(q->id, q->s, n);
        q->id[n] = 0;
        q->s += n;
        q->tok = T_ID;
        return;
    }
    if (!strncmp(q->s, "...", 3)) {
        q->s += 3;
        q->tok = T_DOTS;
        return;
    }
    if ((*q->s == '<' || *q->s == '>') && q->s[1] == '=') {
        q->tok = *q->s == '<' ? T_LE : T_GE;
        q->s += 2;
        return;
    }
    q->tok = *q->s ? (unsigned char)*q->s++ : T_END;
}
static int node(Parser *q, int kind, int a, int b, int c) {
    if (q->p->used >= DCE_NODES) {
        fail(q, "Expression too complex");
        return -1;
    }
    int i = q->p->used++;
    Node *n = &q->p->n[i];
    memset(n, 0, sizeof(*n));
    n->kind = (uint8_t)kind;
    n->a = a;
    n->b = b;
    n->c = c;
    return i;
}
static int expression(Parser *, int);
static bool take(Parser *q, int t) {
    if (q->tok != t)
        return false;
    next(q);
    return true;
}
static void expect(Parser *q, int t) {
    if (!take(q, t))
        fail(q, "Missing delimiter");
}
static int chain(Parser *q, int end) {
    int head = -1, tail = -1;
    while (q->tok != end && q->tok != T_END && !q->p->error[0]) {
        int v = expression(q, 0), n = node(q, N_ARG, v, -1, -1);
        if (n < 0)
            return -1;
        if (head < 0)
            head = n;
        else
            q->p->n[tail].b = n;
        tail = n;
        if (!take(q, ','))
            break;
    }
    return head;
}
static int atom(Parser *q) {
    if (++q->depth > 28) {
        fail(q, "Nesting limit (28)");
        q->depth--;
        return -1;
    }
    int r = -1;
    if (q->tok == T_NUM) {
        r = node(q, N_NUM, -1, -1, -1);
        if (r >= 0)
            q->p->n[r].number = q->num;
        next(q);
    } else if (q->tok == T_ID) {
        char name[DCE_NAME];
        strcpy(name, q->id);
        next(q);
        if (take(q, '(')) {
            int args = chain(q, ')');
            expect(q, ')');
            r = node(q, N_CALL, args, -1, -1);
        } else
            r = node(q, N_VAR, -1, -1, -1);
        if (r >= 0)
            strcpy(q->p->n[r].name, name);
    } else if (take(q, '(')) {
        r = expression(q, 0);
        if (take(q, ',')) {
            int b = expression(q, 0);
            r = node(q, N_POINT, r, b, -1);
        }
        expect(q, ')');
    } else if (take(q, '[')) {
        if (q->tok == ']') {
            fail(q, "Empty lists unsupported");
            next(q);
        } else {
            int a = expression(q, 0);
            if (take(q, T_DOTS)) {
                int b = expression(q, 0);
                r = node(q, N_RANGE, a, b, -1);
            } else {
                int head = node(q, N_ARG, a, -1, -1), tail = head;
                while (take(q, ',')) {
                    int b = expression(q, 0);
                    if (take(q, T_DOTS)) {
                        int end = expression(q, 0);
                        r = node(q, N_RANGE, a, end, b);
                        break;
                    }
                    int t = node(q, N_ARG, b, -1, -1);
                    if (t < 0 || tail < 0)
                        break;
                    q->p->n[tail].b = t;
                    tail = t;
                }
                if (r < 0)
                    r = node(q, N_LIST, head, -1, -1);
            }
            expect(q, ']');
        }
    } else if (take(q, '{')) {
        int c = expression(q, 0);
        expect(q, ':');
        int a = expression(q, 0), head = node(q, N_IF, c, a, -1), tail = head;
        while (take(q, ',')) {
            int cc = expression(q, 0);
            expect(q, ':');
            int aa = expression(q, 0), z = node(q, N_IF, cc, aa, -1);
            if (tail >= 0)
                q->p->n[tail].c = z;
            tail = z;
        }
        expect(q, '}');
        r = head;
    } else if (take(q, '-')) {
        int a = expression(q, 35);
        r = node(q, N_NEG, a, -1, -1);
    } else if (take(q, '+'))
        r = expression(q, 35);
    else {
        fail(q, "Expected a number or expression");
        if (q->tok != T_END)
            next(q);
    }
    q->depth--;
    return r;
}
static int prec(int t) {
    if (t == '=' || t == '~' || t == '<' || t == '>' || t == T_LE || t == T_GE)
        return 10;
    if (t == '+' || t == '-')
        return 20;
    if (t == '*' || t == '/')
        return 30;
    if (t == '^')
        return 40;
    return -1;
}
static int kind(int t) {
    switch (t) {
    case '+':
        return N_ADD;
    case '-':
        return N_SUB;
    case '*':
        return N_MUL;
    case '/':
        return N_DIV;
    case '^':
        return N_POW;
    case '=':
        return N_EQ;
    case '~':
        return N_REG;
    case '<':
        return N_LT;
    case '>':
        return N_GT;
    case T_LE:
        return N_LE;
    default:
        return N_GE;
    }
}
static int expression(Parser *q, int minimum) {
    if (q->depth > 28) {
        fail(q, "Nesting limit (28)");
        return -1;
    }
    q->depth++;
    int a = atom(q);
    while (a >= 0 && !q->p->error[0]) {
        if (q->tok == '!' && 50 >= minimum) {
            next(q);
            a = node(q, N_FACT, a, -1, -1);
            continue;
        }
        if (q->tok == '[' && 50 >= minimum) {
            next(q);
            int b = expression(q, 0);
            expect(q, ']');
            a = node(q, N_INDEX, a, b, -1);
            continue;
        }
        if (q->tok == '{' && 5 >= minimum) {
            next(q);
            int c = expression(q, 0);
            expect(q, '}');
            a = node(q, N_IF, c, a, -1);
            continue;
        }
        int t = q->tok, priority = prec(t);
        bool implicit = (t == T_NUM || t == T_ID || t == '(');
        if (t == T_ID && (!strcmp(q->id, "and") || !strcmp(q->id, "or"))) {
            priority = !strcmp(q->id, "and") ? 7 : 6;
            t = !strcmp(q->id, "and") ? N_AND : N_OR;
            implicit = false;
            if (priority < minimum)
                break;
            next(q);
            int b = expression(q, priority + 1);
            a = node(q, t, a, b, -1);
            continue;
        }
        if (implicit) {
            priority = 30;
            t = '*';
        }
        if (priority < minimum)
            break;
        if (!implicit)
            next(q);
        int b = expression(q, priority + (t == '^' ? 0 : 1));
        if (priority == 10 && a >= 0 && (q->p->n[a].kind >= N_LT && q->p->n[a].kind <= N_GE)) {
            int c = node(q, kind(t), q->p->n[a].b, b, -1);
            a = node(q, N_AND, a, c, -1);
        } else
            a = node(q, kind(t), a, b, -1);
    }
    q->depth--;
    return a;
}
bool dce_parse(Program *p, const char *text) {
    memset(p, 0, sizeof(*p));
    p->root = -1;
    if (strlen(text) >= DCE_TEXT) {
        strcpy(p->error, "Expression too long");
        return false;
    }
    Parser q = {text, 0, 0, {0}, p, 0};
    next(&q);
    if (q.tok == T_END)
        return true;
    p->root = expression(&q, 0);
    if (q.tok != T_END)
        fail(&q, "Unexpected input");
    if (p->error[0])
        p->root = -1;
    return !p->error[0];
}
