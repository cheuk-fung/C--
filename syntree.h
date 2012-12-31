#ifndef CMM_SYNTREE_H
#define CMM_SYNTREE_H

#include <stdio.h>
#include "parser.h"
#include "env.h"
#include "stab.h"

enum Node_kind { K_FUNC, K_STRUCT, K_DEF, K_STMT, K_EXPR };
enum Stmt_kind { K_IF, K_IFELSE, K_WHILE, K_DO, K_FOR, K_SWITCH, K_GOTO, K_RET };
enum Expr_kind { K_CHAR, K_STR, K_INT, K_DOUBLE, K_SYM, K_ARY, K_DOT, K_PTR, K_OPR, K_CALL };

union SE_kind {
    void *nothing;
    enum Stmt_kind stmt;
    enum Expr_kind expr;
};

union Information {
    void *nothing;
    enum yytokentype token;
    char c;
    int strno;
    int val;
    int dblno;
    struct Stab *symbol;
};

struct Syntree_node {
    int child_count;
    struct Syntree_node **child;
    struct Syntree_node *next;
    struct Syntree_node *tail;
    struct Env *env;
    int lineno;
    enum Node_kind nkind;
    struct Type_info ntype;
    union SE_kind se;
    union Information info;
    size_t tmppos;

    int nodeid;
};

extern FILE *fmsg;

struct Syntree_node *syntree_new_node(int, enum Node_kind, enum Type_kind, void *, void *,
        struct Syntree_node *, struct Syntree_node *, struct Syntree_node *, struct Syntree_node *);
struct Syntree_node *syntree_insert_last(struct Syntree_node *, struct Syntree_node *);
int syntree_translate(struct Syntree_node *);

#endif /* !CMM_SYNTREE_H */


