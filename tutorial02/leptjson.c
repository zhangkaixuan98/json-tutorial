#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char* json;
}lept_context;

typedef enum {true = 1,false = 0} bool;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, const char* literal) {
    EXPECT(c, *literal);
    const char *p = c->json;
    while (*++literal != '\0') {
        if (*p++ != *literal) 
            return LEPT_PARSE_INVALID_VALUE;
    }
    c->json = p;
    return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    int ret = lept_parse_literal(c, "true");
    if (ret == LEPT_PARSE_OK)
        v->type = LEPT_TRUE;
    return ret;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    int ret = lept_parse_literal(c, "false");
    if (ret == LEPT_PARSE_OK)
        v->type = LEPT_FALSE;
    return ret;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    int ret = lept_parse_literal(c, "null");
    if (ret == LEPT_PARSE_OK)
        v->type = LEPT_NULL;
    return ret;
}

static void number_loop(const char* p) {
    while (*p >= '0' && *p <= '9') ++p;
}

static int json_validate_number(lept_context* c) {
    const char* p = c->json;
    if (*p == '-') ++p;
    if (*p == '0') ++p;
    else {
        if (*p >= '1' && *p <= '9') {
            number_loop(++p);
        }
        else return LEPT_PARSE_INVALID_VALUE;
    }
    if (*p == '.') {
        ++p;
        if (*p < '0' || *p > '9' )
            return LEPT_PARSE_INVALID_VALUE;
        number_loop(++p);
    }
    if (*p == 'e' || *p == 'E') {
        ++p;
        if (*p == '+' || *p == '-') ++p;
        if (*p < '0' || *p > '9' )
            return LEPT_PARSE_INVALID_VALUE;
        number_loop(++p);
    }
    return LEPT_PARSE_OK;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* \TODO validate number */
    int ret = json_validate_number(c);
    if (ret != LEPT_PARSE_OK) return ret;
    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_true(c, v);
        case 'f':  return lept_parse_false(c, v);
        case 'n':  return lept_parse_null(c, v);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
