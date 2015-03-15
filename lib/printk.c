#include <kernel/console.h>
#include <stdarg.h>

int vsprintk(char *str, const char *fmt, va_list va)
{
    unsigned int arg_u;
    int i, j, width, precision, count, arg_i;
    char prefix, conversion, *arg_s, *hex = "0123456789abcdef",
         conv[26] = {
             ['d' - 'a'] = 1,
             ['p' - 'a'] = 1,
             ['s' - 'a'] = 1,
             ['x' - 'a'] = 1,
         };
    enum {
        s_normal,
        s_start,
        s_width_prefix,
        s_width_digit,
        s_dot,
        s_precision,
        s_print,
    } state;

    for (i = 0, j = 0, state = s_normal, count = 0; fmt[i] != '\0'; ) {
        if (state == s_normal) {
            if (fmt[i] != '%') {
                str[j++] = fmt[i];
                count++;
            }
            else {
                state = s_start;
                width = 0;
                precision = 0;
                prefix = '+';
            }
            i++;
            continue;
        }

        switch (state) {
            case s_start:
                if (fmt[i] == '+' || fmt[i] == '-' || fmt[i] == '0') {
                    prefix = fmt[i];
                    state = s_width_prefix;
                }
                else if (fmt[i] >= '1' && fmt[i] <= '9') {
                    width = width * 10 + (fmt[i] - '0');
                    state = s_width_digit;
                }
                else if (fmt[i] == '.')
                    state = s_dot;
                else if (fmt[i] == '%') {
                    conversion = '%';
                    state = s_print;
                }
                else if (conv[fmt[i] - 'a']) {
                    conversion = fmt[i];
                    state = s_print;
                }
                else
                    goto error;
                break;

            case s_width_prefix:
                if (fmt[i] >= '1' && fmt[i] <= '9') {
                    width = width * 10 + (fmt[i] - '0');
                    state = s_width_digit;
                }
                else
                    goto error;
                break;

            case s_width_digit:
                if (fmt[i] >= '0' && fmt[i] <= '9') {
                    width = width * 10 + (fmt[i] - '0');
                    state = s_width_digit;
                }
                else if (fmt[i] == '.')
                    state = s_dot;
                else if (conv[fmt[i] - 'a']) {
                    conversion = fmt[i];
                    state = s_print;
                }
                else
                    goto error;
                break;

            case s_dot:
                if (fmt[i] >= '0' && fmt[i] <= '9') {
                    precision = precision * 10 + (fmt[i] - '0');
                    state = s_precision;
                }
                else if (conv[fmt[i] - 'a']) {
                    conversion = fmt[i];
                    state = s_print;
                }
                else
                    goto error;
                break;

            case s_precision:
                if (fmt[i] >= '0' && fmt[i] <= '9') {
                    precision = precision * 10 + (fmt[i] - '0');
                    state = s_precision;
                }
                else if (conv[fmt[i] - 'a']) {
                    conversion = fmt[i];
                    state = s_print;
                }
                else
                    goto error;
                break;
        }

        if (state == s_print) {
            char tmp[32];
            int k = 0, idx, pad_precision = 0, pad_width = 0, len, negative = 0;

            if (conversion == 'p') {
                arg_u = va_arg(va, unsigned int);
                str[j++] = '0';
                str[j++] = 'x';
                count += 2;
                if (arg_u == 0) {
                    str[j++] = '0';
                    count++;
                }
                else {
                    for (k = 0; arg_u > 0; ) {
                        tmp[k++] = hex[arg_u % 0x10];
                        arg_u /= 0x10;
                    }
                    for (--k; k >= 0; k--) {
                        str[j++] = tmp[k];
                        count++;
                    }
                }
            }
            else if (conversion == 'x' || conversion == 'd') {
                if (conversion == 'x') {
                    arg_u = va_arg(va, unsigned int);
                    if (arg_u == 0)
                        tmp[k++] = '0';
                    else {
                        for (k = 0; arg_u > 0; ) {
                            tmp[k++] = hex[arg_u % 0x10];
                            arg_u /= 0x10;
                        }
                    }
                } else {
                    arg_i = va_arg(va, int);
                    if (arg_i < 0) {
                        negative = 1;
                        arg_i = -arg_i;
                    }
                    if (arg_i == 0) {
                        tmp[k++] = '0';
                        len = k;
                    }
                    else {
                        for (k = 0; arg_i > 0; ) {
                            tmp[k++] = hex[arg_i % 10];
                            arg_i /= 10;
                        }
                    }
                }

                pad_precision = precision > k ? precision - k : 0;
                len = pad_precision > 0 ? precision : k;
                len = negative ? len + 1 : len;
                pad_width = width > len ? width - len : 0;

                if (prefix == '-') {
                    if (negative) {
                        str[j++] = '-';
                        count++;
                    }
                    for (idx = 0; idx < pad_precision; idx++) {
                        str[j++] = '0';
                        count++;
                    }
                    for (--k; k >= 0; k--) {
                        str[j++] = tmp[k];
                        count++;
                    }
                    for (idx = 0; idx < pad_width; idx++) {
                        str[j++] = ' ';
                        count++;
                    }
                } else {
                    for (idx = 0; idx < pad_width; idx++) {
                        str[j++] = prefix == '+' || negative ? ' ' : '0';
                        count++;
                    }
                    if (negative) {
                        str[j++] = '-';
                        count++;
                    }
                    for (idx = 0; idx < pad_precision; idx++) {
                        str[j++] = '0';
                        count++;
                    }
                    for (--k; k >= 0; k--) {
                        str[j++] = tmp[k];
                        count++;
                    }
                }
            }
            else if (conversion == '%') {
                str[j++] = '%';
                count++;
            }
            else {
                arg_s = va_arg(va, char *);
                for (k = 0; *arg_s && (precision == 0 || k < precision); )
                    tmp[k++] = *arg_s++;
                pad_width = width > k ? width - k : 0;
                if (prefix == '-') {
                    for (idx = 0; idx < k; idx++) {
                        str[j++] = tmp[idx];
                        count++;
                    }
                    for (idx = 0; idx < pad_width; idx++) {
                        str[j++] = ' ';
                        count++;
                    }
                } else {
                    for (idx = 0; idx < pad_width; idx++) {
                        str[j++] = ' ';
                        count++;
                    }
                    for (idx = 0; idx < k; idx++) {
                        str[j++] = tmp[idx];
                        count++;
                    }
                }
            }

            state = s_normal;
        }

        i++;
    }
    str[j] = '\0';
    return count;

error:
    return -1;
}

int printk(const char *fmt, ...)
{
    char tmp[1024];
    int count;
    va_list va;

    va_start(va, fmt);
    count = vsprintk(tmp, fmt, va);
    va_end(va);
    if (count != -1)
        cons_puts(tmp);
    return count;
}
