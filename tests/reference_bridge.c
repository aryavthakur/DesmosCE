#include "dce.h"
#include <string.h>
static Model m;
int reference_fit(const char *expression, const double *x, const double *y, unsigned n, char *names,
                  double *values, double *metrics) {
    if (n > DCE_LIST)
        return -1;
    dce_init(&m);
    m.s.points = n;
    for (unsigned i = 0; i < n; i++) {
        m.s.x[i] = x[i];
        m.s.y[i] = y[i];
    }
    strncpy(m.s.row[0].text, expression, DCE_TEXT - 1);
    dce_compile(&m);
    FitResult f;
    dce_fit(&m, &f, 0, 0);
    memset(names, 0, DCE_PARAMS * DCE_NAME);
    for (unsigned i = 0; i < f.count; i++) {
        memcpy(names + i * DCE_NAME, f.p[i].name, DCE_NAME);
        values[i] = f.p[i].value;
    }
    metrics[0] = f.sse;
    metrics[1] = f.rmse;
    metrics[2] = f.has_r2 ? f.r2 : -999;
    metrics[3] = f.iterations;
    return f.status;
}
