#include "ui.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
static int alloc(Editor *e, int kind) {
    for (int i = 1; i < ED_CAP; i++)
        if (e->n[i].kind == E_FREE) {
            memset(&e->n[i], 0, sizeof(EdNode));
            e->n[i].kind = kind;
            return i;
        }
    e->overflow = true;
    return 0;
}
static int last(const Editor *e, int seq) {
    int n = e->n[seq].head;
    while (n && e->n[n].next)
        n = e->n[n].next;
    return n;
}
static void insert(Editor *e, int seq, int before, int n) {
    int prev = before ? e->n[before].prev : last(e, seq);
    e->n[n].parent = seq;
    e->n[n].prev = prev;
    e->n[n].next = before;
    if (prev)
        e->n[prev].next = n;
    else
        e->n[seq].head = n;
    if (before)
        e->n[before].prev = n;
}
static void unlink_node(Editor *e, int n) {
    EdNode *a = &e->n[n];
    if (a->prev)
        e->n[a->prev].next = a->next;
    else
        e->n[a->parent].head = a->next;
    if (a->next)
        e->n[a->next].prev = a->prev;
}
static void release(Editor *e, int n) {
    if (!n)
        return;
    if (e->n[n].kind == E_SEQ) {
        for (int c = e->n[n].head; c;) {
            int next = e->n[c].next;
            release(e, c);
            c = next;
        }
    } else {
        release(e, e->n[n].a);
        release(e, e->n[n].b);
    }
    memset(&e->n[n], 0, sizeof(EdNode));
}
bool ed_char(Editor *e, char ch) {
    if (e->read_only)
        return false;
    if (ch == '^')
        return ed_template(e, E_POWER);
    if (ch == '/')
        return ed_template(e, E_FRAC);
    if (ch == '_')
        return ed_template(e, E_SUB);
    int n = alloc(e, E_CHAR);
    if (!n)
        return false;
    e->n[n].ch = ch;
    insert(e, e->seq, e->before, n);
    return true;
}
static int atom_start(const Editor *e, int n) {
    if (!n)
        return 0;
    if (e->n[n].kind != E_CHAR)
        return n;
    char c = e->n[n].ch;
    if (c == ')' || c == ']') {
        char close = c, open = c == ')' ? '(' : '[';
        int depth = 0;
        for (int j = n; j; j = e->n[j].prev) {
            if (e->n[j].kind == E_CHAR) {
                if (e->n[j].ch == close)
                    depth++;
                if (e->n[j].ch == open && !--depth) {
                    int start = j;
                    for (int p = e->n[j].prev;
                         p && e->n[p].kind == E_CHAR && isalpha((unsigned char)e->n[p].ch);
                         p = e->n[p].prev)
                        start = p;
                    return start;
                }
            }
        }
        return n;
    }
    if (isdigit((unsigned char)c) || c == '.') {
        while (e->n[n].prev) {
            int p = e->n[n].prev;
            if (e->n[p].kind != E_CHAR ||
                (!isdigit((unsigned char)e->n[p].ch) && e->n[p].ch != '.'))
                break;
            n = p;
        }
    }
    return n;
}
bool ed_template(Editor *e, int kind) {
    if (e->read_only)
        return false;
    int block = alloc(e, kind);
    if (!block)
        return false;
    int a = alloc(e, E_SEQ), b = kind == E_ROOT ? 0 : alloc(e, E_SEQ);
    if (!a || (kind != E_ROOT && !b)) {
        release(e, a);
        release(e, b);
        release(e, block);
        return false;
    }
    e->n[block].a = a;
    e->n[block].b = b;
    e->n[a].parent = block;
    if (b)
        e->n[b].parent = block;
    int end = e->before ? e->n[e->before].prev : last(e, e->seq),
        start = kind == E_ROOT ? 0 : atom_start(e, end), seq = e->seq, before = e->before;
    if (start) {
        for (int n = start; n && n != before;) {
            int next = e->n[n].next;
            unlink_node(e, n);
            insert(e, a, 0, n);
            n = next;
        }
    }
    int first = e->n[a].head, tail = last(e, a);
    if (first && tail && first != tail && e->n[first].kind == E_CHAR && e->n[first].ch == '(' &&
        e->n[tail].kind == E_CHAR && e->n[tail].ch == ')') {
        unlink_node(e, first);
        release(e, first);
        unlink_node(e, tail);
        release(e, tail);
    }
    insert(e, seq, before, block);
    e->seq = kind == E_ROOT ? a : b;
    e->before = 0;
    return true;
}
void ed_right(Editor *e) {
    if (e->before) {
        int n = e->before;
        if (e->n[n].kind != E_CHAR) {
            e->seq = e->n[n].a;
            e->before = e->n[e->seq].head;
        } else
            e->before = e->n[n].next;
        return;
    }
    int parent = e->n[e->seq].parent;
    if (!parent)
        return;
    if (e->seq == e->n[parent].a && e->n[parent].b) {
        e->seq = e->n[parent].b;
        e->before = e->n[e->seq].head;
    } else {
        e->seq = e->n[parent].parent;
        e->before = e->n[parent].next;
    }
}
void ed_left(Editor *e) {
    int n = e->before ? e->n[e->before].prev : last(e, e->seq);
    if (n) {
        if (e->n[n].kind != E_CHAR) {
            e->seq = e->n[n].b ? e->n[n].b : e->n[n].a;
            e->before = 0;
        } else
            e->before = n;
        return;
    }
    int parent = e->n[e->seq].parent;
    if (!parent)
        return;
    if (e->seq == e->n[parent].b) {
        e->seq = e->n[parent].a;
        e->before = 0;
    } else {
        e->seq = e->n[parent].parent;
        e->before = parent;
    }
}
void ed_vertical(Editor *e, int dir) {
    int seq = e->seq;
    while (e->n[seq].parent) {
        int p = e->n[seq].parent;
        bool upper = e->n[p].kind == E_FRAC ? seq == e->n[p].a : seq == e->n[p].b;
        if (e->n[p].b && ((dir > 0 && upper) || (dir < 0 && !upper))) {
            e->seq = seq == e->n[p].a ? e->n[p].b : e->n[p].a;
            e->before = 0;
            return;
        }
        seq = e->n[p].parent;
    }
}
void ed_delete(Editor *e) {
    int n = e->before ? e->n[e->before].prev : last(e, e->seq);
    if (!n) {
        ed_left(e);
        return;
    }
    unlink_node(e, n);
    release(e, n);
}
static bool emit(const Editor *e, int id, char *out, size_t cap, size_t *pos) {
    const EdNode *n = &e->n[id];
#define PUT(c)                                                                                     \
    do {                                                                                           \
        if (*pos + 1 >= cap)                                                                       \
            return false;                                                                          \
        out[(*pos)++] = (c);                                                                       \
    } while (0)
    if (n->kind == E_SEQ) {
        int previous = 0;
        for (int j = n->head; j; j = e->n[j].next) {
            const EdNode *cur = &e->n[j], *prev = &e->n[previous];
            bool left = previous && (prev->kind != E_CHAR || isalnum((unsigned char)prev->ch) ||
                                     prev->ch == ')' || prev->ch == ']');
            bool right = cur->kind != E_CHAR || isalnum((unsigned char)cur->ch) || cur->ch == '(';
            if (left && right && (prev->kind != E_CHAR || cur->kind != E_CHAR)) {
                PUT('*');
            }
            if (!emit(e, j, out, cap, pos))
                return false;
            previous = j;
        }
    } else if (n->kind == E_CHAR) {
        PUT(n->ch);
    } else if (n->kind == E_SUB) {
        if (!emit(e, n->a, out, cap, pos))
            return false;
        PUT('_');
        if (!emit(e, n->b, out, cap, pos))
            return false;
    } else {
        if (n->kind == E_ROOT) {
            PUT('s');
            PUT('q');
            PUT('r');
            PUT('t');
        }
        PUT('(');
        if (!emit(e, n->a, out, cap, pos))
            return false;
        PUT(')');
        if (n->kind != E_ROOT) {
            PUT(n->kind == E_FRAC ? '/' : '^');
            PUT('(');
            if (!emit(e, n->b, out, cap, pos))
                return false;
            PUT(')');
        }
    }
#undef PUT
    return true;
}
bool ed_write(const Editor *e, char *out, size_t cap) {
    size_t p = 0;
    if (!cap)
        return false;
    if (e->read_only) {
        size_t n = strlen(e->preserved);
        if (n >= cap)
            return false;
        memcpy(out, e->preserved, n + 1);
        return true;
    }
    bool ok = emit(e, 0, out, cap, &p);
    out[p] = 0;
    return ok;
}
static void load_seq(Editor *, const char **, bool, unsigned);
static void load_atom(Editor *e, const char **s, unsigned depth) {
    if (depth > 24) {
        e->overflow = true;
        return;
    }
    if (**s == '(') {
        (*s)++;
        load_seq(e, s, true, depth + 1);
    } else {
        if (**s == '-' || **s == '+')
            ed_char(e, *(*s)++);
        if (!strncmp(*s, "sqrt(", 5)) {
            *s += 5;
            int parent = e->seq;
            ed_template(e, E_ROOT);
            load_seq(e, s, true, depth + 1);
            e->seq = parent;
            e->before = 0;
        } else if (isdigit((unsigned char)**s) || **s == '.') {
            char *end;
            strtold(*s, &end);
            while (*s < end)
                ed_char(e, *(*s)++);
        } else if (isalpha((unsigned char)**s)) {
            size_t n = dce_name_length(*s);
            int parent = e->seq;
            for (size_t i = 0; i < n; i++)
                ed_char(e, *(*s)++);
            e->seq = parent;
            e->before = 0;
            if (**s == '(') {
                (*s)++;
                ed_char(e, '(');
                load_seq(e, s, true, depth + 1);
                ed_char(e, ')');
            }
        }
    }
    if (**s == '^') {
        (*s)++;
        int parent = e->seq;
        if (ed_template(e, E_POWER))
            load_atom(e, s, depth + 1);
        e->seq = parent;
        e->before = 0;
    }
}
static void load_seq(Editor *e, const char **s, bool stop, unsigned depth) {
    if (depth > 24) {
        e->overflow = true;
        return;
    }
    int parens = 0;
    while (**s) {
        char c = **s;
        if (c == ')' && stop && parens == 0) {
            (*s)++;
            break;
        }
        if (!strncmp(*s, "sqrt(", 5)) {
            *s += 5;
            int parent = e->seq;
            if (ed_template(e, E_ROOT))
                load_seq(e, s, true, depth + 1);
            e->seq = parent;
            e->before = 0;
            continue;
        }
        (*s)++;
        if (c == '^' || c == '/' || c == '_') {
            int parent = e->seq;
            if (ed_template(e, c == '^' ? E_POWER : c == '/' ? E_FRAC : E_SUB)) {
                if (c == '_') {
                    if (isalpha((unsigned char)**s))
                        ed_char(e, *(*s)++);
                    else
                        while (isdigit((unsigned char)**s))
                            ed_char(e, *(*s)++);
                } else
                    load_atom(e, s, depth + 1);
            }
            e->seq = parent;
            e->before = 0;
        } else {
            if (c == '(')
                parens++;
            if (c == ')')
                parens--;
            ed_char(e, c);
        }
    }
}
void ed_load(Editor *e, const char *s) {
    char original[DCE_TEXT];
    strncpy(original, s, sizeof(original) - 1);
    original[sizeof(original) - 1] = 0;
    memset(e->n, 0, sizeof(e->n));
    e->n[0].kind = E_SEQ;
    e->seq = 0;
    e->before = 0;
    e->overflow = false;
    e->read_only = false;
    load_seq(e, &s, false, 0);
    e->seq = 0;
    e->before = 0;
    if (e->overflow || *s) {
        e->read_only = true;
        strcpy(e->preserved, original);
    }
}
void ed_init(Editor *e, const char *s) {
    memset(e, 0, sizeof(*e));
    ed_load(e, s);
    ed_record(e);
}
void ed_record(Editor *e) {
    char s[DCE_TEXT];
    if (!ed_write(e, s, sizeof(s)))
        return;
    if (e->hcount && !strcmp(e->history[e->hpos], s))
        return;
    if (e->hcount && e->hpos + 1 < e->hcount)
        e->hcount = e->hpos + 1;
    if (e->hcount == ED_HISTORY) {
        memmove(e->history, e->history + 1, (ED_HISTORY - 1) * DCE_TEXT);
        e->hcount--;
    }
    strcpy(e->history[e->hcount], s);
    e->hpos = e->hcount++;
}
void ed_undo(Editor *e) {
    if (e->hpos)
        ed_load(e, e->history[--e->hpos]);
}
void ed_redo(Editor *e) {
    if (e->hpos + 1 < e->hcount)
        ed_load(e, e->history[++e->hpos]);
}
Box ed_measure(const Editor *e, int id) {
    if (e->read_only && id == 0) {
        Box b = {(int)strlen(e->preserved) * 6, 7, 2};
        return b;
    }
    const EdNode *n = &e->n[id];
    Box z = {0, 7, 2};
    if (n->kind == E_CHAR) {
        z.w = 6;
        return z;
    }
    if (n->kind == E_SEQ) {
        for (int j = n->head; j; j = e->n[j].next) {
            Box b = ed_measure(e, j);
            z.w += b.w;
            if (b.a > z.a)
                z.a = b.a;
            if (b.d > z.d)
                z.d = b.d;
        }
        if (!z.w)
            z.w = 6;
        return z;
    }
    Box a = ed_measure(e, n->a), b = {0, 0, 0};
    if (n->b)
        b = ed_measure(e, n->b);
    if (n->kind == E_FRAC) {
        z.w = (a.w > b.w ? a.w : b.w) + 6;
        z.a = a.a + a.d + 3;
        z.d = b.a + b.d + 3;
    } else if (n->kind == E_ROOT) {
        z.w = a.w + 9;
        z.a = a.a + 3;
        z.d = a.d;
    } else {
        z.w = a.w + b.w + 1;
        if (n->kind == E_POWER) {
            z.a = a.a + b.a + b.d - 1;
            z.d = a.d;
        } else {
            z.a = a.a;
            z.d = a.d + b.a + b.d - 1;
        }
    }
    return z;
}
static void draw_node(Editor *e, Draw *d, int id, int x, int y, unsigned color, bool cursor) {
    EdNode *n = &e->n[id];
    if (n->kind == E_CHAR) {
        char s[2] = {n->ch, 0};
        d->text(d->user, x, y - 7, s, color);
        return;
    }
    if (n->kind == E_SEQ) {
        int xx = x;
        for (int j = n->head; j; j = e->n[j].next) {
            if (e->seq == id && e->before == j) {
                e->cx = xx;
                e->cy = y;
                if (cursor)
                    d->rect(d->user, xx, y - 8, 1, 11, C_BLUE);
            }
            draw_node(e, d, j, xx, y, color, cursor);
            xx += ed_measure(e, j).w;
        }
        if (e->seq == id && !e->before) {
            e->cx = xx;
            e->cy = y;
            if (cursor)
                d->rect(d->user, xx, y - 8, 1, 11, C_BLUE);
        }
        if (!n->head)
            d->rect(d->user, x, y - 6, 5, 7, C_GRID);
        return;
    }
    Box a = ed_measure(e, n->a), b = {0, 0, 0}, z = ed_measure(e, id);
    if (n->b)
        b = ed_measure(e, n->b);
    if (n->kind == E_FRAC) {
        draw_node(e, d, n->a, x + (z.w - a.w) / 2, y - 3 - a.d, color, cursor);
        draw_node(e, d, n->b, x + (z.w - b.w) / 2, y + 3 + b.a, color, cursor);
        d->line(d->user, x + 1, y, x + z.w - 2, y, color);
    } else if (n->kind == E_ROOT) {
        d->line(d->user, x, y - 1, x + 3, y + 2, color);
        d->line(d->user, x + 3, y + 2, x + 6, y - a.a - 2, color);
        d->line(d->user, x + 6, y - a.a - 2, x + z.w, y - a.a - 2, color);
        draw_node(e, d, n->a, x + 8, y, color, cursor);
    } else {
        draw_node(e, d, n->a, x, y, color, cursor);
        draw_node(e, d, n->b, x + a.w + 1,
                  n->kind == E_POWER ? y - a.a - b.d + 1 : y + a.d + b.a - 1, color, cursor);
    }
}
void ed_draw(Editor *e, Draw *d, int x, int y, unsigned color, bool cursor) {
    if (e->read_only) {
        d->text(d->user, x, y - 7, e->preserved, color);
        return;
    }
    draw_node(e, d, 0, x, y, color, cursor);
}
