#ifdef NGDEBUG
#include <assert.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "global.h"
#include "lexer.h"
#include "syntree.h"

static int nodeid_count = 0;

struct Syntree_node *syntree_new_node(int child_count, enum Node_kind nkind)
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
        printf("Children:");
        int child_it;
        for (child_it = 0; child_it < node->child_count; child_it++) {
            if (node->child[child_it]) {
                printf(" %d", node->child[child_it]->nodeid);
            }
        }
    }
    putchar(10);
}

int syntree_translate(struct Syntree_node *root)
{
    if (root == NULL) return 0;

    struct Syntree_node *node = root;
    for ( ; node; node = node->next) {
        printf("%2d(%d):\t", node->nodeid, node->env->envid);
        switch (node->nkind) {
            case K_FUNC:
                printf("Function definition:\tsymbol: %s\t", node->id->name);
                print_child(node);
                syntree_translate(node->child[0]);
                syntree_translate(node->child[1]);
                break;
            case K_STRUCT:
                break;
            case K_DEF:
                printf("Var definition: ");
                switch (node->id->type) {
                    case T_VOID: printf("void"); break;
                    case T_INT: printf("int"); break;
                    case T_CHAR: printf("char"); break;
                    case T_FLOAT: printf("float"); break;
                    case T_DOUBLE: printf("double"); break;
                    case T_STRUCT: /* TODO */ break;
                }
                if (node->id->ptrcount) {
                    putchar(' ');
                    int t;
                    for (t = 0; t < node->id->ptrcount; t++) putchar('*');
                }
                if (node->id->arycount) {
                    putchar(' ');
                    struct Array_size *as;
                    for (as = node->id->arysize; as; as = as->next) {
                        printf("[%zd]", as->size);
                    }
                }
                printf("\tsymbol: %s\t", node->id->name);
                print_child(node);
                break;
            case K_STMT:
                switch (node->stmt) {
                    case K_IFELSE:
                        printf("IF-ELSE statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        syntree_translate(node->child[2]);
                        break;
                    case K_IF:
                        printf("IF statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_WHILE:
                        printf("WHILE statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_DO:
                        printf("DO statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_FOR:
                        printf("FOR Statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        syntree_translate(node->child[2]);
                        syntree_translate(node->child[3]);
                        break;
                    case K_RET:
                        printf("Return statement:\t");
                        print_child(node);
                        syntree_translate(node->child[0]);
                        break;
                }
                break;
            case K_EXPR:
                switch (node->expr) {
                    case K_CHAR:
                        printf("CHAR constant:\t%c\t", node->c);
                        print_child(node);
                        break;
                    case K_STR:
                        printf("STRING constant:\t%s\t", node->str);
                        print_child(node);
                        break;
                    case K_INT:
                        printf("Integer constant:\t%d\t", node->val);
                        print_child(node);
                        break;
                    case K_FLOAT:
                        printf("Float point constant:\t%lf\t", node->dval);
                        print_child(node);
                        break;
                    case K_ID:
                        printf("symbol:\t%s\t", node->id->name);
                        print_child(node);
                        break;
                    case K_ARY:
                        printf("ARY symbol:\t%s\t", node->id->name);
                        print_child(node);
                        syntree_translate(node->child[0]);
                        syntree_translate(node->child[1]);
                        break;
                    case K_OPR:
                        printf("Operation expression:\t");
                        switch (node->token) {
                            case INC:
                                printf("OP:\t++(suffix)\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                break;
                            case DEC:
                                printf("OP:\t--(suffix)\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                break;
                            case DOT:
                                puts("Struct get member:");
                                puts("expr:");
                                syntree_translate(node->child[0]);
                                puts("member id:");
                                syntree_translate(node->child[1]);
                                break;
                            case MEMBER:
                                puts("Struct pointer get member:");
                                puts("expr:");
                                syntree_translate(node->child[0]);
                                puts("member id:");
                                syntree_translate(node->child[1]);
                                break;
                            case PINC:
                                puts("Prefix INC expression:");
                                syntree_translate(node->child[0]);
                                break;
                            case PDEC:
                                puts("Prefix DEC expression:");
                                syntree_translate(node->child[0]);
                                break;
                            case UPLUS:
                                puts("Unary + expression:");
                                syntree_translate(node->child[0]);
                                break;
                            case UMINUS:
                                printf("OP:\tunary -\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                break;
                            case LNOT:
                                puts("Logical NOT expression:");
                                syntree_translate(node->child[0]);
                                break;
                            case NOT:
                                puts("Arithmetic NOT expression:");
                                syntree_translate(node->child[0]);
                                break;
                            case PTR:
                                puts("Pointer expression:");
                                syntree_translate(node->child[0]);
                                break;
                            case REFR:
                                puts("Reference expression:");
                                syntree_translate(node->child[0]);
                                break;
                            case MULTIPLY:
                                puts("Multiply expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case DIVIDE:
                                puts("DIVIDE expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case MOD:
                                printf("OP:\t%%\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case PLUS:
                                printf("OP:\t+\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case MINUS:
                                printf("OP:\t-\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case SHL:
                                puts("SHL expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case SHR:
                                puts("SHR expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case LT:
                                printf("OP:\t<\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case LE:
                                puts("LE expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case GT:
                                puts("GT expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case GE:
                                puts("GE expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case EQ:
                                printf("OP:\t==\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case NE:
                                puts("NE expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case AND:
                                puts("Arithmetic AND expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case XOR:
                                puts("Arithmetic XOR expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case OR:
                                puts("Arithmetic OR expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case LAND:
                                puts("Logical AND expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case LOR:
                                puts("Logical OR expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case ASSIGN:
                                printf("OP:\t=\t");
                                print_child(node);
                                syntree_translate(node->child[0]);
                                syntree_translate(node->child[1]);
                                break;
                            case PLUSASN:
                                puts("PLUS and ASSIGN expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                            case MINUSASN:
                                puts("MINUS and ASSIGN expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case MULASN:
                                puts("MULTIPLY and ASSIGN expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case DIVASN:
                                puts("DIVIDE and ASSIGN expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case MODASN:
                                puts("MOD and ASSIGN expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case SHLASN:
                                puts("SHL and ASSIGN expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case SHRASN:
                                puts("SHR and ASSIGN expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case ANDASN:
                                puts("AND and ASSIGN expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case XORASN:
                                puts("XOR and ASSIGN expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            case ORASN:
                                puts("OR and ASSIGN expression:");
                                puts("expression 1:");
                                syntree_translate(node->child[0]);
                                puts("expression 2:");
                                syntree_translate(node->child[1]);
                                break;
                            default:
                                break;
                        }
                }
                break;
        }
    }

    return 0;
}
