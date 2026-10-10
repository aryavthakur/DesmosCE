#ifndef DCE_H
#define DCE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#ifdef __TICE__
typedef long double Real; /* CE double is binary32; long double is binary64. */
#else
typedef double Real;
#endif
#define DCE_ROWS 10
#define DCE_TEXT 192
#define DCE_NODES 128
#define DCE_LIST 32
#define DCE_PARAMS 6
#define DCE_RESID 48
#define DCE_NAME 12
#define DCE_PI ((Real)3.1415926535897932384626433832795L)
#define DCE_E ((Real)2.7182818284590452353602874713527L)
typedef enum {
    N_NUM,
    N_VAR,
    N_ADD,
    N_SUB,
    N_MUL,
    N_DIV,
    N_POW,
    N_NEG,
    N_CALL,
    N_ARG,
    N_LIST,
    N_RANGE,
    N_POINT,
    N_EQ,
    N_LT,
    N_LE,
    N_GT,
    N_GE,
    N_REG,
    N_AND,
    N_OR,
    N_IF,
    N_INDEX,
    N_FACT
} NodeKind;
typedef struct {
    union {
        Real number;
        char name[DCE_NAME];
    };
    int8_t a, b, c;
    uint8_t kind;
} Node;
typedef struct {
    Node n[DCE_NODES];
    uint8_t used;
    int8_t root;
    char error[56];
} Program;
typedef struct {
    Real v[DCE_LIST];
    uint8_t len, point;
} Value;
typedef struct {
    char text[DCE_TEXT];
    uint8_t visible, color;
} Row;
typedef struct {
    char name[DCE_NAME];
    Real value, min, max, step;
} Parameter;
typedef struct {
    Row row[DCE_ROWS];
    Real x[DCE_LIST], y[DCE_LIST];
    Parameter slider[DCE_PARAMS];
    Real xmin, xmax, ymin, ymax;
    uint8_t rows, points, sliders, degrees, grid;
    uint32_t generation;
} Session;
typedef struct {
    Session s;
    Program p[DCE_ROWS];
    Parameter fit[DCE_PARAMS];
    uint8_t fits;
    char message[80];
} Model;
typedef struct Binding {
    const char *name;
    Real value;
    const struct Binding *parent;
} Binding;
typedef struct {
    Model *model;
    Real x, y, t;
    const Parameter *params;
    uint8_t count;
    uint16_t active;
    uint8_t depth;
    unsigned budget;
    const Binding *bindings;
    char error[56];
} Eval;
typedef enum {
    FIT_OK,
    FIT_NO_DATA,
    FIT_UNDER,
    FIT_RANK,
    FIT_DOMAIN,
    FIT_LIMIT,
    FIT_CANCEL
} FitStatus;
typedef struct {
    FitStatus status;
    Parameter p[DCE_PARAMS];
    uint8_t count, observations;
    unsigned iterations;
    Real sse, rmse, r2;
    bool has_r2;
    Real residual[DCE_RESID];
} FitResult;
typedef bool (*CancelFn)(void *);
void dce_init(Model *m);
bool dce_parse(Program *p, const char *text);
size_t dce_name_length(const char *s);
bool dce_window_valid(const Session *s);
void dce_compile(Model *m);
Eval dce_context(Model *m, Real x, Real y);
bool dce_eval(Eval *e, const Program *p, int node, Value *v);
bool dce_scalar(Model *m, int row, Real x, Real y, Real *out);
bool dce_defined(const Model *m, const char *name);
int dce_function_arg(const Program *p, int lhs);
unsigned dce_unknowns(Model *m, bool regressions, Parameter *out, unsigned cap);
void dce_add_sliders(Model *m);
bool dce_eval_text(Model *m, const char *text, Value *v);
void dce_fit(Model *m, FitResult *r, CancelFn cancel, void *user);
const char *dce_fit_status(FitStatus s);
bool dce_qr(Real *a, Real *b, unsigned rows, unsigned cols, Real *x, unsigned *rank);
void dce_format(Real x, char *out, size_t n, unsigned digits);
size_t dce_pack(const Session *s, uint8_t *out, size_t cap);
bool dce_unpack(Session *s, const uint8_t *data, size_t size);
typedef struct {
    Real x, y;
    uint8_t kind, row;
} Interest;
unsigned dce_analyze(Model *m, Interest *out, unsigned cap, CancelFn cancel, void *user);
#endif
