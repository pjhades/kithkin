#include <kernel/console.h>
#include <stdarg.h>

#define isdigit(x) ((x) >= '0' && (x) <= '9')

#define printarg()                                  \
    do {                                            \
        if (negative == 1)                          \
            str[bytes++] = '-';                     \
        else if (negative == 2) {                   \
            str[bytes++] = '0'; str[bytes++] = 'x'; \
        }                                           \
        for (; pad_precision > 0; pad_precision--)  \
            str[bytes++] = '0';                     \
        for (--digits; digits >= 0; digits--)       \
            str[bytes++] = temp[digits];            \
    } while (0)

#define printpad()                                          \
    do {                                                    \
        for (; pad_width > 0; pad_width--)                  \
            str[bytes++] = width_prefix == '0' ? '0' : ' '; \
    } while (0)


static int tonum(char *buf, unsigned int num, int base)
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
    int digits, bytes, width, precision, argint,
        pad_precision, pad_width, len;
    unsigned int arguint;
    char *argstr, width_prefix, negative, temp[1024];
    const char *p;

    for (bytes = 0; *fmt; ) {
        if (*fmt != '%') {
            str[bytes++] = *fmt;
            ++fmt;
            continue;
        }

        width = precision = negative = 0;
        width_prefix = '+';
        p = fmt + 1;
        if (!*p)
            goto end;
        if (*p == '%') {
            str[bytes++] = '%';
            continue;
        }
        /* width */
        if (*p == '+' || *p == '-' || *p == '0') {
            width_prefix = *p;
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
                argint = va_arg(va, int);
                if (argint < 0) {
                    negative = 1;
                    argint = -argint;
                }
                digits = tonum(temp, (unsigned int)argint, 10);
                goto set_length;
            case 'p':
                negative = 2;
            case 'x':
                arguint = va_arg(va, unsigned int);
                digits = tonum(temp, arguint, 16);
set_length:
                pad_precision = precision > digits ? precision - digits : 0;
                len = (precision > digits ? precision : digits) + negative;
                break;
            case 's':
                argstr = va_arg(va, char *);
                digits = tostr(temp, argstr, precision);
                pad_precision = 0;
                len = digits;
                break;
            default:
                fmt = p + 1;
                continue;
        }

        pad_width = width > len ? width - len : 0;
        if (width_prefix == '-') {
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
