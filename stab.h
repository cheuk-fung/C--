#ifndef PARSER_STAB_H
#define PARSER_STAB_H

#include <stdlib.h>
#include <string.h>
#include "global.h"

enum Type_kind { T_VOID, T_INT, T_CHAR, T_FLOAT, T_DOUBLE, T_STRUCT };

struct Stab {
    char *name;
    int addr;			// given during translation
    int lineno; 		// declaration line
    enum Type_kind type;
    BOOL isfunc;
    int ptrcount;
    int arysize;		/* -1 means no size specified,
                                   other value means the size of the array */
};

extern int type_top;
enum Type_kind type_stack[MAX_STACK_SIZE];

extern int id_top;
struct Stab *id_stack[MAX_STACK_SIZE];

struct Stab *stab_new_symbol(const char *, int);

#endif /* !PARSER_STAB_H */

