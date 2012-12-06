#include <stdlib.h>
#include <string.h>
#include "stab.h"

int type_top = 0;
int id_top = 0;

struct Stab *stab_new_symbol(const char *name, int lineno)
{
    struct Stab *symbol = (struct Stab *)malloc(sizeof(struct Stab));
    memset(symbol, 0, sizeof(struct Stab));
    symbol->name = strdup(name);
    symbol->lineno = lineno;
    return symbol;
}

static struct Array_size *array_size_new(size_t size)
{
    struct Array_size *as = (struct Array_size *)malloc(sizeof(struct Array_size));
    as->size = size;
    as->next = NULL;
    return as;
}

void array_size_insert(struct Stab *symbol, size_t size)
{
    if (symbol->arysize == NULL) {
        symbol->arysize = array_size_new(size);
    } else {
        struct Array_size *ptr = symbol->arysize;
        while (ptr->next != NULL) ptr = ptr->next;
        ptr->next = array_size_new(size);
    }
    symbol->arycount++;
}

