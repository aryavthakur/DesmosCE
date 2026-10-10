#include "dce.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Model m;
static unsigned checks;
static void near(Real a, Real b, Real eps) {
    checks++;
    if (!isfinite(a) || fabsl(a - b) > eps) {
        fprintf(stderr, "FAIL %u got %.14Lg expected %.14Lg\n", checks, (long double)a,
                (long double)b);
        assert(0);
    }
}
static void val(const char *s, Real x) {
    Value v;
    bool ok = dce_eval_text(&m, s, &v);
    if (!ok)
        fprintf(stderr, "Eval failed: %s\n", s);
    assert(ok && v.len == 1);
    near(v.v[0], x, 1e-7L * (1 + fabsl(x)));
}
static void row(unsigned i, const char *s) {
    strcpy(m.s.row[i].text, s);
    if (m.s.rows <= i)
        m.s.rows = i + 1;
    dce_compile(&m);
}
static void fit(const char *s, const char *param, Real expected) {
    dce_init(&m);
    row(0, s);
    FitResult f;
    dce_fit(&m, &f, NULL, NULL);
    if (f.status != FIT_OK)
        fprintf(stderr, "Fit failed %s: %s\n", s, dce_fit_status(f.status));
    assert(f.status == FIT_OK);
    unsigned i;
    for (i = 0; i < f.count; i++)
        if (!strcmp(f.p[i].name, param))
            break;
    assert(i < f.count);
    near(f.p[i].value, expected, 1e-6L);
}
int main(void) {
    dce_init(&m);
    val("2+3*4", 14);
    val("-2^2", -4);
    val("2^3^2", 512);
    val("2^-2", .25);
    val("3(2+1)", 9);
    val("(2+1)(3-1)", 6);
    val("sqrt(81)", 9);
    val("log(100)", 2);
    val("ln(e^3)", 3);
    val("6!", 720);
    val("gcd(24,36)", 12);
    val("lcm(12,18)", 36);
    val("nCr(6,2)", 15);
    val("sin(pi/2)", 1);
    val("0<2<5", 1);
    val("0<7<5", 0);
    val("{2<0:-1,2>=0:1}", 1);
    row(0, "a=5");
    val("2a+1", 11);
    row(1, "f(x)=x^2+3x-4");
    row(2, "g(x)=sin(x)");
    val("f(3)", 14);
    val("f(g(0))", -4);
    row(3, "L=[1,3,3,5,8]");
    val("mean(L)", 4);
    val("median(L)", 3);
    val("stdev(L)", sqrtl(7));
    val("L[2]", 3);
    val("total([1...10])", 55);
    val("total([1,3...11])", 36);
    val("count(unique(L))", 4);
    val("quantile(L,0.9)", 6.8);
    val("distance((0,0),(3,4))", 5);
    val("diff(x^3,x,2)", 12);
    val("sum(k^2,k,1,5)", 55);
    val("int(x^2,x,0,3)", 9);
    Value v;
    assert(dce_eval_text(&m, "2[1,2,3]", &v) == false);
    assert(dce_eval_text(&m, "[1,2,3]^2", &v));
    near(v.v[2], 9, 1e-9);
    assert(!dce_eval_text(&m, "1/0", &v));
    assert(!dce_eval_text(&m, "sqrt(-1)", &v));
    assert(!dce_eval_text(&m, "[1...33]", &v));
    assert(!dce_eval_text(&m, "L[0]", &v));
    row(4, "b=c+1");
    row(5, "c=b+1");
    assert(!dce_eval_text(&m, "b", &v));
    fit("2k+5~17", "k", 6);
    val("2k", 12);
    fit("7~3m+1", "m", 2);
    fit("k^2~9", "k", 3);
    dce_init(&m);
    row(0, "12~4a+b");
    FitResult f;
    dce_fit(&m, &f, NULL, NULL);
    assert(f.status == FIT_UNDER);
    row(1, "5~a+b");
    dce_fit(&m, &f, NULL, NULL);
    assert(f.status == FIT_OK);
    val("a", 7.0L / 3);
    val("b", 8.0L / 3);
    dce_init(&m);
    m.s.points = 5;
    for (unsigned i = 0; i < 5; i++) {
        m.s.x[i] = i + 1;
        m.s.y[i] = 2 * (i + 1) + 1;
    }
    row(0, "y_1~mx_1+b");
    dce_fit(&m, &f, NULL, NULL);
    assert(f.status == FIT_OK && f.has_r2);
    val("m", 2);
    val("b", 1);
    near(f.r2, 1, 1e-9);
    for (unsigned i = 0; i < 5; i++)
        m.s.y[i] = (m.s.x[i] + 2) * (m.s.x[i] + 2);
    row(0, "y_1~ax_1^2+bx_1+c");
    dce_fit(&m, &f, NULL, NULL);
    assert(f.status == FIT_OK);
    val("a", 1);
    val("b", 4);
    val("c", 4);
    for (unsigned i = 0; i < 5; i++)
        m.s.x[i] = 1;
    dce_compile(&m);
    dce_fit(&m, &f, NULL, NULL);
    assert(f.status == FIT_RANK);
    char b[48];
    dce_format(123.45, b, sizeof(b), 7);
    assert(!strcmp(b, "123.45"));
    dce_format(-.00000123, b, sizeof(b), 7);
    assert(!strcmp(b, "-1.23e-6"));
    printf("core: %u numerical checks plus parser/domain/fit assertions passed\n", checks);
    return 0;
}
