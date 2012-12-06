#include <stdlib.h>
#include <string.h>
#include "stab.h"

int type_top = 0;
int symbol_top = 0;

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
    struct Arysize_entry *as = (struct Arysize_entry *)malloc(sizeof(struct Arysize_entry));
    as->size = size;
    as->next = NULL;
    return as;
}

