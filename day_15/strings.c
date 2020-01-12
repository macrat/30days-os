#include "math.h"
#include "strings.h"


char* itoa_s(int value, char* str, size_t size, int radix) {
    char* buf = str;

    if (value < 0 && size > 0) {
        value = -value;
        *buf = '-';
        buf++;
        size--;
    }

    size_t len = 0;

    for (int x = value; x > 0; x /= radix) {
        len++;
    }

    if (value == 0 && size > 0) {
        *buf = '0';
        len = 1;
    }

    for (int i = max(0, (int)(len - size - 1)); i < (int)len; value /= radix, i++) {
        const int cur = value % radix;

        buf[len - i - 1] = cur < 10 ? ('0' + cur) : ('A' + cur - 10);
    }

    if (size > 0) {
        buf[len] = '\0';
    }

    return str;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[++len]);
    return len;
}

char* strcpy_s(char* dest, size_t size, const char* src) {
    unsigned int i;
    for (i = 0; i < size && src[i] != 0; i++) {
        dest[i] = src[i];
    }
    dest[i] = 0x00;
    return dest;
}

char* pad_right(char* str, char pad, uint32_t len) {
    const size_t origlen = strlen(str);

    if (origlen >= len) return str;

    const uint32_t shift = len - origlen;

    for (unsigned int i = len; i >= shift; i--) {
        str[i] = str[i - shift];
    }

    for (unsigned int i=0; i < shift; i++) {
        str[i] = pad;
    }

    return str;
}
