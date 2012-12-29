#ifdef NGDEBUG
#include <assert.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "lexer.h"
#include "syntree.h"
#include "stab.h"

#define MAX(x, y) (x) > (y) ? (x) : (y)

FILE *fmsg;
static int nodeid_count = 0;

static void syntree_type_check(struct Syntree_node *node)
{
    if (node->ntype.kind != T_VOID) return ;
    if (node->nkind == K_STMT) {
        switch (node->se.stmt) {
            case K_IFELSE: case K_IF:
            case K_WHILE:
                if (node->child[0]->ntype.kind >= T_VOID) {
                    fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                    exit(1);
                }
                break;
            case K_RET:
                if (node->child_count == 0) {
                    if (node->info.symbol->type->kind != T_VOID) {
                        fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                        exit(1);
                    }
                } else if (node->info.symbol->type->kind < T_VOID) {
                    if (node->child[0]->ntype.kind >= T_VOID) {
                        fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                        exit(1);
                    }
                    node->child[0]->ntype = *(node->info.symbol->type);
                } else if (node->info.symbol->type->kind == T_VOID) {
                    if (node->child[0]->ntype.kind != T_VOID) {
                        fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                        exit(1);
                    }
                } else if (node->info.symbol->type->kind == T_STRUCT) {
                    if (node->child[0]->ntype.kind != T_STRUCT || node->child[0]->ntype.struct_sym != node->info.symbol->type->struct_sym) {
                        fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                        exit(1);
                    }
                } else {
                    fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                    exit(1);
                }
                break;
            case K_FOR: break;
            case K_DO: /* TODO */
            default:
                        fprintf(stderr, "C-- currently does not support line %d now.\n", node->lineno);
                        exit(1);
        }
    } else if (node->nkind == K_EXPR) {
        switch (node->se.expr) {
            case K_OPR: {
                            node->ntype.kind = -1;
                            switch (node->info.token) {
                                case INC: case DEC: case PINC: case PDEC: case NOT:
                                    if (node->child[0]->ntype.kind > T_INT) {
                                        fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                        exit(1);
                                    }
                                    break;
                                case DOT:
                                    if (node->child[0]->ntype.kind != T_STRUCT || node->child[1]->se.expr != K_SYM) {
                                        fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                        exit(1);
                                    }
                                    node->ntype = node->child[1]->ntype;
                                    break;
                                case MEMBER:
                                    /* TODO */
                                    break;
                                case LNOT:
                                    node->ntype.kind = T_INT;
                                case UPLUS: case UMINUS:
                                    if (node->child[0]->ntype.kind > T_DOUBLE) {
                                        fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                        exit(1);
                                    }
                                    break;
                                case PTR:
                                    break; // check when generating codes
                                case REFR:
                                    break; // check when generating codes
                                case LT: case LE: case GT: case GE: case EQ: case NE: case LAND: case LOR:
                                    node->ntype.kind = T_INT;
                                case MULTIPLY: case DIVIDE: case PLUS: case MINUS:
                                    if (node->child[0]->ntype.kind > T_DOUBLE || node->child[1]->ntype.kind > T_DOUBLE) {
                                        fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                        exit(1);
                                    }
                                    node->child[0]->ntype.kind = node->child[1]->ntype.kind = MAX(node->child[0]->ntype.kind, node->child[1]->ntype.kind);
                                    break;
                                case MOD: case SHL: case SHR: case AND: case XOR: case OR:
                                    if (node->child[0]->ntype.kind > T_INT || node->child[1]->ntype.kind > T_INT) {
                                        fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                        exit(1);
                                    }
                                    node->child[0]->ntype.kind = node->child[1]->ntype.kind = MAX(node->child[0]->ntype.kind, node->child[1]->ntype.kind);
                                    break;
                                case ASSIGN:
                                    if (node->child[0]->ntype.kind == T_STRUCT && node->child[1]->ntype.kind == T_STRUCT) {
                                        if (node->child[0]->ntype.struct_sym != node->child[1]->ntype.struct_sym) {
                                            fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                            exit(1);
                                        }
                                    } else if (node->child[0]->ntype.kind > T_DOUBLE || node->child[1]->ntype.kind > T_DOUBLE) {
                                        fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                        exit(1);
                                    }
                                    node->ntype = node->child[1]->ntype = node->child[0]->ntype;
                                    break;
                                case PLUSASN: case MINUSASN: case MULASN: case DIVASN:
                                    /* TODO */
                                    break;
                                case MODASN: case SHLASN: case SHRASN: case ANDASN: case XORASN: case ORASN:
                                    /* TODO */
                                    break;
                                case COMMA:
                                    node->ntype = node->child[1]->ntype;
                            }
                            if (node->ntype.kind == -1) {
                                node->ntype = node->child[0]->ntype;
                            }
                            break;
                        }
            case K_ARY:
                        node->ntype = *(node->info.symbol->type);
                        break; // check when generating codes
            case K_CALL: {
                             if (node->info.symbol->lineno < 0) {
                                 node->ntype = *(node->info.symbol->type);
                                 break;
                             }
                             struct Param_entry *pe = node->info.symbol->param_list;
                             if (node->child[0] == NULL) {
                                 if (pe != NULL) {
                                     fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                     exit(1);
                                 }
                             } else if (pe == NULL) {
                                 fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                 exit(1);
                             } else {
                                 struct Syntree_node *sn = node->child[0];
                                 while (sn->se.expr == K_OPR && sn->info.token == COMMA && pe->next) {
                                     if (sn->child[0]->ntype.kind != pe->symbol->type->kind || sn->child[0]->ntype.struct_sym != pe->symbol->type->struct_sym) {
                                         fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                         exit(1);
                                     }
                                     sn = sn->child[1];
                                     pe = pe->next;
                                 }
#ifdef NGDEBUG
                                 assert(pe);
#endif
                                 if ((sn->se.expr == K_OPR && sn->info.token == COMMA) || pe->next) {
                                     fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                     exit(1);
                                 }
                                 if (sn->ntype.kind != pe->symbol->type->kind || sn->ntype.struct_sym != pe->symbol->type->struct_sym) {
                                     fprintf(stderr, "Bad type at line: %d.\n", node->lineno);
                                     exit(1);
                                 }
                             }
                             node->ntype = *(node->info.symbol->type);
                             break;
                         }
            default:
                         fprintf(stderr, "There must be something wrong at line %d.\n", node->lineno);
                         exit(1);
        }
    }
}

struct Syntree_node *syntree_new_node(int child_count, enum Node_kind nkind, enum Type_kind ntype, void *se, void *info,
        struct Syntree_node *child0, struct Syntree_node *child1, struct Syntree_node *child2, struct Syntree_node *child3)
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
    node->ntype.kind = ntype;
    if (ntype == T_STRUCT) {
        node->ntype.struct_sym = ((struct Stab *)info)->type->struct_sym;
    }
    node->se = (union SE_kind)se;
    if (nkind == K_DOUBLE && ntype == T_DOUBLE) {
        node->info.dval = lastdval;
    } else {
        node->info = (union Information)info;
    }

    if (child0) node->child[0] = child0;
    if (child1) node->child[1] = child1;
    if (child2) node->child[2] = child2;
    if (child3) node->child[3] = child3;
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
        fprintf(fmsg, "Children:");
        int child_it;
        for (child_it = 0; child_it < node->child_count; child_it++) {
            if (node->child[child_it]) {
                fprintf(fmsg, " %d", node->child[child_it]->nodeid);
            }
        }
    }
    fputc(10, fmsg);
}

static void print_symbol(struct Stab *symbol, enum Node_kind nkind)
{
    switch (symbol->type->kind) {
        case T_VOID: fprintf(fmsg, "void"); break;
        case T_INT: fprintf(fmsg, "int"); break;
        case T_CHAR: fprintf(fmsg, "char"); break;
        case T_FLOAT: fprintf(fmsg, "float"); break;
        case T_DOUBLE: fprintf(fmsg, "double"); break;
        case T_STRUCT: fprintf(fmsg, "struct %s", symbol->type->struct_sym->name);
    }
    fprintf(fmsg, "(size %zd)", symbol->size);
    if (symbol->ptr_cnt) {
        fprintf(fmsg, " (%d)", symbol->ptr_cnt);
        int t;
        for (t = 0; t < symbol->ptr_cnt; t++) fputc('*', fmsg);
    }
    if (nkind == K_FUNC && symbol->param_cnt) {
        fputc(10, fmsg);
        fprintf(fmsg, "\tParamlist(%d): ", symbol->param_cnt);
        struct Param_entry *pe;
        for (pe = symbol->param_list; pe; pe = pe->next) {
            print_symbol(pe->symbol, K_DEF);
            fprintf(fmsg, " %s\t", pe->symbol->name);
        }
    } else if (nkind == K_DEF && symbol->arysize_cnt) {
        fprintf(fmsg, " (%d)", symbol->arysize_cnt);
        struct Arysize_entry *ae;
        for (ae = symbol->arysize_list; ae; ae = ae->next) {
            fprintf(fmsg, "[%zd]", ae->size);
        }
    } else if (nkind == K_STRUCT) {
        fprintf(fmsg, " (%d)\t", symbol->member_cnt);
    }
}

int syntree_translate(struct Syntree_node *root)
{
    if (root == NULL) return 0;

    struct Syntree_node *node = root;
    for ( ; node; node = node->next) {
        fprintf(fmsg, "%2d(%d):\t", node->nodeid, node->env->envid);
        switch (node->nkind) {
            case K_FUNC:
                fprintf(fmsg, "Function definition:\t");
                print_symbol(node->info.symbol, K_FUNC);
                fprintf(fmsg, "\tsymbol: %s\t", node->info.symbol->name);
                print_child(node);
                syntree_translate(node->child[0]);
                break;
            case K_STRUCT:
                fprintf(fmsg, "Struct definition:\t");
                print_symbol(node->info.symbol, K_STRUCT);
                fprintf(fmsg, "\tsymbol: %s\t", node->info.symbol->name);
                print_child(node);
                syntree_translate(node->child[0]);
                break;
            case K_DEF:
                fprintf(fmsg, "Var definition: ");
                print_symbol(node->info.symbol, K_DEF);
                fprintf(fmsg, "\tsymbol: %s\t", node->info.symbol->name);
                print_child(node);
                break;
            case K_STMT:
                switch (node->se.stmt) {
                    case K_IFELSE:
                        fprintf(fmsg, "IF-ELSE statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        syntree_translate(node->child[2]);
                        break;
                    case K_IF:
                        fprintf(fmsg, "IF statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_WHILE:
                        fprintf(fmsg, "WHILE statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_DO:
                        fprintf(fmsg, "DO statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_FOR:
                        fprintf(fmsg, "FOR Statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        syntree_translate(node->child[2]);
                        syntree_translate(node->child[3]);
                        break;
                    case K_RET:
                        fprintf(fmsg, "Return statement of %s:\t", node->info.symbol->name);
                        print_child(node);
                        if (node->child_count) syntree_translate(node->child[0]);
                        break;
                }
                break;
            case K_EXPR:
                switch (node->se.expr) {
                    case K_CHAR:
                        fprintf(fmsg, "CHAR constant:\t%c\t", node->info.c);
                        print_child(node);
                        break;
                    case K_STR:
                        fprintf(fmsg, "STRING constant:\t%s\t", strings[node->info.strno]);
                        print_child(node);
                        break;
                    case K_INT:
                        fprintf(fmsg, "Integer constant:\t%d\t", node->info.val);
                        print_child(node);
                        break;
                    case K_DOUBLE:
                        fprintf(fmsg, "Float point constant:\t%lf\t", node->info.dval);
                        print_child(node);
                        break;
                    case K_SYM:
                        fprintf(fmsg, "symbol:\t%s\t", node->info.symbol->name);
                        print_child(node);
                        break;
                    case K_ARY:
                        fprintf(fmsg, "ARY symbol:\t%s\t", node->info.symbol->name);
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_CALL:
                        fprintf(fmsg, "Function call symbol\t%s\t", node->info.symbol->name);
                        print_child(node);
                        syntree_translate(node->child[0]);
                        break;
                    case K_OPR:
                        fprintf(fmsg, "Operation expression:\t");
                        switch (node->info.token) {
                            case INC:
                                fprintf(fmsg, "OP:\t++(suffix)\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                break;
                            case DEC:
                                fprintf(fmsg, "OP:\t--(suffix)\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                break;
                            case DOT:
                                fputs("Struct get member:\n", fmsg);
                                fputs("expr:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("member id:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case MEMBER:
                                fputs("Struct pointer get member:\n", fmsg);
                                fputs("expr:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("member id:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case PINC:
                                fputs("Prefix INC expression:\n", fmsg);
                                syntree_translate(node->child[0]);
                                break;
                            case PDEC:
                                fputs("Prefix DEC expression:\n", fmsg);
                                syntree_translate(node->child[0]);
                                break;
                            case UPLUS:
                                fputs("Unary + expression:\n", fmsg);
                                syntree_translate(node->child[0]);
                                break;
                            case UMINUS:
                                fprintf(fmsg, "OP:\tunary -\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                break;
                            case LNOT:
                                fputs("Logical NOT expression:\n", fmsg);
                                syntree_translate(node->child[0]);
                                break;
                            case NOT:
                                fputs("Arithmetic NOT expression:\n", fmsg);
                                syntree_translate(node->child[0]);
                                break;
                            case PTR:
                                fputs("Pointer expression:\n", fmsg);
                                syntree_translate(node->child[0]);
                                break;
                            case REFR:
                                fputs("Reference expression:\n", fmsg);
                                syntree_translate(node->child[0]);
                                break;
                            case MULTIPLY:
                                fputs("Multiply expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case DIVIDE:
                                fputs("DIVIDE expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case MOD:
                                fprintf(fmsg, "OP:\t%%\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case PLUS:
                                fprintf(fmsg, "OP:\t+\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case MINUS:
                                fprintf(fmsg, "OP:\t-\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case SHL:
                                fputs("SHL expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case SHR:
                                fputs("SHR expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case LT:
                                fprintf(fmsg, "OP:\t<\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case LE:
                                fputs("LE expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case GT:
                                fputs("GT expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case GE:
                                fputs("GE expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case EQ:
                                fprintf(fmsg, "OP:\t==\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case NE:
                                fputs("NE expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case AND:
                                fputs("Arithmetic AND expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case XOR:
                                fputs("Arithmetic XOR expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case OR:
                                fputs("Arithmetic OR expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case LAND:
                                fputs("Logical AND expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case LOR:
                                fputs("Logical OR expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case ASSIGN:
                                fprintf(fmsg, "OP:\t=\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case PLUSASN:
                                fputs("PLUS and ASSIGN expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                            case MINUSASN:
                                fputs("MINUS and ASSIGN expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case MULASN:
                                fputs("MULTIPLY and ASSIGN expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case DIVASN:
                                fputs("DIVIDE and ASSIGN expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case MODASN:
                                fputs("MOD and ASSIGN expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case SHLASN:
                                fputs("SHL and ASSIGN expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case SHRASN:
                                fputs("SHR and ASSIGN expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case ANDASN:
                                fputs("AND and ASSIGN expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case XORASN:
                                fputs("XOR and ASSIGN expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case ORASN:
                                fputs("OR and ASSIGN expression:\n", fmsg);
                                fputs("expression 1:\n", fmsg);
                                syntree_translate(node->child[0]);
                                fputs("expression 2:\n", fmsg);
                                syntree_translate(node->child[1]);
                                break;
                            case COMMA:
                                fprintf(fmsg, "OP:\t,\t");
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
