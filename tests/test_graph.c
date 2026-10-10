#include "ui.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
static Model m;
static Interest p[40];
static bool has(unsigned n, Real x, Real y, unsigned kind) {
    for (unsigned i = 0; i < n; i++)
        if (p[i].kind == kind && fabsl(p[i].x - x) < 1e-6L && fabsl(p[i].y - y) < 1e-6L)
            return true;
    return false;
}
int main(void) {
    dce_init(&m);
    strcpy(m.s.row[0].text, "y=x^2-6x+5");
    dce_compile(&m);
    unsigned n = dce_analyze(&m, p, 40, NULL, NULL);
    assert(has(n, 1, 0, 0) && has(n, 5, 0, 0) && has(n, 3, -4, 2));
    strcpy(m.s.row[0].text, "y=2x+1");
    strcpy(m.s.row[1].text, "y=-x+7");
    m.s.rows = 2;
    dce_compile(&m);
    n = dce_analyze(&m, p, 40, NULL, NULL);
    assert(has(n, 2, 5, 3));
    strcpy(m.s.row[0].text, "y=1/x");
    m.s.rows = 1;
    dce_compile(&m);
    n = dce_analyze(&m, p, 40, NULL, NULL);
    for (unsigned i = 0; i < n; i++)
        assert(p[i].kind != 0);
    strcpy(m.s.row[0].text, "y=(x-2)^2");
    dce_compile(&m);
    n = dce_analyze(&m, p, 40, NULL, NULL);
    assert(has(n, 2, 0, 0));
    assert(graph_px(&m.s, -10, 120, 200) == 120);
    assert(graph_px(&m.s, 10, 120, 200) == 319);
    assert(graph_py(&m.s, 10, 20, 200) == 20);
    assert(graph_y(&m.s, 219, 20, 200) == -10);
    puts("graph: roots, vertex, repeated root, intersection, pole rejection and transforms passed");
    return 0;
}
