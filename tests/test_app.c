#include "ui.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static App a;
int main(void) {
    app_init(&a);
    app_key(&a, 'y');
    app_key(&a, K_STORE);
    app_key(&a, K_X);
    app_key(&a, K_SQUARE);
    app_key(&a, '-');
    app_key(&a, '6');
    app_key(&a, K_X);
    app_key(&a, '+');
    app_key(&a, '5');
    app_key(&a, K_ENTER);
    assert(a.m.s.rows == 2 && a.selected == 1);
    assert(a.m.p[0].root >= 0);
    app_key(&a, K_TRACE);
    assert(a.mode == 2 && a.trace_count >= 3);
    app_key(&a, K_STAT);
    assert(a.mode == 3);
    app_key(&a, '1');
    app_key(&a, K_ENTER);
    app_key(&a, '3');
    app_key(&a, K_ENTER);
    assert(a.m.s.points == 1 && a.m.s.x[0] == 1 && a.m.s.y[0] == 3);
    app_key(&a, K_WINDOW);
    app_key(&a, '9');
    app_key(&a, '9');
    app_key(&a, K_ENTER);
    assert(a.cell_active);
    app_key(&a, K_CLEAR);
    app_key(&a, K_YEQ);
    ed_init(&a.editor, "2k+5~17");
    app_key(&a, K_GRAPH);
    assert(a.fit.status == FIT_OK);
    app_key(&a, K_YEQ);
    ed_init(&a.editor, "y=ax^2+b");
    app_key(&a, K_GRAPH);
    assert(a.m.s.sliders == 2);
    app_key(&a, K_SECOND);
    app_key(&a, K_WINDOW);
    assert(a.mode == 6);
    Real before = a.m.s.slider[0].value;
    app_key(&a, K_RIGHT);
    assert(a.m.s.slider[0].value > before);
    app_key(&a, K_SECOND);
    app_key(&a, K_MODE);
    assert(a.quit);
    puts("app: physical-key workflow, trace, table entry, bounds rejection, fits, sliders, exit "
         "passed");
    return 0;
}
