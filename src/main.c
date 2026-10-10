#include "ui.h"
#ifdef __TICE__
#include <fileioc.h>
#include <graphx.h>
#include <keypadc.h>
#include <stdio.h>
#include <string.h>
#include <sys/timers.h>
#include <ti/getcsc.h>
#include <ti/screen.h>
#include <time.h>
/* Only this adapter touches calculator hardware. */
static App app;
static uint8_t io[4096];
static const char *const slot_names[] = {"DCEA", "DCEB"};
static void rectangle(void *u, int x, int y, int w, int h, unsigned color) {
    (void)u;
    if (w <= 0 || h <= 0)
        return;
    gfx_SetColor(color);
    gfx_FillRectangle(x, y, w, h);
}
static void line(void *u, int x, int y, int xx, int yy, unsigned color) {
    (void)u;
    gfx_SetColor(color);
    gfx_Line(x, y, xx, yy);
}
static void text(void *u, int x, int y, const char *s, unsigned color) {
    (void)u;
    gfx_SetTextFGColor(color);
    gfx_PrintStringXY(s, x, y);
}
static void clip(void *u, int x, int y, int w, int h) {
    (void)u;
    gfx_SetClipRegion(x, y, x + w, y + h);
}
static bool cancel(void *u) {
    (void)u;
    return kb_IsDown(kb_KeyClear);
}
static void colors(void) {
    static const uint8_t rgb[][3] = {{255, 255, 255}, {46, 49, 54},    {229, 232, 235},
                                     {245, 247, 249}, {196, 62, 59},   {47, 111, 185},
                                     {45, 137, 104},  {130, 86, 164},  {221, 143, 41},
                                     {20, 22, 24},    {232, 240, 252}, {117, 124, 134}};
    for (unsigned i = 0; i < sizeof(rgb) / sizeof(*rgb); i++)
        gfx_palette[i] = gfx_RGBTo1555(rgb[i][0], rgb[i][1], rgb[i][2]);
}
static bool read_slot(unsigned slot, Session *s) {
    uint8_t h = ti_Open(slot_names[slot], "r");
    if (!h)
        return false;
    size_t n = ti_GetSize(h);
    bool ok = n <= sizeof(io) && ti_Read(io, 1, n, h) == n;
    ti_Close(h);
    return ok && dce_unpack(s, io, n);
}
static void restore(void) {
    Session s;
    bool found = false;
    for (unsigned i = 0; i < 2; i++)
        if (read_slot(i, &s) && (!found || (int32_t)(s.generation - app.m.s.generation) > 0)) {
            app.m.s = s;
            found = true;
        }
    if (found) {
        ed_init(&app.editor, app.m.s.row[0].text);
        snprintf(app.notice, sizeof(app.notice), "Restored session | 2nd MODE saves + exits");
    }
}
static bool save(void) {
    Session verify;
    app.m.s.generation++;
    size_t n = dce_pack(&app.m.s, io, sizeof(io));
    if (!n)
        return false;
    unsigned slot = app.m.s.generation & 1;
    uint8_t h = ti_Open(slot_names[slot], "w");
    if (!h)
        return false;
    bool ok = ti_Write(io, 1, n, h) == n;
    ti_Close(h);
    if (!ok || !read_slot(slot, &verify) || verify.generation != app.m.s.generation)
        return false;
    h = ti_Open(slot_names[slot], "r");
    if (!h)
        return false;
    ok = ti_SetArchiveStatus(true, h) != 0;
    ti_Close(h);
    return ok;
}
typedef struct {
    kb_lkey_t physical;
    int logical;
} Key;
static const Key keys[] = {
    {kb_Key2nd, K_SECOND},    {kb_KeyAlpha, K_ALPHA}, {kb_KeyYequ, K_YEQ},
    {kb_KeyGraph, K_GRAPH},   {kb_KeyStat, K_STAT},   {kb_KeyTrace, K_TRACE},
    {kb_KeyWindow, K_WINDOW}, {kb_KeyZoom, K_ZOOM},   {kb_KeyMode, K_MODE},
    {kb_KeyMath, K_MATH},     {kb_KeyDel, K_DEL},     {kb_KeyClear, K_CLEAR},
    {kb_KeyEnter, K_ENTER},   {kb_KeyLeft, K_LEFT},   {kb_KeyRight, K_RIGHT},
    {kb_KeyUp, K_UP},         {kb_KeyDown, K_DOWN},   {kb_KeyGraphVar, K_X},
    {kb_KeySin, K_SIN},       {kb_KeyCos, K_COS},     {kb_KeyTan, K_TAN},
    {kb_KeyLog, K_LOG},       {kb_KeyLn, K_LN},       {kb_KeySquare, K_SQUARE},
    {kb_KeyRecip, K_RECIP},   {kb_KeySto, K_STORE},   {kb_KeyApps, 1001},
    {kb_KeyPrgm, 1002},       {kb_Key0, '0'},         {kb_Key1, '1'},
    {kb_Key2, '2'},           {kb_Key3, '3'},         {kb_Key4, '4'},
    {kb_Key5, '5'},           {kb_Key6, '6'},         {kb_Key7, '7'},
    {kb_Key8, '8'},           {kb_Key9, '9'},         {kb_KeyDecPnt, '.'},
    {kb_KeyComma, ','},       {kb_KeyAdd, '+'},       {kb_KeySub, '-'},
    {kb_KeyChs, '-'},         {kb_KeyMul, '*'},       {kb_KeyDiv, '/'},
    {kb_KeyPower, '^'},       {kb_KeyLParen, '('},    {kb_KeyRParen, ')'}};
static int poll(void) {
    static uint8_t previous[sizeof(keys) / sizeof(*keys)];
    static int held;
    static clock_t repeat;
    int result = 0;
    clock_t now = clock();
    for (unsigned i = 0; i < sizeof(keys) / sizeof(*keys); i++) {
        bool down = kb_IsDown(keys[i].physical);
        if (down && !previous[i] && !result) {
            result = keys[i].logical;
            held = result;
            repeat = now + CLOCKS_PER_SEC / 3;
        } else if (down && keys[i].logical == held && now >= repeat &&
                   ((held >= K_LEFT && held <= K_DOWN) || held == K_DEL)) {
            result = held;
            repeat = now + CLOCKS_PER_SEC / 12;
        }
        previous[i] = down;
    }
    return result;
}
int main(void) {
    app_init(&app);
    restore();
    kb_SetMode(MODE_3_CONTINUOUS);
    app.cancel = cancel;
    gfx_Begin();
    colors();
    gfx_SetDrawBuffer();
    gfx_SetTextConfig(gfx_text_clip);
    gfx_SetMonospaceFont(6);
    gfx_SetTextBGColor(C_WHITE);
    gfx_SetTextTransparentColor(C_WHITE);
    gfx_FillScreen(C_WHITE);
    gfx_SetTextFGColor(C_TEXT);
    gfx_PrintStringXY("DesmosCE: loading session...", 10, 108);
    gfx_SwapDraw();
    Draw draw = {NULL, rectangle, line, text, clip};
    app_rebuild(&app);
    while (!app.quit) {
        int key = poll();
        if (key)
            app_key(&app, key);
        if (app.dirty && !app.quit) {
            gfx_BlitScreen();
            app_draw(&app, &draw);
            gfx_SwapDraw();
        }
        delay(8);
    }
    gfx_End();
    bool saved = save();
    os_ClrHome();
    os_SetCursorPos(0, 0);
    os_PutStrFull(saved ? "DesmosCE session saved." : "Save failed; old slot kept.");
    os_NewLine();
    os_PutStrFull("General math / practice only.");
    kb_SetMode(MODE_0_IDLE);
    return 0;
}
#endif
