#include "dce.h"
#include <math.h>
#include <string.h>
/* Versioned little-endian binary64 wire format, independent of C padding/int size. */
_Static_assert(sizeof(Real) == 8, "Session codec requires IEEE-754 binary64 Real");
typedef struct {
    uint8_t *data;
    size_t pos, cap;
    bool ok;
} Writer;
static void bytes(Writer *w, const void *p, size_t n) {
    if (n > w->cap - w->pos) {
        w->ok = false;
        return;
    }
    memcpy(w->data + w->pos, p, n);
    w->pos += n;
}
static void u32(Writer *w, uint32_t x) {
    uint8_t b[4];
    for (unsigned i = 0; i < 4; i++) {
        b[i] = x & 255;
        x >>= 8;
    }
    bytes(w, b, 4);
}
static void real(Writer *w, Real x) {
    const uint16_t one = 1;
    if (*(const uint8_t *)&one)
        bytes(w, &x, 8);
    else {
        uint8_t b[8];
        for (unsigned i = 0; i < 8; i++)
            b[i] = ((const uint8_t *)&x)[7 - i];
        bytes(w, b, 8);
    }
}
static uint32_t get32(const uint8_t *p) {
    uint32_t r = 0;
    for (int i = 3; i >= 0; i--)
        r = (r << 8) | p[i];
    return r;
}
static uint32_t crc(const uint8_t *p, size_t n) {
    uint32_t r = 0xffffffffu;
    for (size_t i = 0; i < n; i++) {
        r ^= i >= 12 && i < 16 ? 0 : p[i];
        for (unsigned i = 0; i < 8; i++)
            r = (r >> 1) ^ ((r & 1) ? 0xedb88320u : 0);
    }
    return ~r;
}
size_t dce_pack(const Session *s, uint8_t *out, size_t cap) {
    if (!dce_window_valid(s) || cap < 20 || s->rows < 1 || s->rows > DCE_ROWS ||
        s->points > DCE_LIST || s->sliders > DCE_PARAMS)
        return 0;
    Writer w = {out, 0, cap, true};
    bytes(&w, "DCE1", 4);
    u32(&w, 0);
    u32(&w, s->generation);
    u32(&w, 0);
    uint8_t flags[5] = {s->rows, s->points, s->sliders, s->degrees, s->grid};
    bytes(&w, flags, 5);
    real(&w, s->xmin);
    real(&w, s->xmax);
    real(&w, s->ymin);
    real(&w, s->ymax);
    for (unsigned i = 0; i < s->rows; i++) {
        if (!memchr(s->row[i].text, 0, DCE_TEXT))
            return 0;
        bytes(&w, s->row[i].text, DCE_TEXT);
        bytes(&w, &s->row[i].visible, 1);
        bytes(&w, &s->row[i].color, 1);
    }
    for (unsigned i = 0; i < s->points; i++) {
        real(&w, s->x[i]);
        real(&w, s->y[i]);
    }
    for (unsigned i = 0; i < s->sliders; i++) {
        bytes(&w, s->slider[i].name, DCE_NAME);
        real(&w, s->slider[i].value);
        real(&w, s->slider[i].min);
        real(&w, s->slider[i].max);
        real(&w, s->slider[i].step);
    }
    if (!w.ok)
        return 0;
    size_t size = w.pos;
    w.pos = 4;
    u32(&w, (uint32_t)size);
    w.pos = 12;
    u32(&w, crc(out, size));
    return size;
}
static bool readreal(const uint8_t *data, size_t size, size_t *pos, Real *x) {
    if (*pos + 8 > size)
        return false;
    const uint16_t one = 1;
    if (*(const uint8_t *)&one)
        memcpy(x, data + *pos, 8);
    else
        for (unsigned i = 0; i < 8; i++)
            ((uint8_t *)x)[i] = data[*pos + 7 - i];
    *pos += 8;
    return isfinite(*x);
}
bool dce_unpack(Session *s, const uint8_t *data, size_t size) {
    if (size < 53 || size > 4096 || memcmp(data, "DCE1", 4) || get32(data + 4) != size ||
        get32(data + 12) != crc(data, size))
        return false;
    static Session t;
    memset(&t, 0, sizeof(t));
    t.generation = get32(data + 8);
    t.rows = data[16];
    t.points = data[17];
    t.sliders = data[18];
    t.degrees = data[19];
    t.grid = data[20];
    if (!t.rows || t.rows > DCE_ROWS || t.points > DCE_LIST || t.sliders > DCE_PARAMS ||
        t.degrees > 1 || t.grid > 1)
        return false;
    size_t pos = 21;
#define R(x)                                                                                       \
    do {                                                                                           \
        if (!readreal(data, size, &pos, &(x)))                                                     \
            return false;                                                                          \
    } while (0)
    R(t.xmin);
    R(t.xmax);
    R(t.ymin);
    R(t.ymax);
    if (!dce_window_valid(&t))
        return false;
    for (unsigned i = 0; i < t.rows; i++) {
        if (pos + DCE_TEXT + 2 > size)
            return false;
        memcpy(t.row[i].text, data + pos, DCE_TEXT);
        pos += DCE_TEXT;
        if (!memchr(t.row[i].text, 0, DCE_TEXT))
            return false;
        t.row[i].visible = data[pos++];
        t.row[i].color = data[pos++];
        if (t.row[i].visible > 1 || t.row[i].color > 5)
            return false;
    }
    for (unsigned i = 0; i < t.points; i++) {
        R(t.x[i]);
        R(t.y[i]);
    }
    for (unsigned i = 0; i < t.sliders; i++) {
        if (pos + DCE_NAME > size)
            return false;
        memcpy(t.slider[i].name, data + pos, DCE_NAME);
        pos += DCE_NAME;
        if (!memchr(t.slider[i].name, 0, DCE_NAME) || !t.slider[i].name[0])
            return false;
        R(t.slider[i].value);
        R(t.slider[i].min);
        R(t.slider[i].max);
        R(t.slider[i].step);
        if (t.slider[i].min >= t.slider[i].max || t.slider[i].step <= 0 ||
            t.slider[i].value < t.slider[i].min || t.slider[i].value > t.slider[i].max)
            return false;
    }
#undef R
    if (pos != size)
        return false;
    *s = t;
    return true;
}
