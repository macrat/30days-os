#include <stdint.h>
#include <stdlib.h>


void* memset(void* dest, int value, size_t len) {
    for (void* itr=dest; itr < dest + len; itr++) {
        *(int8_t*)itr = (int8_t)value;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t len) {
    for (uint8_t *d=(uint8_t*)dest, *s=(uint8_t*)src; d < (uint8_t*)dest + len; d++, s++) {
        *d = *s;
    }
    return dest;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[++len]);
    return len;
}
