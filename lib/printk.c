#include <stdarg.h>
#include <kernel/console.h>
#include <kernel/types.h>

#define isdigit(x) ((x) >= '0' && (x) <= '9')

#define printarg()                                  \
    do {                                            \
        if (sign == 1)                              \
            str[bytes++] = '-';                     \
        else if (sign == 2) {                       \
            str[bytes++] = '0'; str[bytes++] = 'x'; \
        }                                           \
        for (; precisionpad > 0; precisionpad--)    \
            str[bytes++] = '0';                     \
        for (--digits; digits >= 0; digits--)       \
            str[bytes++] = temp[digits];            \
    } while (0)

#define printpad()                                         \
    do {                                                   \
        for (; widthpad > 0; widthpad--)                   \
            str[bytes++] = widthprefix == '0' ? '0' : ' '; \
    } while (0)


static int tonum(char *buf, uint64_t num, int base)
{
    int digits = 0;
    char *hex = "0123456789abcdef";

    if (num == 0) {
        buf[0] = '0';
        return 1;
    }
    while (num > 0) {
        buf[digits++] = hex[num % base];
        num /= base;
    }
    return digits;
}

static int tostr(char *buf, char *argstr, int precision)
{
    int len, i;
    char *p;

    for (len = 0, p = argstr; *p && (precision == 0 || len < precision); p++)
        len++;
    for (i = len; i > 0; i--)
        buf[len - i] = argstr[i - 1];
    return len;
}

int vsprintk(char *str, const char *fmt, va_list va)
{
    int len, digits, bytes, width, precision, i32,
        precisionpad = 0, widthpad = 0;
    unsigned int u32;
    int64_t i64;
    uint64_t u64;
    char *argstr, widthprefix, sign, temp[1024];
    const char *p;

    for (bytes = 0; *fmt; ) {
        if (*fmt != '%') {
            str[bytes++] = *fmt;
            ++fmt;
            continue;
        }

        width = precision = sign = 0;
        widthprefix = '+';
        p = fmt + 1;
        if (!*p)
            goto end;
        if (*p == '%') {
            str[bytes++] = '%';
            continue;
        }

        /* width */
        if (*p == '+' || *p == '-' || *p == '0') {
            widthprefix = *p;
            ++p;
        }
        for (; *p && *p != '.' && isdigit(*p); p++)
            width = width * 10 + (*p - '0');
        if (!*p)
            goto end;

        /* precision */
        if (*p == '.') {
            ++p;
            for (; *p && isdigit(*p); p++)
                precision = precision * 10 + (*p - '0');
            if (!*p)
                goto end;
        }

        /* conversion */
        switch (*p) {
            case 'd':
                i32 = va_arg(va, int);
                if (i32 < 0) {
                    sign = 1;
                    i32 = -i32;
                }
                digits = tonum(temp, i32, 10);
                goto setlength;

            case 'u':
                u32 = va_arg(va, unsigned int);
                digits = tonum(temp, u32, 10);
                goto setlength;

            case 'q':
                i64 = va_arg(va, int64_t);
                if (i64 < 0) {
                    sign = 1;
                    i64 = -i64;
                }
                digits = tonum(temp, i64, 10);
                goto setlength;

            case 'U':
                u64 = va_arg(va, uint64_t);
                digits = tonum(temp, u64, 10);
                goto setlength;

            case 'P':
                sign = 2;
            case 'X':
                u64 = va_arg(va, uint64_t);
                digits = tonum(temp, u64, 16);
                goto setlength;

            case 'p':
                sign = 2;
            case 'x':
                u32 = va_arg(va, unsigned int);
                digits = tonum(temp, u32, 16);
setlength:
                if (precision > digits)
                    precisionpad = precision - digits;
                len = (precision > digits ? precision : digits) + sign;
                break;

            case 's':
                argstr = va_arg(va, char *);
                digits = tostr(temp, argstr, precision);
                precisionpad = 0;
                len = digits;
                break;

            default:
                fmt = p + 1;
                continue;
        }

        if (width > len)
            widthpad = width - len;
        if (widthprefix == '-') {
            printarg();
            printpad();
        }
        else {
            printpad();
            printarg();
        }
        str[bytes] = '\0';

        fmt = p + 1;
    }

end:
    str[bytes] = '\0';
    return bytes;
}

int printk(const char *fmt, ...)
{
    char temp[1024];
    int count;
    va_list va;

    va_start(va, fmt);
    count = vsprintk(temp, fmt, va);
    va_end(va);
    cputs(temp);
    return count;
}
