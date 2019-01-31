#include <uchar.h>
#include <stddef.h>
#include <stdio.h>
#ifndef utf_convert_h
#define utf_convert_h
size_t utf8_to_utf32(unsigned char* inputString, size_t numberOfChars, char32_t* outputString);
size_t utf16_to_utf32(char16_t* inputString, size_t numberOfChar16s, char32_t* outputString);  //Make sure the supplied output buffer is able to hold at least TODO characters
size_t utf32_cut_ASCII(char32_t* inputString, size_t numberOfChar32s, unsigned char* outputString);
#endif
