#include <stdlib.h>
#include <string.h>
#include "stab.h"

struct Stab *stab_new_symbol(const char *name, int lineno)
{
    struct Stab *symbol = (struct Stab *)malloc(sizeof(struct Stab));
    memset(symbol, 0, sizeof(struct Stab));
    symbol->name = strdup(name);
    symbol->lineno = lineno;
    return symbol;
}

