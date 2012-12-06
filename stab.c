#include <stdlib.h>
#include <string.h>
#include "stab.h"

int type_top = 0;
int sym_top = 0;

struct Stab *stab_new_symbol(const char *name, int lineno)
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

