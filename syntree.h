#ifndef PARSER_SYNTREE_H
#define PARSER_SYNTREE_H

#include "parser.h"
#include "env.h"
#include "stab.h"

enum Node_kind { K_FUNC, K_STRUCT, K_DEF, K_STMT, K_EXPR };
enum Stmt_kind { K_IF, K_IFELSE, K_WHILE, K_DO, K_FOR, K_SWITCH, K_GOTO, K_RET };
enum Expr_kind { K_OPR, K_CHAR, K_STR, K_INT, K_FLOAT, K_ID, K_ARY };

struct ID_node {
    struct Stab *id;
    struct ID_node *next;
    struct ID_node *tail;
};

struct Syntree_node {
    int child_count;
    struct Syntree_node **child;
    struct Syntree_node *next;
    struct Syntree_node *tail;
    struct Env *env;
    int lineno;
    enum Node_kind nkind;
    union {
        enum Stmt_kind stmt;
        enum Expr_kind expr;
    };
    union {
        enum yytokentype token;
        char c;
        char *str;
        int val;
        double dval;
        struct Stab *id;
        struct ID_node *idlist;
    };

    int nodeid;
};

struct ID_node *id_new_node(struct Stab *);
struct ID_node *id_insert_last(struct ID_node *, struct ID_node *);
struct Syntree_node *syntree_new_node(int, enum Node_kind);
struct Syntree_node *syntree_insert_last(struct Syntree_node *, struct Syntree_node *);
int syntree_translate(struct Syntree_node *);

#endif /* !PARSER_SYNTREE_H */


