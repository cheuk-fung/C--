#ifdef NGDEBUG
#include <assert.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "lexer.h"
#include "syntree.h"

static int nodeid_count = 0;

static void syntree_type_check(struct Syntree_node *node)
{
}

struct Syntree_node *syntree_new_node(int child_count, enum Node_kind nkind, enum Type_kind ntype)
{
    struct Syntree_node *node = (struct Syntree_node *)malloc(sizeof(struct Syntree_node));
    node->child_count = child_count;
    if (child_count) {
        node->child = (struct Syntree_node **)malloc(sizeof(struct Syntree_node) * child_count);
    } else {
        node->child = 0;
    }
    node->next = 0;
    node->tail = node;
    node->env = curr_env;
    node->lineno = yyget_lineno();
    node->nkind = nkind;
    node->ntype = ntype;
    syntree_type_check(node);

    node->nodeid = nodeid_count++;

    return node;
}

struct Syntree_node *syntree_insert_last(struct Syntree_node *dest, struct Syntree_node *src)
{
#ifdef NGDEBUG
    assert(dest->tail->next == NULL);
#endif
    dest->tail->next = src;
    dest->tail = src->tail;
    return dest;
}

static void print_child(struct Syntree_node *node)
{
    if (node->child) {
        fprintf(stderr, "Children:");
        int child_it;
        for (child_it = 0; child_it < node->child_count; child_it++) {
            if (node->child[child_it]) {
                fprintf(stderr, " %d", node->child[child_it]->nodeid);
            }
        }
    }
    fputc(10, stderr);
}

static void print_symbol(struct Stab *symbol, enum Node_kind nkind)
{
    switch (symbol->type->kind) {
        case T_VOID: fprintf(stderr, "void"); break;
        case T_INT: fprintf(stderr, "int"); break;
        case T_CHAR: fprintf(stderr, "char"); break;
        case T_FLOAT: fprintf(stderr, "float"); break;
        case T_DOUBLE: fprintf(stderr, "double"); break;
        case T_STRUCT: fprintf(stderr, "struct %s", symbol->type->struct_sym->name);
    }
    if (symbol->ptrcount) {
        fprintf(stderr, " (%d)", symbol->ptrcount);
        int t;
        for (t = 0; t < symbol->ptrcount; t++) fputc('*', stderr);
    }
    if (nkind == K_FUNC && symbol->param_cnt) {
        fputc(10, stderr);
        fprintf(stderr, "\tParamlist(%d): ", symbol->param_cnt);
        struct Param_entry *pe;
        for (pe = symbol->param_list; pe; pe = pe->next) {
            print_symbol(pe->symbol, K_DEF);
            fprintf(stderr, " %s\t", pe->symbol->name);
        }
    } else if (nkind == K_DEF && symbol->arysize_cnt) {
        fprintf(stderr, " (%d)", symbol->arysize_cnt);
        struct Arysize_entry *ae;
        for (ae = symbol->arysize_list; ae; ae = ae->next) {
            fprintf(stderr, "[%zd]", ae->size);
        }
    } else if (nkind == K_STRUCT) {
        fprintf(stderr, " (%d)\t", symbol->member_cnt);
    }
}

int syntree_translate(struct Syntree_node *root)
{
    if (root == NULL) return 0;

    struct Syntree_node *node = root;
    for ( ; node; node = node->next) {
        fprintf(stderr, "%2d(%d):\t", node->nodeid, node->env->envid);
        switch (node->nkind) {
            case K_FUNC:
                fprintf(stderr, "Function definition:\t");
                print_symbol(node->symbol, K_FUNC);
                fprintf(stderr, "\tsymbol: %s\t", node->symbol->name);
                print_child(node);
                syntree_translate(node->child[0]);
                break;
            case K_STRUCT:
                fprintf(stderr, "Struct definition:\t");
                print_symbol(node->symbol, K_STRUCT);
                fprintf(stderr, "\tsymbol: %s\t", node->symbol->name);
                print_child(node);
                syntree_translate(node->child[0]);
                break;
            case K_DEF:
                fprintf(stderr, "Var definition: ");
                print_symbol(node->symbol, K_DEF);
                fprintf(stderr, "\tsymbol: %s\t", node->symbol->name);
                print_child(node);
                break;
            case K_STMT:
                switch (node->stmt) {
                    case K_IFELSE:
                        fprintf(stderr, "IF-ELSE statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        syntree_translate(node->child[2]);
                        break;
                    case K_IF:
                        fprintf(stderr, "IF statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_WHILE:
                        fprintf(stderr, "WHILE statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_DO:
                        fprintf(stderr, "DO statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_FOR:
                        fprintf(stderr, "FOR Statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        syntree_translate(node->child[2]);
                        syntree_translate(node->child[3]);
                        break;
                    case K_RET:
                        fprintf(stderr, "Return statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        break;
                }
                break;
            case K_EXPR:
                switch (node->expr) {
                    case K_CHAR:
                        fprintf(stderr, "CHAR constant:\t%c\t", node->c);
                        print_child(node);
                        break;
                    case K_STR:
                        fprintf(stderr, "STRING constant:\t%s\t", node->str);
                        print_child(node);
                        break;
                    case K_INT:
                        fprintf(stderr, "Integer constant:\t%d\t", node->val);
                        print_child(node);
                        break;
                    case K_DOUBLE:
                        fprintf(stderr, "Float point constant:\t%lf\t", node->dval);
                        print_child(node);
                        break;
                    case K_SYM:
                        fprintf(stderr, "symbol:\t%s\t", node->symbol->name);
                        print_child(node);
                        break;
                    case K_ARY:
                        fprintf(stderr, "ARY symbol:\t%s\t", node->symbol->name);
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_CALL:
                        fprintf(stderr, "Function call symbol\t%s\t", node->symbol->name);
                        print_child(node);
                        syntree_translate(node->child[0]);
                        break;
                    case K_OPR:
                        fprintf(stderr, "Operation expression:\t");
                        switch (node->token) {
                            case INC:
                                fprintf(stderr, "OP:\t++(suffix)\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                break;
                            case DEC:
                                fprintf(stderr, "OP:\t--(suffix)\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                break;
                            case DOT:
                                fputs("Struct get member:\n", stderr);
                                fputs("expr:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("member id:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case MEMBER:
                                fputs("Struct pointer get member:\n", stderr);
                                fputs("expr:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("member id:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case PINC:
                                fputs("Prefix INC expression:\n", stderr);
                                syntree_translate(node->child[0]);
                                break;
                            case PDEC:
                                fputs("Prefix DEC expression:\n", stderr);
                                syntree_translate(node->child[0]);
                                break;
                            case UPLUS:
                                fputs("Unary + expression:\n", stderr);
                                syntree_translate(node->child[0]);
                                break;
                            case UMINUS:
                                fprintf(stderr, "OP:\tunary -\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                break;
                            case LNOT:
                                fputs("Logical NOT expression:\n", stderr);
                                syntree_translate(node->child[0]);
                                break;
                            case NOT:
                                fputs("Arithmetic NOT expression:\n", stderr);
                                syntree_translate(node->child[0]);
                                break;
                            case PTR:
                                fputs("Pointer expression:\n", stderr);
                                syntree_translate(node->child[0]);
                                break;
                            case REFR:
                                fputs("Reference expression:\n", stderr);
                                syntree_translate(node->child[0]);
                                break;
                            case MULTIPLY:
                                fputs("Multiply expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case DIVIDE:
                                fputs("DIVIDE expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case MOD:
                                fprintf(stderr, "OP:\t%%\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case PLUS:
                                fprintf(stderr, "OP:\t+\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case MINUS:
                                fprintf(stderr, "OP:\t-\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case SHL:
                                fputs("SHL expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case SHR:
                                fputs("SHR expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case LT:
                                fprintf(stderr, "OP:\t<\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case LE:
                                fputs("LE expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case GT:
                                fputs("GT expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case GE:
                                fputs("GE expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case EQ:
                                fprintf(stderr, "OP:\t==\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case NE:
                                fputs("NE expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case AND:
                                fputs("Arithmetic AND expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case XOR:
                                fputs("Arithmetic XOR expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case OR:
                                fputs("Arithmetic OR expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case LAND:
                                fputs("Logical AND expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case LOR:
                                fputs("Logical OR expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case ASSIGN:
                                fprintf(stderr, "OP:\t=\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case PLUSASN:
                                fputs("PLUS and ASSIGN expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                            case MINUSASN:
                                fputs("MINUS and ASSIGN expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case MULASN:
                                fputs("MULTIPLY and ASSIGN expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case DIVASN:
                                fputs("DIVIDE and ASSIGN expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case MODASN:
                                fputs("MOD and ASSIGN expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case SHLASN:
                                fputs("SHL and ASSIGN expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case SHRASN:
                                fputs("SHR and ASSIGN expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case ANDASN:
                                fputs("AND and ASSIGN expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case XORASN:
                                fputs("XOR and ASSIGN expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case ORASN:
                                fputs("OR and ASSIGN expression:\n", stderr);
                                fputs("expression 1:\n", stderr);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", stderr);
                                syntree_translate(node->child[1]);
                                break;
                            case COMMA:
                                fprintf(stderr, "OP:\t,\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                            default:
                                break;
                        }
                }
                break;
        }
    }

    return 0;
}
