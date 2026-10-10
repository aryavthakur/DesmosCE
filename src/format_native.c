/* Integer/string-only formatter. Avoids linking a general-purpose printf engine.
 * The application formats binary64 values separately in dce_format(). */
#ifdef __TICE__
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
int snprintf(char *out, size_t cap, const char *fmt, ...) {
    va_list args;
    size_t used = 0;
    va_start(args, fmt);
    while (*fmt) {
        char digits[24], *s = digits;
        if (*fmt != '%') {
            digits[0] = *fmt++;
            digits[1] = 0;
        } else {
            ++fmt;
            int wide = *fmt == 'l';
            if (wide)
                ++fmt;
            char spec = *fmt ? *fmt++ : 0;
            if (spec == 's')
                s = va_arg(args, char *);
            else if (spec == 'u' || spec == 'd') {
                unsigned long value;
                int negative = 0;
                if (spec == 'd') {
                    long v = wide ? va_arg(args, long) : va_arg(args, int);
                    negative = v < 0;
                    value = negative ? 0UL - (unsigned long)v : (unsigned long)v;
                } else
                    value = wide ? va_arg(args, unsigned long) : va_arg(args, unsigned);
                s = digits + sizeof(digits) - 1;
                *s = 0;
                do {
                    *--s = '0' + value % 10;
                    value /= 10;
                } while (value);
                if (negative)
                    *--s = '-';
            } else {
                digits[0] = spec ? spec : '%';
                digits[1] = 0;
            }
        }
        if (!s)
            s = "(null)";
        while (*s) {
            if (used + 1 < cap)
                out[used] = *s;
            ++used;
            ++s;
        }
    }
    va_end(args);
    if (cap)
        out[used < cap ? used : cap - 1] = 0;
    return (int)used;
}
#endif
