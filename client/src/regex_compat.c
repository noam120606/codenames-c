#include "../lib/all.h"

#ifdef _WIN32

typedef enum {
    TOK_LITERAL,
    TOK_DOT,
    TOK_CLASS
} CnRegexTokenType;

typedef struct {
    CnRegexTokenType type;
    unsigned char literal;
    unsigned char class_map[256];
    int min_rep;
    int max_rep; /* -1 = infini */
} CnRegexToken;

static int is_digit_char(char c) {
    return c >= '0' && c <= '9';
}

static int parse_int(const char** p, int* out) {
    int v = 0;
    int seen = 0;
    while (**p && is_digit_char(**p)) {
        v = v * 10 + (**p - '0');
        (*p)++;
        seen = 1;
    }
    if (!seen) return 0;
    *out = v;
    return 1;
}

static int token_matches(const CnRegexToken* tok, unsigned char ch) {
    if (tok->type == TOK_DOT) return 1;
    if (tok->type == TOK_LITERAL) return ch == tok->literal;
    return tok->class_map[ch] != 0;
}

static int match_from(const CnRegexToken* toks, int tok_count, const char* text, int text_len, int tok_idx, int pos, int anchored_end) {
    if (tok_idx >= tok_count) {
        return anchored_end ? (pos == text_len) : 1;
    }

    const CnRegexToken* tok = &toks[tok_idx];
    int max_possible = text_len - pos;
    int max_rep = tok->max_rep < 0 ? max_possible : tok->max_rep;
    if (max_rep > max_possible) max_rep = max_possible;

    for (int rep = max_rep; rep >= tok->min_rep; rep--) {
        int ok = 1;
        for (int i = 0; i < rep; i++) {
            if (!token_matches(tok, (unsigned char)text[pos + i])) {
                ok = 0;
                break;
            }
        }
        if (!ok) continue;

        if (match_from(toks, tok_count, text, text_len, tok_idx + 1, pos + rep, anchored_end)) {
            return 1;
        }
    }

    return 0;
}

int cn_regex_compile(CnRegex* re, const char* pattern) {
    if (!re || !pattern) return -1;

    memset(re, 0, sizeof(*re));
    re->pattern = strdup(pattern);
    if (!re->pattern) return -1;

    const char* p = re->pattern;
    if (*p == '^') {
        re->anchored_start = 1;
        p++;
    }

    size_t cap = 8;
    CnRegexToken* toks = (CnRegexToken*)calloc(cap, sizeof(CnRegexToken));
    if (!toks) {
        free(re->pattern);
        re->pattern = NULL;
        return -1;
    }

    int count = 0;

    while (*p) {
        if (*p == '$' && p[1] == '\0') {
            re->anchored_end = 1;
            p++;
            break;
        }

        CnRegexToken tok;
        memset(&tok, 0, sizeof(tok));
        tok.min_rep = 1;
        tok.max_rep = 1;

        if (*p == '.') {
            tok.type = TOK_DOT;
            p++;
        } else if (*p == '[') {
            tok.type = TOK_CLASS;
            p++;
            int negate = 0;
            if (*p == '^') {
                negate = 1;
                p++;
            }

            while (*p && *p != ']') {
                unsigned char start = (unsigned char)*p;
                if (p[1] == '-' && p[2] && p[2] != ']') {
                    unsigned char end = (unsigned char)p[2];
                    if (start <= end) {
                        for (unsigned int c = start; c <= end; c++) tok.class_map[c] = 1;
                    } else {
                        for (unsigned int c = end; c <= start; c++) tok.class_map[c] = 1;
                    }
                    p += 3;
                } else {
                    tok.class_map[start] = 1;
                    p++;
                }
            }
            if (*p != ']') {
                free(toks);
                free(re->pattern);
                re->pattern = NULL;
                return -1;
            }
            p++;

            if (negate) {
                for (int i = 0; i < 256; i++) tok.class_map[i] = tok.class_map[i] ? 0 : 1;
            }
        } else if (*p == '\\') {
            p++;
            if (!*p) {
                free(toks);
                free(re->pattern);
                re->pattern = NULL;
                return -1;
            }
            tok.type = TOK_LITERAL;
            tok.literal = (unsigned char)*p;
            p++;
        } else {
            tok.type = TOK_LITERAL;
            tok.literal = (unsigned char)*p;
            p++;
        }

        if (*p == '*') {
            tok.min_rep = 0;
            tok.max_rep = -1;
            p++;
        } else if (*p == '+') {
            tok.min_rep = 1;
            tok.max_rep = -1;
            p++;
        } else if (*p == '?') {
            tok.min_rep = 0;
            tok.max_rep = 1;
            p++;
        } else if (*p == '{') {
            p++;
            int m = 0;
            int n = 0;
            if (!parse_int(&p, &m)) {
                free(toks);
                free(re->pattern);
                re->pattern = NULL;
                return -1;
            }
            if (*p == '}') {
                tok.min_rep = m;
                tok.max_rep = m;
                p++;
            } else if (*p == ',') {
                p++;
                if (*p == '}') {
                    tok.min_rep = m;
                    tok.max_rep = -1;
                    p++;
                } else {
                    if (!parse_int(&p, &n) || *p != '}') {
                        free(toks);
                        free(re->pattern);
                        re->pattern = NULL;
                        return -1;
                    }
                    if (n < m) {
                        free(toks);
                        free(re->pattern);
                        re->pattern = NULL;
                        return -1;
                    }
                    tok.min_rep = m;
                    tok.max_rep = n;
                    p++;
                }
            } else {
                free(toks);
                free(re->pattern);
                re->pattern = NULL;
                return -1;
            }
        }

        if ((size_t)count >= cap) {
            cap *= 2;
            CnRegexToken* grown = (CnRegexToken*)realloc(toks, cap * sizeof(CnRegexToken));
            if (!grown) {
                free(toks);
                free(re->pattern);
                re->pattern = NULL;
                return -1;
            }
            toks = grown;
        }

        toks[count++] = tok;
    }

    re->tokens = toks;
    re->token_count = count;
    return 0;
}

int cn_regex_match(const CnRegex* re, const char* text) {
    if (!re || !text || !re->tokens) return -1;

    const CnRegexToken* toks = (const CnRegexToken*)re->tokens;
    int len = (int)strlen(text);

    if (re->anchored_start) {
        return match_from(toks, re->token_count, text, len, 0, 0, re->anchored_end) ? 0 : 1;
    }

    for (int start = 0; start <= len; start++) {
        if (match_from(toks, re->token_count, text, len, 0, start, re->anchored_end)) {
            return 0;
        }
    }

    return 1;
}

void cn_regex_free(CnRegex* re) {
    if (!re) return;
    if (re->tokens) {
        free(re->tokens);
        re->tokens = NULL;
    }
    if (re->pattern) {
        free(re->pattern);
        re->pattern = NULL;
    }
    re->token_count = 0;
    re->anchored_start = 0;
    re->anchored_end = 0;
}

#else

int cn_regex_compile(CnRegex* re, const char* pattern) {
    if (!re || !pattern) return -1;
    return regcomp(&re->impl, pattern, REG_EXTENDED | REG_NOSUB);
}

int cn_regex_match(const CnRegex* re, const char* text) {
    if (!re || !text) return -1;
    return regexec(&re->impl, text, 0, NULL, 0);
}

void cn_regex_free(CnRegex* re) {
    if (!re) return;
    regfree(&re->impl);
}

#endif
