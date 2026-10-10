#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static Editor e;
static Model m;
static void equivalence(const char *s) {
    char out[DCE_TEXT];
    Value a, b;
    ed_init(&e, s);
    assert(ed_write(&e, out, sizeof(out)));
    if (!dce_eval_text(&m, out, &b))
        fprintf(stderr, "Editor output rejected: %s -> %s\n", s, out);
    assert(dce_eval_text(&m, s, &a) && dce_eval_text(&m, out, &b));
    assert(a.len == b.len);
    for (unsigned i = 0; i < a.len; i++)
        assert(a.v[i] == b.v[i]);
}
int main(void) {
    dce_init(&m);
    equivalence("(2^2+1)/(2-3)");
    equivalence("2^3^2");
    equivalence("2^-2");
    equivalence("sqrt(4)+2");
    equivalence("1/2^3");
    ed_init(&e, "");
    ed_char(&e, 'x');
    ed_template(&e, E_POWER);
    ed_char(&e, '2');
    ed_record(&e);
    int exponent = e.seq;
    ed_vertical(&e, 1);
    assert(e.seq != exponent);
    ed_vertical(&e, -1);
    assert(e.seq == exponent);
    ed_right(&e);
    ed_char(&e, '+');
    ed_char(&e, '1');
    ed_record(&e);
    char s[DCE_TEXT];
    assert(ed_write(&e, s, sizeof(s)));
    assert(!strcmp(s, "(x)^(2)+1"));
    ed_undo(&e);
    ed_write(&e, s, sizeof(s));
    assert(!strcmp(s, "(x)^(2)"));
    ed_redo(&e);
    ed_write(&e, s, sizeof(s));
    assert(!strcmp(s, "((x))^((2))+1") || !strcmp(s, "(x)^(2)+1"));
    ed_init(&e, "(x^2+1)/(x-3)");
    assert(ed_write(&e, s, sizeof(s)));
    ed_left(&e);
    ed_char(&e, '4');
    assert(ed_write(&e, s, sizeof(s)));
    assert(strstr(s, "x-34"));
    ed_vertical(&e, -1);
    assert(e.seq == e.n[e.n[e.seq].parent].a);
    uint8_t wire[4096];
    m.s.points = 2;
    m.s.x[0] = 1.2345678912345;
    m.s.y[0] = -3;
    m.s.x[1] = 2;
    m.s.y[1] = 9;
    strcpy(m.s.row[0].text, "y=x^2");
    size_t n = dce_pack(&m.s, wire, sizeof(wire));
    assert(n);
    Session copy;
    assert(dce_unpack(&copy, wire, n));
    assert(copy.x[0] == m.s.x[0] && !strcmp(copy.row[0].text, "y=x^2"));
    wire[n - 1] ^= 1;
    assert(!dce_unpack(&copy, wire, n));
    assert(!dce_unpack(&copy, wire, 12));
    puts("editor: structural navigation, round-trip, undo/redo, persistence corruption checks "
         "passed");
    return 0;
}
