#pragma once

#include <stddef.h>
#include <stdint.h>


extern char* itoa_s(int value, char* str, size_t size, int radix);
extern size_t strlen(const char* str);

extern char* pad_right(char* str, char pad, uint32_t len);
