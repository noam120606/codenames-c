#ifndef REGEX_COMPAT_H
#define REGEX_COMPAT_H

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32

typedef struct {
    char* pattern;
    int anchored_start;
    int anchored_end;
    void* tokens;
    int token_count;
} CnRegex;

#else

#include <regex.h>

typedef struct {
    regex_t impl;
} CnRegex;

#endif

int cn_regex_compile(CnRegex* re, const char* pattern);
int cn_regex_match(const CnRegex* re, const char* text);
void cn_regex_free(CnRegex* re);

#endif
