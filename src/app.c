#include "ui.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *const palette[] = {"~",
                                      "=",
                                      "fraction",
                                      "sqrt",
                                      "power",
                                      "subscript",
                                      "x_1",
                                      "y_1",
                                      "[",
                                      "]",
                                      "{",
                                      "}",
                                      ":",
                                      "<",
                                      ">",
                                      "<=",
                                      ">=",
                                      "pi",
                                      "e",
                                      "L",
                                      "f(",
                                      "g(",
                                      "mean(",
                                      "median(",
                                      "stdev(",
                                      "stdevp(",
                                      "total(",
                                      "count(",
                                      "min(",
                                      "max(",
                                      "abs(",
                                      "exp(",
                                      "log(",
                                      "gcd(",
                                      "lcm(",
                                      "nCr(",
                                      "nPr(",
                                      "sort(",
                                      "unique(",
                                      "join(",
                                      "quantile(",
                                      "quartile(",
                                      "corr(",
                                      "mod(",
                                      "floor(",
                                      "ceil(",
                                      "round(",
                                      "sum(",
                                      "prod(",
                                      "int(",
                                      "diff(",
                                      "distance(",
                                      "midpoint(",
                                      "Linear fit",
                                      "Quadratic fit",
                                      "Exponential fit"};
#define PALETTE_COUNT (sizeof(palette) / sizeof(*palette))
static void notice(App *a, const char *s) {
    snprintf(a->notice, sizeof(a->notice), "%s", s);
}
void app_rebuild(App *a) {
    dce_compile(&a->m);
    dce_fit(&a->m, &a->fit, a->cancel, a->cancel_user);
    dce_add_sliders(&a->m);
    a->dirty = true;
    a->graph_dirty = true;
    a->trace = false;
}
void app_init(App *a) {
    memset(a, 0, sizeof(*a));
    dce_init(&a->m);
    ed_init(&a->editor, "");
    a->dirty = true;
    a->graph_dirty = true;
    notice(a, "Enter applies | 2nd MODE saves + exits");
}
static bool commit(App *a) {
    if (a->mode > 1)
        return true;
    char s[DCE_TEXT];
    if (!ed_write(&a->editor, s, sizeof(s))) {
        notice(a, "Expression exceeds 191 characters");
        return false;
    }
    strcpy(a->m.s.row[a->selected].text, s);
    app_rebuild(a);
    if (a->editor.read_only)
        notice(a, "Too complex to format; source preserved");
    else if (a->m.p[a->selected].error[0])
        notice(a, a->m.p[a->selected].error);
    else if (a->m.p[a->selected].root >= 0 &&
             a->m.p[a->selected].n[a->m.p[a->selected].root].kind == N_REG)
        notice(a, dce_fit_status(a->fit.status));
    else
        notice(a, "Applied | 2nd UP/DOWN selects a row");
    return true;
}
static void choose(App *a, unsigned row) {
    if (row >= a->m.s.rows)
        return;
    a->selected = row;
    ed_init(&a->editor, a->m.s.row[row].text);
    a->scroll = 0;
}
static void text(Editor *e, const char *s) {
    while (*s)
        ed_char(e, *s++);
}
static int alpha_char(int k) {
    switch (k) {
    case K_MATH:
        return 'a';
    case K_RECIP:
        return 'd';
    case K_SIN:
        return 'e';
    case K_COS:
        return 'f';
    case K_TAN:
        return 'g';
    case '^':
        return 'h';
    case K_SQUARE:
        return 'i';
    case ',':
        return 'j';
    case '(':
        return 'k';
    case ')':
        return 'l';
    case '/':
        return 'm';
    case K_LOG:
        return 'n';
    case '7':
        return 'o';
    case '8':
        return 'p';
    case '9':
        return 'q';
    case '*':
        return 'r';
    case K_LN:
        return 's';
    case '4':
        return 't';
    case '5':
        return 'u';
    case '6':
        return 'v';
    case '-':
        return 'w';
    case K_STORE:
        return 'x';
    case '1':
        return 'y';
    case '2':
        return 'z';
    case '0':
        return ' ';
    case '.':
        return ':';
    case 1001:
        return 'b';
    case 1002:
        return 'c';
    default:
        return 0;
    }
}
static bool cell_commit(App *a) {
    if (!a->cell_active)
        return true;
    static Value v;
    if (!dce_eval_text(&a->m, a->cell, &v) || v.len != 1 || !isfinite(v.v[0])) {
        notice(a, "Cell needs a finite real number");
        return false;
    }
    Real x = v.v[0];
    Session *s = &a->m.s;
    if (a->mode == 3) {
        if (a->table_col)
            s->y[a->table_row] = x;
        else
            s->x[a->table_row] = x;
        if (s->points <= a->table_row)
            s->points = a->table_row + 1;
    } else if (a->mode == 4) {
        Real bounds[4] = {s->xmin, s->xmax, s->ymin, s->ymax};
        bounds[a->field] = x;
        if (fabsl(x) > 1e12L || bounds[0] >= bounds[1] || bounds[2] >= bounds[3] ||
            bounds[1] - bounds[0] < 1e-9L || bounds[3] - bounds[2] < 1e-9L) {
            notice(a, "Invalid window (max > min required)");
            return false;
        }
        s->xmin = bounds[0];
        s->xmax = bounds[1];
        s->ymin = bounds[2];
        s->ymax = bounds[3];
    } else if (a->mode == 6 && s->sliders) {
        Parameter p = s->slider[a->table_row];
        if (a->field == 0)
            p.value = x;
        if (a->field == 1)
            p.min = x;
        if (a->field == 2)
            p.max = x;
        if (a->field == 3)
            p.step = x;
        if (p.min >= p.max || p.step <= 0 || p.value < p.min || p.value > p.max) {
            notice(a, "Slider value/bounds/step invalid");
            return false;
        }
        s->slider[a->table_row] = p;
    }
    a->cell_active = false;
    a->cell_len = 0;
    a->cell[0] = 0;
    app_rebuild(a);
    return true;
}
static void zoom(App *a, Real factor) {
    Session *s = &a->m.s;
    Real cx = (s->xmin + s->xmax) / 2, cy = (s->ymin + s->ymax) / 2,
         dx = (s->xmax - s->xmin) * factor / 2, dy = (s->ymax - s->ymin) * factor / 2;
    if (dx < 1e-8L || dy < 1e-8L || fabsl(cx) + dx > 1e12L || fabsl(cy) + dy > 1e12L)
        return;
    s->xmin = cx - dx;
    s->xmax = cx + dx;
    s->ymin = cy - dy;
    s->ymax = cy + dy;
    a->graph_dirty = true;
    a->trace = false;
}
static void fit_view(App *a) {
    Session *s = &a->m.s;
    if (!s->points) {
        notice(a, "Zoom fit needs table data");
        return;
    }
    Real xmin = s->x[0], xmax = xmin, ymin = s->y[0], ymax = ymin;
    for (unsigned i = 1; i < s->points; i++) {
        xmin = fminl(xmin, s->x[i]);
        xmax = fmaxl(xmax, s->x[i]);
        ymin = fminl(ymin, s->y[i]);
        ymax = fmaxl(ymax, s->y[i]);
    }
    Real dx = fmaxl((xmax - xmin) * .15L, 1), dy = fmaxl((ymax - ymin) * .15L, 1);
    if (!isfinite(dx) || !isfinite(dy) || fabsl(xmin) + dx > 1e12L || fabsl(xmax) + dx > 1e12L ||
        fabsl(ymin) + dy > 1e12L || fabsl(ymax) + dy > 1e12L) {
        notice(a, "Data outside supported graph window");
        return;
    }
    s->xmin = xmin - dx;
    s->xmax = xmax + dx;
    s->ymin = ymin - dy;
    s->ymax = ymax + dy;
    a->graph_dirty = true;
}
void app_key(App *a, int key) {
    a->dirty = true;
    if (key == K_SECOND) {
        a->second = !a->second;
        return;
    }
    if (key == K_ALPHA) {
        if (a->second) {
            a->alpha_lock = !a->alpha_lock;
            a->alpha = a->alpha_lock;
        } else
            a->alpha = !a->alpha;
        a->second = 0;
        return;
    }
    bool second = a->second;
    a->second = 0;
    if (a->alpha) {
        int letter = alpha_char(key);
        if (letter) {
            key = letter;
            if (!a->alpha_lock)
                a->alpha = 0;
        }
    }
    if (second && key == K_MODE) {
        if (commit(a) && cell_commit(a))
            a->quit = true;
        return;
    }
    if ((a->mode == 3 || a->mode == 4 || a->mode == 6) && a->cell_active) {
        if (key == K_ENTER) {
            if (cell_commit(a) && a->mode == 3) {
                if (a->table_col == 0)
                    a->table_col = 1;
                else if (a->table_row + 1 < DCE_LIST) {
                    a->table_col = 0;
                    a->table_row++;
                }
            }
            return;
        }
        if (key == K_CLEAR) {
            a->cell_active = false;
            a->cell_len = 0;
            return;
        }
        if (key == K_DEL) {
            if (a->cell_len)
                a->cell[--a->cell_len] = 0;
            return;
        }
        if (key >= 32 && key < 127 && a->cell_len < sizeof(a->cell) - 1) {
            a->cell[a->cell_len++] = key;
            a->cell[a->cell_len] = 0;
            return;
        }
    }
    if (key == K_YEQ) {
        if (!commit(a) || !cell_commit(a))
            return;
        a->mode = a->mode == 0 ? 1 : 0;
        a->graph_dirty = true;
        return;
    }
    if (key == K_GRAPH) {
        if (!commit(a) || !cell_commit(a))
            return;
        a->mode = 2;
        a->graph_dirty = true;
        return;
    }
    if (key == K_STAT) {
        if (!commit(a) || !cell_commit(a))
            return;
        if (second) {
            a->mode = 0;
            ed_init(&a->editor, "y_1~mx_1+b");
            commit(a);
        } else {
            a->mode = 3;
            a->table_row = 0;
            a->table_col = 0;
        }
        return;
    }
    if (key == K_WINDOW) {
        if (!commit(a) || !cell_commit(a))
            return;
        a->mode = second ? 6 : 4;
        a->table_row = 0;
        a->field = 0;
        return;
    }
    if (key == K_MODE) {
        a->m.s.degrees = !a->m.s.degrees;
        app_rebuild(a);
        notice(a, a->m.s.degrees ? "Angle mode: degrees" : "Angle mode: radians");
        return;
    }
    if (key == K_ZOOM) {
        if (!commit(a))
            return;
        if (second) {
            a->m.s.xmin = a->m.s.ymin = -10;
            a->m.s.xmax = a->m.s.ymax = 10;
            a->graph_dirty = true;
        } else
            fit_view(a);
        return;
    }
    if (key == K_TRACE) {
        if (!commit(a))
            return;
        if (second) {
            a->m.s.row[a->selected].visible = !a->m.s.row[a->selected].visible;
            a->graph_dirty = true;
        } else {
            a->mode = 2;
            a->trace_count = dce_analyze(&a->m, a->interest, 40, a->cancel, a->cancel_user);
            a->trace_index = 0;
            a->trace = a->trace_count > 0;
            a->graph_dirty = true;
            if (!a->trace)
                notice(a, "No scalar explicit points found");
        }
        return;
    }
    if (second && key == K_ENTER) {
        if (commit(a)) {
            a->mode = 8;
            a->field = 0;
        }
        return;
    }
    if (a->mode == 8) {
        if (key == K_UP && a->field)
            a->field--;
        if (key == K_DOWN && a->field + 8 < a->fit.observations)
            a->field++;
        if (key == K_CLEAR) {
            a->mode = 0;
            a->graph_dirty = true;
        }
        return;
    }
    if (a->mode == 5) {
        if (key == K_UP && a->palette)
            a->palette--;
        if (key == K_DOWN && (unsigned)a->palette + 1 < PALETTE_COUNT)
            a->palette++;
        if (key == K_LEFT && a->palette >= 8)
            a->palette -= 8;
        if (key == K_RIGHT && (unsigned)a->palette + 8 < PALETTE_COUNT)
            a->palette += 8;
        if (key == K_CLEAR) {
            a->mode = 0;
            a->graph_dirty = true;
        }
        if (key == K_ENTER) {
            a->mode = 0;
            a->graph_dirty = true;
            unsigned p = a->palette;
            if (p == 2)
                ed_template(&a->editor, E_FRAC);
            else if (p == 3)
                ed_template(&a->editor, E_ROOT);
            else if (p == 4)
                ed_template(&a->editor, E_POWER);
            else if (p == 5)
                ed_template(&a->editor, E_SUB);
            else if (p >= 53)
                ed_init(&a->editor, p == 53   ? "y_1~mx_1+b"
                                    : p == 54 ? "y_1~ax_1^2+bx_1+c"
                                              : "y_1~a*b^(x_1)");
            else if (p == 6 || p == 7) {
                ed_char(&a->editor, p == 6 ? 'x' : 'y');
                ed_template(&a->editor, E_SUB);
                ed_char(&a->editor, '1');
                ed_right(&a->editor);
            } else
                text(&a->editor, palette[p]);
            ed_record(&a->editor);
        }
        return;
    }
    if (a->mode == 3 || a->mode == 4 || a->mode == 6) {
        Session *s = &a->m.s;
        if (a->mode == 3) {
            if (key == K_UP && a->table_row)
                a->table_row--;
            if (key == K_DOWN && a->table_row + 1 < DCE_LIST)
                a->table_row++;
            if (key == K_LEFT)
                a->table_col = 0;
            if (key == K_RIGHT)
                a->table_col = 1;
            if (second && key == K_DEL && a->table_row < s->points) {
                for (unsigned i = a->table_row; i + 1 < s->points; i++) {
                    s->x[i] = s->x[i + 1];
                    s->y[i] = s->y[i + 1];
                }
                s->points--;
                app_rebuild(a);
                return;
            }
        }
        if (a->mode == 4) {
            if (key == K_UP && a->field)
                a->field--;
            if (key == K_DOWN && a->field < 3)
                a->field++;
            if (key == K_MATH) {
                s->grid = !s->grid;
                a->graph_dirty = true;
                return;
            }
        }
        if (a->mode == 6) {
            if (!s->sliders) {
                notice(a, "Enter y=ax^2+b to create sliders");
                return;
            }
            if (key == K_UP && a->table_row)
                a->table_row--;
            if (key == K_DOWN && a->table_row + 1 < s->sliders)
                a->table_row++;
            Parameter *p = &s->slider[a->table_row];
            if (key == K_LEFT || key == K_RIGHT) {
                p->value =
                    fmaxl(p->min, fminl(p->max, p->value + (key == K_LEFT ? -1 : 1) * p->step));
                app_rebuild(a);
            }
            if (key == K_MATH) {
                a->field = (a->field + 1) % 4;
                return;
            }
        }
        if (key == K_ENTER || (key >= 32 && key < 127)) {
            a->cell_active = true;
            a->cell_len = 0;
            a->cell[0] = 0;
            if (key < 127) {
                a->cell[0] = key;
                a->cell[1] = 0;
                a->cell_len = 1;
            }
        }
        return;
    }
    if (key == K_MATH) {
        a->mode = 5;
        return;
    }
    if (a->mode == 2) {
        Session *s = &a->m.s;
        if (a->trace && (key == K_LEFT || key == K_RIGHT)) {
            a->trace_index =
                (a->trace_index + a->trace_count + (key == K_LEFT ? -1 : 1)) % a->trace_count;
            a->graph_dirty = true;
            return;
        }
        Real dx = (s->xmax - s->xmin) * .12L, dy = (s->ymax - s->ymin) * .12L;
        if (key == K_LEFT && s->xmin - dx >= -1e12L) {
            s->xmin -= dx;
            s->xmax -= dx;
        }
        if (key == K_RIGHT && s->xmax + dx <= 1e12L) {
            s->xmin += dx;
            s->xmax += dx;
        }
        if (key == K_UP && s->ymax + dy <= 1e12L) {
            s->ymin += dy;
            s->ymax += dy;
        }
        if (key == K_DOWN && s->ymin - dy >= -1e12L) {
            s->ymin -= dy;
            s->ymax -= dy;
        }
        if (key == '+')
            zoom(a, .75);
        if (key == '-')
            zoom(a, 1 / .75);
        if (key == K_CLEAR)
            a->trace = false;
        a->graph_dirty = true;
        return;
    }
    if (second) {
        if (key == K_LEFT) {
            ed_undo(&a->editor);
            return;
        }
        if (key == K_RIGHT) {
            ed_redo(&a->editor);
            return;
        }
        if (key == K_UP || key == K_DOWN) {
            if (!commit(a))
                return;
            int next = a->selected + (key == K_UP ? -1 : 1);
            if (next >= 0 && next < a->m.s.rows)
                choose(a, next);
            return;
        }
        if (key == K_DEL) {
            if (!commit(a))
                return;
            if (a->m.s.rows < DCE_ROWS) {
                unsigned n = a->m.s.rows++;
                a->m.s.row[n] = a->m.s.row[a->selected];
                a->m.s.row[n].color = n % 6;
                choose(a, n);
                app_rebuild(a);
            }
            return;
        }
        if (key == K_CLEAR) {
            unsigned n = a->selected;
            for (unsigned i = n; i + 1 < a->m.s.rows; i++)
                a->m.s.row[i] = a->m.s.row[i + 1];
            if (a->m.s.rows > 1)
                a->m.s.rows--;
            else
                a->m.s.row[0].text[0] = 0;
            memset(&a->m.s.row[a->m.s.rows], 0, (DCE_ROWS - a->m.s.rows) * sizeof(Row));
            choose(a, n < a->m.s.rows ? n : (unsigned)a->m.s.rows - 1);
            app_rebuild(a);
            return;
        }
        if (key == '+') {
            ed_write(&a->editor, a->clipboard, sizeof(a->clipboard));
            notice(a, "Expression copied");
            return;
        }
        if (key == '-') {
            ed_load(&a->editor, a->clipboard);
            ed_record(&a->editor);
            return;
        }
        if (key == K_STORE)
            key = '~';
        else if (key == '^')
            key = '_';
        else if (key == '(')
            key = '[';
        else if (key == ')')
            key = ']';
        else if (key == '.')
            key = ':';
        else if (key == '*')
            key = '<';
        else if (key == '/')
            key = '>';
        else if (key == K_X)
            key = 'L';
        else if (key == K_SQUARE) {
            ed_template(&a->editor, E_ROOT);
            ed_record(&a->editor);
            return;
        }
    }
    if (key == K_ENTER) {
        if (!commit(a))
            return;
        if (a->selected + 1 == a->m.s.rows && a->m.s.rows < DCE_ROWS) {
            a->m.s.rows++;
            a->m.s.row[a->m.s.rows - 1].visible = 1;
            a->m.s.row[a->m.s.rows - 1].color = (a->m.s.rows - 1) % 6;
        }
        if (a->selected + 1 < a->m.s.rows)
            choose(a, a->selected + 1);
        return;
    }
    if (key == K_LEFT)
        ed_left(&a->editor);
    else if (key == K_RIGHT)
        ed_right(&a->editor);
    else if (key == K_UP)
        ed_vertical(&a->editor, -1);
    else if (key == K_DOWN)
        ed_vertical(&a->editor, 1);
    else if (key == K_DEL)
        ed_delete(&a->editor);
    else if (key == K_CLEAR)
        ed_load(&a->editor, "");
    else if (key == K_SQUARE || key == K_RECIP) {
        ed_template(&a->editor, E_POWER);
        text(&a->editor, key == K_SQUARE ? "2" : "-1");
        ed_right(&a->editor);
    } else if (key == K_SIN)
        text(&a->editor, second ? "asin(" : "sin(");
    else if (key == K_COS)
        text(&a->editor, second ? "acos(" : "cos(");
    else if (key == K_TAN)
        text(&a->editor, second ? "atan(" : "tan(");
    else if (key == K_LOG)
        text(&a->editor, "log(");
    else if (key == K_LN)
        text(&a->editor, "ln(");
    else if (key == K_STORE)
        ed_char(&a->editor, '=');
    else if (key == K_X)
        ed_char(&a->editor, 'x');
    else if (key >= 32 && key < 127) {
        if (!ed_char(&a->editor, key))
            notice(a, "Editor node limit reached");
    }
    ed_record(&a->editor);
}
static void print(Draw *d, int x, int y, const char *s, unsigned c) {
    d->text(d->user, x, y, s, c);
}
static void num(Draw *d, int x, int y, Real z, unsigned c) {
    char s[24];
    dce_format(z, s, sizeof(s), 7);
    print(d, x, y, s, c);
}
void app_draw(App *a, Draw *d) {
    static Editor display;
    Session *s = &a->m.s;
    d->clip(d->user, 0, 0, 320, 240);
    if (a->mode > 2 || a->graph_dirty)
        d->rect(d->user, 0, 0, 320, 240, C_WHITE);
    d->rect(d->user, 0, 0, 320, 22, C_PANEL);
    print(d, 7, 7, "DesmosCE", C_TEXT);
    print(d, 191, 7, s->degrees ? "DEG" : "RAD", C_MUTED);
    if (a->alpha)
        print(d, 231, 7, a->alpha_lock ? "aLOCK" : "alpha", C_BLUE);
    if (a->second)
        print(d, 278, 7, "2nd", C_BLUE);
    if (a->mode <= 2) {
        int left = a->mode == 0 ? 147 : 0;
        if (a->mode != 1 && a->graph_dirty)
            graph_draw(&a->m, d, left, 23, 320 - left, 197, a->cancel, a->cancel_user);
        if (a->mode != 2) {
            int width = a->mode == 1 ? 320 : 146;
            d->rect(d->user, 0, 23, width, 197, C_WHITE);
            unsigned start = a->selected >= 3 ? a->selected - 2 : 0;
            int y = 29;
            for (unsigned i = start; i < s->rows && y < 207; i++) {
                bool selected = i == a->selected;
                Editor *e = selected ? &a->editor : &display;
                if (!selected)
                    ed_load(e, s->row[i].text);
                Box b = ed_measure(e, 0);
                int h = b.a + b.d + 23;
                if (h < 43)
                    h = 43;
                if (h > 84)
                    h = 84;
                d->clip(d->user, 0, y, width, h < 220 - y ? h : 220 - y);
                if (selected)
                    d->rect(d->user, 0, y, width, h - 1, C_SELECT);
                d->rect(d->user, 6, y + 10, 6, 6,
                        s->row[i].visible ? C_RED + s->row[i].color : C_GRID);
                if (selected) {
                    int shift = a->scroll;
                    if (e->cx > width - 20)
                        shift += e->cx - (width - 20);
                    if (e->cx < 22)
                        shift -= 22 - e->cx;
                    if (shift < 0)
                        shift = 0;
                    a->scroll = shift;
                    ed_draw(e, d, 20 - shift, y + b.a + 4, C_TEXT, true);
                } else
                    ed_draw(e, d, 20, y + b.a + 4, C_TEXT, false);
                int info = y + h - 13;
                Program *p = &a->m.p[i];
                if (p->error[0])
                    print(d, 20, info, p->error, C_RED);
                else if (p->root >= 0) {
                    Node *root = &p->n[p->root];
                    if (root->kind == N_REG) {
                        if (a->fit.status == FIT_OK && a->fit.count) {
                            static char result[48], n1[20];
                            dce_format(a->fit.p[0].value, n1, sizeof(n1), 6);
                            snprintf(result, sizeof(result), "%s = %s  [2nd ENTER]",
                                     a->fit.p[0].name, n1);
                            print(d, 20, info, result, C_BLUE);
                        } else
                            print(d, 20, info, dce_fit_status(a->fit.status), C_RED);
                    } else if (!graph_explicit(&a->m, i) || !strchr(s->row[i].text, 'x')) {
                        static Value v;
                        static Eval ectx;
                        ectx = dce_context(&a->m, 0, 0);
                        int target = root->kind == N_EQ ? root->b : p->root;
                        if (dce_eval(&ectx, p, target, &v)) {
                            if (v.len == 1)
                                num(d, 20, info, v.v[0], C_MUTED);
                            else {
                                char q[32];
                                snprintf(q, sizeof(q), "list: %u values", v.len);
                                print(d, 20, info, q, C_MUTED);
                            }
                        } else
                            print(d, 20, info, ectx.error[0] ? ectx.error : "Outside real domain",
                                  C_RED);
                    }
                }
                if (selected && (e->read_only || e->overflow))
                    print(d, 20, info, "Editor capacity limit", C_RED);
                d->line(d->user, 0, y + h - 1, width, y + h - 1, C_GRID);
                y += h;
            }
            d->clip(d->user, 0, 0, 320, 240);
            if (a->mode == 0)
                d->line(d->user, 146, 23, 146, 219, C_GRID);
        }
        if (a->mode == 2 && a->trace) {
            Interest *p = &a->interest[a->trace_index];
            int x = graph_px(s, p->x, 0, 320), y = graph_py(s, p->y, 23, 197);
            d->clip(d->user, 0, 23, 320, 197);
            d->line(d->user, x - 4, y, x + 4, y, C_BLACK);
            d->line(d->user, x, y - 4, x, y + 4, C_BLACK);
            d->clip(d->user, 0, 0, 320, 240);
            d->rect(d->user, 8, 28, 212, 28, C_PANEL);
            static const char *const names[] = {"root", "y-intercept", "extremum", "intersection"};
            print(d, 12, 31, names[p->kind], C_TEXT);
            static char xx[24], yy[24], label[60];
            dce_format(p->x, xx, sizeof(xx), 8);
            dce_format(p->y, yy, sizeof(yy), 8);
            snprintf(label, sizeof(label), "(%s, %s)", xx, yy);
            print(d, 12, 43, label, C_BLUE);
        }
    } else if (a->mode == 3) {
        print(d, 12, 30, "Data table    STAT: x_1 / y_1", C_TEXT);
        print(d, 53, 49, "x_1", C_BLUE);
        print(d, 179, 49, "y_1", C_RED);
        unsigned start = a->table_row / 8 * 8;
        for (unsigned k = 0; k < 8 && start + k < DCE_LIST; k++) {
            unsigned i = start + k;
            int y = 66 + k * 18;
            char label[8];
            snprintf(label, sizeof(label), "%u", i + 1);
            print(d, 10, y + 3, label, C_MUTED);
            for (unsigned c = 0; c < 2; c++) {
                int x = 38 + c * 133;
                if (i == a->table_row && c == a->table_col)
                    d->rect(d->user, x, y, 129, 16, C_SELECT);
                if (i == a->table_row && c == a->table_col && a->cell_active)
                    print(d, x + 4, y + 3, a->cell, C_TEXT);
                else if (i < s->points)
                    num(d, x + 4, y + 3, c ? s->y[i] : s->x[i], C_TEXT);
            }
            d->line(d->user, 37, y + 17, 305, y + 17, C_GRID);
        }
    } else if (a->mode == 4) {
        print(d, 12, 32, "Window bounds", C_TEXT);
        const char *labels[] = {"x minimum", "x maximum", "y minimum", "y maximum"};
        Real bounds[] = {s->xmin, s->xmax, s->ymin, s->ymax};
        for (unsigned i = 0; i < 4; i++) {
            int y = 55 + 30 * i;
            if (i == a->field)
                d->rect(d->user, 8, y - 3, 304, 24, C_SELECT);
            print(d, 15, y, labels[i], C_TEXT);
            if (a->cell_active && i == a->field)
                print(d, 144, y, a->cell, C_BLUE);
            else
                num(d, 144, y, bounds[i], C_BLUE);
        }
        print(d, 12, 192, s->grid ? "MATH: grid ON" : "MATH: grid OFF", C_MUTED);
    } else if (a->mode == 5) {
        print(d, 12, 31, "Functions / templates", C_TEXT);
        unsigned start = a->palette / 9 * 9;
        for (unsigned i = 0; i < 9 && start + i < PALETTE_COUNT; i++) {
            unsigned p = start + i;
            int y = 52 + i * 18;
            if (p == a->palette)
                d->rect(d->user, 7, y - 2, 305, 17, C_SELECT);
            print(d, 15, y, palette[p], C_TEXT);
        }
        print(d, 12, 215, "Arrows select. ENTER inserts.", C_MUTED);
    } else if (a->mode == 6) {
        print(d, 12, 31, "Sliders | arrows adjust | MATH field", C_TEXT);
        for (unsigned i = 0; i < s->sliders; i++) {
            Parameter *p = &s->slider[i];
            int y = 51 + i * 22;
            if (i == a->table_row)
                d->rect(d->user, 8, y - 2, 303, 20, C_SELECT);
            print(d, 14, y, p->name, C_TEXT);
            num(d, 42, y, p->value, C_BLUE);
            d->line(d->user, 142, y + 5, 300, y + 5, C_GRID);
            int x = 142 + (int)(158 * (p->value - p->min) / (p->max - p->min));
            d->rect(d->user, x - 2, y + 1, 5, 9, C_BLUE);
        }
        if (s->sliders) {
            Parameter *p = &s->slider[a->table_row];
            const char *names[] = {"value", "minimum", "maximum", "step"};
            Real values[] = {p->value, p->min, p->max, p->step};
            print(d, 12, 199, names[a->field], C_TEXT);
            if (a->cell_active)
                print(d, 110, 199, a->cell, C_BLUE);
            else
                num(d, 110, 199, values[a->field], C_BLUE);
        } else
            print(d, 12, 65, "Use a parameter, e.g. y=ax^2+b", C_MUTED);
    } else if (a->mode == 8) {
        print(d, 12, 31, dce_fit_status(a->fit.status), a->fit.status == FIT_OK ? C_GREEN : C_RED);
        for (unsigned i = 0; i < a->fit.count; i++) {
            int x = 12 + (i % 3) * 102, y = 51 + (i / 3) * 18;
            print(d, x, y, a->fit.p[i].name, C_TEXT);
            num(d, x + 18, y, a->fit.p[i].value, C_BLUE);
        }
        print(d, 12, 92, "RMSE", C_TEXT);
        num(d, 60, 92, a->fit.rmse, C_BLUE);
        if (a->fit.has_r2) {
            print(d, 164, 92, "R^2", C_TEXT);
            num(d, 194, 92, a->fit.r2, C_BLUE);
        }
        print(d, 12, 115, "Residuals (UP/DOWN scroll):", C_TEXT);
        for (unsigned i = 0; i < 8 && i + a->field < a->fit.observations; i++)
            num(d, 20 + (i % 2) * 145, 133 + (i / 2) * 18, a->fit.residual[a->field + i], C_MUTED);
        print(d, 12, 208, "Numerical result; uniqueness not proved.", C_MUTED);
    }
    d->clip(d->user, 0, 0, 320, 240);
    d->rect(d->user, 0, 221, 320, 19, C_PANEL);
    print(d, 5, 226, a->notice, C_MUTED);
    a->graph_dirty = false;
    a->dirty = false;
}
