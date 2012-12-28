#ifndef PARSER_STAB_H
#define PARSER_STAB_H

#include <stdlib.h>
#include <string.h>
#include "global.h"
#include "env.h"

enum Type_kind { T_CHAR, T_INT, T_FLOAT, T_DOUBLE, T_VOID, T_STR, T_STRUCT };

struct Stab;

struct Arysize_entry {
    size_t size;
    struct Arysize_entry *next;
};

struct Param_entry {
    struct Stab *symbol;
    struct Param_entry *next;
};

struct Type_info {
    enum Type_kind kind;
    struct Stab *struct_sym;
};

struct Stab {
    char *name;
    int addr;			// given during translation
    int lineno; 		// declaration line
    struct Type_info *type;
    BOOL isfunc;
    int ptrcount;
    union {
        int arysize_cnt;
        int param_cnt;
        int member_cnt;
    };
    union {
        struct Arysize_entry *arysize_list;
        struct Param_entry *param_list;
        struct Env *member_env;
    };
};

extern int type_top;
struct Type_info *type_stack[MAX_STACK_SIZE];

extern int sym_top;
struct Stab *sym_stack[MAX_STACK_SIZE];

struct Stab *stab_new(const char *, int);
struct Arysize_entry *arysize_new(size_t);
struct Param_entry *param_new(struct Stab *);
struct Type_info *type_new(enum Type_kind, struct Stab *);

#endif /* !PARSER_STAB_H */

