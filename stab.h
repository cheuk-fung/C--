#ifndef PARSER_STAB_H
#define PARSER_STAB_H

#include <stdlib.h>
#include <string.h>
#include "global.h"

enum Type_kind { T_VOID, T_INT, T_CHAR, T_FLOAT, T_DOUBLE, T_STRUCT };

struct Stab;

struct Arysize_entry {
    size_t size;
    struct Arysize_entry *next;
};

struct Param_entry {
    struct Stab *symbol;
    struct Param_entry *next;
};

struct Stab {
    char *name;
    int addr;			// given during translation
    int lineno; 		// declaration line
    enum Type_kind type;
    BOOL isfunc;
    int ptrcount;
    union {
        int arycount;
        int parmacount;
    };
    union {
        struct Arysize_entry *arysize_list;
        struct Param_entry *param_list;
    };
};

extern int type_top;
enum Type_kind type_stack[MAX_STACK_SIZE];

extern int symbol_top;
struct Stab *symbol_stack[MAX_STACK_SIZE];

struct Stab *stab_new_symbol(const char *, int);
struct Arysize_entry *arysize_new(size_t);

#endif /* !PARSER_STAB_H */

