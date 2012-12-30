#ifdef NGDEBUG
#include <assert.h>
#endif

#include <stdlib.h>
#include <string.h>
#include "stab.h"
#include "env.h"

int type_top = 0;
int sym_top = 0;
int str_cnt = 0;

struct Stab *stab_new(const char *name, int lineno)
{
    struct Stab *symbol = (struct Stab *)malloc(sizeof(struct Stab));
    memset(symbol, 0, sizeof(struct Stab));
    symbol->name = strdup(name);
    symbol->lineno = lineno;
    return symbol;
}

struct Arysize_entry *arysize_new(size_t size)
{
    struct Arysize_entry *ae = (struct Arysize_entry *)malloc(sizeof(struct Arysize_entry));
    ae->size = size;
    ae->next = NULL;
    return ae;
}

struct Param_entry *param_new(struct Stab *symbol)
{
    struct Param_entry *pe = (struct Param_entry *)malloc(sizeof(struct Param_entry));
    pe->symbol = symbol;
    pe->next = NULL;
    return pe;
}

struct Type_info *type_new(enum Type_kind kind, struct Stab *struct_sym)
{
    struct Type_info *ti = (struct Type_info *)malloc(sizeof(struct Type_info));
    ti->kind = kind;
    ti->struct_sym = struct_sym;
    return ti;
}

size_t type_size(struct Type_info *ti)
{
    switch (ti->kind) {
        case T_CHAR: return CHAR_SIZE;
        case T_STR: return PTR_SIZE;
        case T_INT: return INT_SIZE;
        case T_FLOAT: return FLOAT_SIZE;
        case T_DOUBLE: return DOUBLE_SIZE;
        case T_STRUCT: return ti->struct_sym->size;
        default: return 0;
    }
    return 0;
}

