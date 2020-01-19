#pragma once

#include <stddef.h>
#include <stdint.h>


extern char* itoa_s(int value, char* str, size_t size, int radix);
extern char* strcpy_s(char* dest, size_t size, const char* src);

extern char* pad_right(char* str, char pad, uint32_t len);
