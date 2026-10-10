#ifndef DCE_UI_H
#define DCE_UI_H
#include "dce.h"
#define ED_CAP 128
#define ED_HISTORY 10
typedef struct Draw {
    void *user;
    void (*rect)(void *, int, int, int, int, unsigned);
    void (*line)(void *, int, int, int, int, unsigned);
    void (*text)(void *, int, int, const char *, unsigned);
    void (*clip)(void *, int, int, int, int);
} Draw;
enum {
    C_WHITE,
    C_TEXT,
    C_GRID,
    C_PANEL,
    C_RED,
    C_BLUE,
    C_GREEN,
    C_PURPLE,
    C_ORANGE,
    C_BLACK,
    C_SELECT,
    C_MUTED,
    C_SHADE0
};
enum { E_FREE, E_SEQ, E_CHAR, E_FRAC, E_POWER, E_SUB, E_ROOT };
typedef struct {
    int8_t parent, prev, next, head, a, b;
    uint8_t kind;
    char ch;
} EdNode;
typedef struct {
    EdNode n[ED_CAP];
    int8_t seq, before;
    int cx, cy;
    char history[ED_HISTORY][DCE_TEXT];
    uint8_t hcount, hpos;
    bool overflow, read_only;
    char preserved[DCE_TEXT];
} Editor;
typedef struct {
    int w, a, d;
} Box;
void ed_init(Editor *e, const char *s);
void ed_load(Editor *e, const char *s);
bool ed_write(const Editor *e, char *s, size_t cap);
bool ed_char(Editor *e, char ch);
bool ed_template(Editor *e, int kind);
void ed_left(Editor *e);
void ed_right(Editor *e);
void ed_vertical(Editor *e, int dir);
void ed_delete(Editor *e);
void ed_record(Editor *e);
void ed_undo(Editor *e);
void ed_redo(Editor *e);
Box ed_measure(const Editor *e, int n);
void ed_draw(Editor *e, Draw *d, int x, int baseline, unsigned color, bool cursor);
bool graph_explicit(Model *m, unsigned row);
void graph_draw(Model *m, Draw *d, int left, int top, int width, int height, CancelFn cancel,
                void *user);
int graph_px(const Session *s, Real x, int left, int width);
int graph_py(const Session *s, Real y, int top, int height);
Real graph_x(const Session *s, int pixel, int left, int width);
Real graph_y(const Session *s, int pixel, int top, int height);
enum {
    K_LEFT = 256,
    K_RIGHT,
    K_UP,
    K_DOWN,
    K_ENTER,
    K_DEL,
    K_CLEAR,
    K_SECOND,
    K_ALPHA,
    K_YEQ,
    K_GRAPH,
    K_STAT,
    K_TRACE,
    K_WINDOW,
    K_ZOOM,
    K_MODE,
    K_MATH,
    K_SIN,
    K_COS,
    K_TAN,
    K_LOG,
    K_LN,
    K_SQUARE,
    K_RECIP,
    K_STORE,
    K_X
};
typedef struct {
    Model m;
    Editor editor;
    FitResult fit;
    Interest interest[40];
    uint8_t mode, selected, second, alpha, alpha_lock, table_row, table_col, field, palette,
        trace_count, trace_index;
    bool quit, dirty, graph_dirty, trace, cell_active;
    char cell[64], clipboard[DCE_TEXT], notice[80];
    unsigned cell_len;
    int scroll;
    CancelFn cancel;
    void *cancel_user;
} App;
void app_init(App *a);
void app_rebuild(App *a);
void app_key(App *a, int key);
void app_draw(App *a, Draw *d);
#endif
