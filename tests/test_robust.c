#include "ui.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Model m;
static Editor editor;
static Program p;
static uint32_t state = 0xCE2026;
static unsigned rng(void) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}
static void value(const char *s, Real expected) {
    Value v;
    assert(dce_eval_text(&m, s, &v) && v.len == 1);
    assert(fabsl(v.v[0] - expected) < 1e-7L * (1 + fabsl(expected)));
}
static void roundtrip(const char *s) {
    char a[DCE_TEXT], b[DCE_TEXT];
    Value x, y;
    ed_init(&editor, s);
    assert(ed_write(&editor, a, sizeof(a)));
    assert(dce_eval_text(&m, s, &x));
    if (!dce_eval_text(&m, a, &y))
        fprintf(stderr, "roundtrip %s -> %s\n", s, a);
    assert(dce_eval_text(&m, a, &y));
    assert(x.len == y.len);
    for (unsigned i = 0; i < x.len; i++)
        assert(fabsl(x.v[i] - y.v[i]) < 1e-12L * (1 + fabsl(x.v[i])));
    for (unsigned i = 0; i < 10; i++) {
        ed_load(&editor, a);
        assert(ed_write(&editor, b, sizeof(b)));
        assert(!strcmp(a, b));
        strcpy(a, b);
    }
}
int main(void) {
    dce_init(&m);
    strcpy(m.s.row[0].text, "f(a)=sum(a*k,k,1,3)");
    dce_compile(&m);
    value("f(2)", 12);
    value("sum(sum(k*j,j,1,3),k,1,2)", 18);
    const char *cases[] = {"2^sin(1)",   "2^sqrt(4)",  "2^-2^2",
                           "1/sin(2)^2", "2^(2+1)(3)", "sin(2)^2+cos(2)^2",
                           "log(100)^2", "[1,2,3]^2",  "(2+3)/((4+1)/2)",
                           "2/3/4",      "2^2^3"};
    for (unsigned i = 0; i < sizeof(cases) / sizeof(*cases); i++)
        roundtrip(cases[i]);
    strcpy(m.s.row[0].text, "sin(x)=0.5");
    dce_compile(&m);
    value("sin(pi/2)", 1);
    assert(!graph_explicit(&m, 0));
    Real z;
    assert(!dce_scalar(&m, 0, 1, 0, &z));
    strcpy(m.s.row[0].text, "y=x^2 {0<x<5}");
    dce_compile(&m);
    assert(dce_scalar(&m, 0, 2, 0, &z) && z == 4);
    assert(!dce_scalar(&m, 0, -2, 0, &z));
    char b[64];
    dce_format(DBL_TRUE_MIN, b, sizeof(b), 9);
    assert(strstr(b, "e-324"));
    dce_format(DBL_MAX, b, sizeof(b), 9);
    assert(strstr(b, "e308"));
    char huge[DCE_TEXT];
    memset(huge, '1', sizeof(huge) - 1);
    huge[sizeof(huge) - 1] = 0;
    ed_init(&editor, huge);
    assert(editor.read_only);
    char saved[DCE_TEXT];
    assert(ed_write(&editor, saved, sizeof(saved)) && !strcmp(saved, huge));
    uint8_t wire[4096];
    size_t n = dce_pack(&m.s, wire, sizeof(wire));
    assert(n);
    Session copy;
    for (size_t i = 0; i < n; i++) {
        wire[i] ^= 1;
        assert(!dce_unpack(&copy, wire, n));
        wire[i] ^= 1;
    }
    const char alphabet[] = "0123456789xyzab+-*/^~=<>()[]{},.!_ sincosqrt";
    for (unsigned t = 0; t < 12000; t++) {
        char s[DCE_TEXT];
        unsigned len = rng() % DCE_TEXT;
        for (unsigned i = 0; i < len; i++)
            s[i] = alphabet[rng() % (sizeof(alphabet) - 1)];
        s[len] = 0;
        if (dce_parse(&p, s) && p.root >= 0) {
            Eval e = dce_context(&m, 1, 1);
            Value v;
            dce_eval(&e, &p, p.root, &v);
        }
        ed_init(&editor, s);
        char out[DCE_TEXT];
        ed_write(&editor, out, sizeof(out));
        for (unsigned k = 0; k < 6; k++) {
            switch (rng() % 6) {
            case 0:
                ed_left(&editor);
                break;
            case 1:
                ed_right(&editor);
                break;
            case 2:
                ed_vertical(&editor, 1);
                break;
            case 3:
                ed_vertical(&editor, -1);
                break;
            case 4:
                ed_delete(&editor);
                break;
            case 5:
                ed_char(&editor, '2');
                break;
            }
        }
    }
    puts("robust: 12000 malformed-input/editor trials, lexical scopes, semantic round-trips, "
         "subnormals, all-byte save corruption passed");
    return 0;
}
