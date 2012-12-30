#ifdef NGDEBUG
#include <assert.h>
#endif

#include <stdio.h>
#include "asm.h"
#include "global.h"
#include "stab.h"
#include "env.h"
#include "syntree.h"

#define TO	0
#define FROM	1

FILE *fasm;
struct Env *curr_func_env;
char postmp[1024];
char strtmp[1024];
char eptmp[32];

int label_cnt = 0;

static void asm_global_var(struct Trie_node *node)
{
    if (node->symbol) {
        struct Stab *symbol = node->symbol;
        if (symbol->funcno < 0) {
            if (symbol->type->kind == T_STRUCT) {
                /* TODO */
            } else if (symbol->arysize_cnt) {
                /* TODO */
            } else {
                fprintf(fasm, "\t.comm\t%s,%zd,%zd\n", symbol->name, symbol->size, symbol->size);
                symbol->offset = -1;
            }
        }
    }
    int i;
    for (i = 0; i < 53; i++) {
        if (node->next[i]) {
            asm_global_var(node->next[i]);
        }
    }
}

void asm_head()
{
    int i;
    for (i = 0; i < str_cnt; i++) {
        fprintf(fasm, ".str%d:\t.string\t%s\n", i, strings[i]);
    }
    for (i = 0; i < dbl_cnt; i++) {
        fprintf(fasm, ".dbl%d:\t.long\t%d\n\t.long\t%d\n", i, *(int *)(dbl + i), *((int *)(dbl + i) + 1));
    }
    asm_global_var(global_env->trie_root);

    fprintf(fasm, "\n");
    fprintf(fasm, "\t.text\n");
}

static void translate_function(struct Syntree_node *);
static void translate_statement(struct Syntree_node *);
static void translate_expression(struct Syntree_node *);

static char *mov_action(enum Type_kind kind)
{
    switch (kind) {
        case T_CHAR: return "movb";
        case T_STR: return "movl";
        case T_INT: return "movl";
        case T_FLOAT: return "movl";
        default: return "kidding";
    }
    return 0;
}

static char *type_register(enum Type_kind kind)
{
    switch (kind) {
        case T_CHAR: return "%cl";
        case T_INT: return "%ecx";
        case T_STR: return "%ecx";
        case T_FLOAT: return "%ecx";
        default: return "kidding";
    }
    return 0;
}

static char *get_esp(size_t offset)
{
    if (offset) {
        sprintf(eptmp, "%zd(%%esp)", offset);
    } else {
        sprintf(eptmp, "(%%esp)");
    }
    return eptmp;
}

static char *get_ebp(size_t offset)
{
    sprintf(eptmp, "%zd(%%ebp)", offset + 8);
    return eptmp;
}

static void get_pos(struct Syntree_node *sn, int dir)
{
#ifdef NGDEBUG
    assert(sn->nkind == K_EXPR);
#endif
    switch (sn->se.expr) {
        case K_CHAR:
            sprintf(postmp, "$%d", sn->info.c);
            break;
        case K_STR:
            sprintf(postmp, "$.str%d", sn->info.strno);
            break;
        case K_INT:
            sprintf(postmp, "$%d", sn->info.val);
            break;
        case K_DOUBLE:
            sprintf(postmp, ".dbl%d", sn->info.dblno);
            break;
            /* TODO */
        case K_SYM:
            {
                int offset = sn->info.symbol->offset;
                if (offset == -1) {
                    sprintf(eptmp, "%s", sn->info.symbol->name);
                } else if (offset < curr_func_env->param_size) {
                    get_ebp(offset);
                } else {
                    get_esp(offset + curr_func_env->tmp_size + curr_func_env->call_size - curr_func_env->param_size);
                }
                if (dir == TO || sn->ntype.kind == T_DOUBLE) {
                    sprintf(postmp, "%s", eptmp);
                } else {
                    fprintf(fasm, "\t%s\t%s, %s\n", mov_action(sn->ntype.kind), eptmp, type_register(sn->ntype.kind));
                    sprintf(postmp, "%s", type_register(sn->ntype.kind));
                }
                break;
            }
        case K_ARY:
            /* TODO */
        case K_CALL:
        case K_OPR:
            {
                get_esp(sn->tmppos);
                if (sn->ntype.kind != T_DOUBLE) {
                    fprintf(fasm, "\t%s\t%s, %s\n", mov_action(sn->ntype.kind), eptmp, type_register(sn->ntype.kind));
                    sprintf(postmp, "%s", type_register(sn->ntype.kind));
                } else {
                    sprintf(postmp, "%s", eptmp);
                }
            }
            break;
    }
}

void asm_translate(struct Syntree_node *root)
{
    if (root == NULL) return ;

    struct Syntree_node *sn;
    for (sn = root; sn; sn = sn->next) {
        switch (sn->nkind) {
            case K_FUNC:
                translate_function(sn);
                break;
            case K_STMT:
                translate_statement(sn);
                break;
            case K_EXPR:
                translate_expression(sn);
        }
    }
}

void translate_function(struct Syntree_node *node)
{
    curr_func_env = node->child[0]->env;
    fprintf(fasm, "\n");
    fprintf(fasm, "\t.globl\t%s\n", node->info.symbol->name);
    fprintf(fasm, "%s:\n", node->info.symbol->name);
    fprintf(fasm, "\tpushl\t%%ebp\n");
    fprintf(fasm, "\tmovl\t%%esp, %%ebp\n");
    size_t es = env_size(curr_func_env);
    if (es) {
        fprintf(fasm, "\tsubl\t$%zd, %%esp\n", es);
    }
    asm_translate(node->child[0]);
    fprintf(fasm, ".ret%d:\n", node->info.symbol->funcno);
    if (es) {
        fprintf(fasm, "\taddl\t$%zd, %%esp\n", es);
    }
    fprintf(fasm, "\tpopl\t%%ebp\n");
    fprintf(fasm, "\tret\n");
}

void translate_statement(struct Syntree_node *node)
{
    switch (node->se.stmt) {
        case K_IF:
            {
                asm_translate(node->child[0]);
                get_pos(node->child[0], TO);
                if (postmp[0] == '$') {
                    fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                    sprintf(postmp, "%%eax");
                }
                fprintf(fasm, "\tcmpl\t$0, %s\n", postmp);
                int next_label = label_cnt++;
                fprintf(fasm, "\tjz\t.L%d\n", next_label);
                asm_translate(node->child[1]);
                fprintf(fasm, ".L%d:\n", next_label);
                break;
            }
        case K_IFELSE:
            {
                asm_translate(node->child[0]);
                get_pos(node->child[0], TO);
                if (postmp[0] == '$') {
                    fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                    sprintf(postmp, "%%eax");
                }
                fprintf(fasm, "\tcmpl\t$0, %s\n", postmp);
                int else_label = label_cnt++;
                int next_label = label_cnt++;
                fprintf(fasm, "\tjz\t.L%d\n", else_label);
                asm_translate(node->child[1]);
                fprintf(fasm, "\tjmp\t.L%d\n", next_label);
                fprintf(fasm, ".L%d:\n", else_label);
                asm_translate(node->child[2]);
                fprintf(fasm, ".L%d:\n", next_label);
                break;
            }
        case K_WHILE:
            {
                int start_label = label_cnt++;
                int next_label = label_cnt++;
                fprintf(fasm, ".L%d:\n", start_label);
                asm_translate(node->child[0]);
                get_pos(node->child[0], TO);
                if (postmp[0] == '$') {
                    fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                    sprintf(postmp, "%%eax");
                }
                fprintf(fasm, "\tcmpl\t$0, %s\n", postmp);
                fprintf(fasm, "\tjz\t.L%d\n", next_label);
                asm_translate(node->child[1]);
                fprintf(fasm, "\tjmp\t.L%d\n", start_label);
                fprintf(fasm, ".L%d:\n", next_label);
                break;
            }
        case K_DO:
            /* TODO */
        case K_FOR:
            {
                int judge_label = label_cnt++;
                int body_label = label_cnt++;
                asm_translate(node->child[0]);
                fprintf(fasm, "\tjmp\t.L%d\n", judge_label);
                fprintf(fasm, ".L%d:\n", body_label);
                asm_translate(node->child[3]);
                asm_translate(node->child[2]);
                fprintf(fasm, ".L%d:\n", judge_label);
                if (node->child[1]) {
                    asm_translate(node->child[1]);
                    get_pos(node->child[1], TO);
                    if (postmp[0] == '$') {
                        fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                        sprintf(postmp, "%%eax");
                    }
                    fprintf(fasm, "\tcmpl\t$0, %s\n", postmp);
                    fprintf(fasm, "\tjnz\t.L%d\n", body_label);
                } else {
                    fprintf(fasm, "\tjmp\t.L%d\n", body_label);
                }
                break;
            }
        case K_SWITCH:
            /* TODO */
        case K_GOTO:
            /* TODO */
        case K_RET:
            if (node->child_count) {
                asm_translate(node->child[0]);
                get_pos(node->child[0], TO);
                if (node->info.symbol->type->kind == T_CHAR) {
                    fprintf(fasm, "\tmovzbl\t%s, %%eax\n", postmp);
                } else if (node->info.symbol->type->kind == T_INT) {
                    fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                }
            }
            fprintf(fasm, "\tjmp\t.ret%d\n", node->info.symbol->funcno);
    }
}

void translate_expression(struct Syntree_node *node)
{
    int i;
    for (i = 0; i < node->child_count; i++) {
        asm_translate(node->child[i]);
    }
    node->tmppos += curr_func_env->call_size;
    switch (node->se.expr) {
        case K_CALL:
            {
                size_t offset = 0;
                struct Syntree_node *sn = node->child[0];
                if (sn) {
                    while (sn->se.expr == K_OPR && sn->info.token == COMMA) {
                        get_pos(sn->child[0], FROM);
                        get_esp(offset);
                        if (sn->child[0]->ntype.kind != T_DOUBLE) {
                            fprintf(fasm, "\t%s\t%s, %s\n", mov_action(sn->child[0]->ntype.kind), postmp, eptmp);
                        } else {
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            fprintf(fasm, "\tfstpl\t%s\n", eptmp);
                        }
                        offset += type_size(&sn->child[0]->ntype);
                        sn = sn->child[1];
                    }
                    get_pos(sn, FROM);
                    get_esp(offset);
                    if (sn->ntype.kind != T_DOUBLE) {
                        fprintf(fasm, "\t%s\t%s, %s\n", mov_action(sn->ntype.kind), postmp, eptmp);
                    } else {
                        fprintf(fasm, "\tfldl\t%s\n", postmp);
                        fprintf(fasm, "\tfstpl\t%s\n", eptmp);
                    }
                    offset += type_size(&sn->ntype);
                }
                fprintf(fasm, "\tcalll\t%s\n", node->info.symbol->name);
                if (node->ntype.kind <= T_DOUBLE) {
                    get_esp(node->tmppos);
                    if (node->ntype.kind != T_DOUBLE) {
                        fprintf(fasm, "\tmovl\t%%eax, %s\n", eptmp);
                    } else {
                        /* TODO */
                    }
                }
                break;
            }
        case K_ARY:
            /* TODO */
        case K_OPR:
            {
                switch (node->info.token) {
                    case INC:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], FROM);
                            fprintf(fasm, "\tmovb\t%s, %s\n", postmp, get_esp(node->tmppos));
                            fprintf(fasm, "\tincb\t%s\n", postmp);
                            sprintf(strtmp, "%s", postmp);
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovb\t%s, %s\n", strtmp, postmp);
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], FROM);
                            fprintf(fasm, "\tmovl\t%s, %s\n", postmp, get_esp(node->tmppos));
                            fprintf(fasm, "\tincl\t%s\n", postmp);
                            sprintf(strtmp, "%s", postmp);
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %s\n", strtmp, postmp);
                        }
                        break;
                    case DEC:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], FROM);
                            fprintf(fasm, "\tmovb\t%s, %s\n", postmp, get_esp(node->tmppos));
                            fprintf(fasm, "\tdecb\t%s\n", postmp);
                            sprintf(strtmp, "%s", postmp);
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovb\t%s, %s\n", strtmp, postmp);
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], FROM);
                            fprintf(fasm, "\tmovl\t%s, %s\n", postmp, get_esp(node->tmppos));
                            fprintf(fasm, "\tdecl\t%s\n", postmp);
                            sprintf(strtmp, "%s", postmp);
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %s\n", strtmp, postmp);
                        }
                        break;
                    case DOT:
                        /* TODO */
                        break;
                    case MEMBER:
                        /* TODO */
                        break;
                    case PINC:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], FROM);
                            fprintf(fasm, "\tincb\t%s\n", postmp);
                            fprintf(fasm, "\tmovb\t%s, %s\n", postmp, get_esp(node->tmppos));
                            sprintf(strtmp, "%s", postmp);
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovb\t%s, %s\n", strtmp, postmp);
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], FROM);
                            fprintf(fasm, "\tincl\t%s\n", postmp);
                            fprintf(fasm, "\tmovl\t%s, %s\n", postmp, get_esp(node->tmppos));
                            sprintf(strtmp, "%s", postmp);
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %s\n", strtmp, postmp);
                        }
                        break;
                    case PDEC:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], FROM);
                            fprintf(fasm, "\tdecb\t%s\n", postmp);
                            fprintf(fasm, "\tmovb\t%s, %s\n", postmp, get_esp(node->tmppos));
                            sprintf(strtmp, "%s", postmp);
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovb\t%s, %s\n", strtmp, postmp);
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], FROM);
                            fprintf(fasm, "\tdecl\t%s\n", postmp);
                            fprintf(fasm, "\tmovl\t%s, %s\n", postmp, get_esp(node->tmppos));
                            sprintf(strtmp, "%s", postmp);
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %s\n", strtmp, postmp);
                        }
                        break;
                    case UPLUS:
                        if (node->ntype.kind != T_DOUBLE) {
                            get_pos(node->child[0], FROM);
                            fprintf(fasm, "\t%s\t%s, %s\n", mov_action(node->ntype.kind), postmp, get_esp(node->tmppos));
                        } else {
                            get_pos(node->child[0], FROM);
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            fprintf(fasm, "\tfstpl\t%s\n", get_esp(node->tmppos));
                        }
                        break;
                    case UMINUS:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], FROM);
                            if (postmp[0] != '%') {
                                fprintf(fasm, "\tmovb\t%s, %%al\n", postmp);
                                sprintf(postmp, "%%al");
                            }
                            fprintf(fasm, "\tnegb\t%s\n", postmp);
                            fprintf(fasm, "\tmovb\t%s, %s\n", postmp, get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], FROM);
                            if (postmp[0] != '%') {
                                fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                                sprintf(postmp, "%%eax");
                            }
                            fprintf(fasm, "\tnegl\t%s\n", postmp);
                            fprintf(fasm, "\tmovl\t%s, %s\n", postmp, get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_DOUBLE) {
                            get_pos(node->child[0], FROM);
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            fprintf(fasm, "\tfchs\n");
                            fprintf(fasm, "\tfstpl\t%s\n", get_esp(node->tmppos));
                        }
                        break;
                    case LNOT:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], TO);
                            if (postmp[0] == '$') {
                                fprintf(fasm, "\tmovb\t%s, %%dl\n", postmp);
                                sprintf(postmp, "%%dl");
                            }
                            fprintf(fasm, "\tcmpb\t$0, %s\n", postmp);
                            fprintf(fasm, "\tsete\t%%al\n");
                            fprintf(fasm, "\tmovb\t%%al, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], TO);
                            if (postmp[0] == '$') {
                                fprintf(fasm, "\tmovl\t%s, %%edx\n", postmp);
                                sprintf(postmp, "%%edx");
                            }
                            fprintf(fasm, "\tcmpl\t$0, %s\n", postmp);
                            fprintf(fasm, "\tsete\t%%al\n");
                            fprintf(fasm, "\tmovzbl\t%%al, %%eax\n");
                            fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        }
                        break;
                    case NOT:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], FROM);
                            if (postmp[0] != '%') {
                                fprintf(fasm, "\tmovb\t%s, %%al\n", postmp);
                                sprintf(postmp, "%%al");
                            }
                            fprintf(fasm, "\tnotb\t%s\n", postmp);
                            fprintf(fasm, "\tmovb\t%s, %s\n", postmp, get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], FROM);
                            if (postmp[0] != '%') {
                                fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                                sprintf(postmp, "%%eax");
                            }
                            fprintf(fasm, "\tnotl\t%s\n", postmp);
                            fprintf(fasm, "\tmovl\t%s, %s\n", postmp, get_esp(node->tmppos));
                        }
                        break;
                    case PTR:
                        /* TODO */
                    case REFR:
                        get_pos(node->child[0], TO);
                        fprintf(fasm, "\tleal\t%s, %%edx\n", postmp);
                        fprintf(fasm, "\tmovl\t%%edx, %s\n", get_esp(node->tmppos));
                        break;
                    case MULTIPLY:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%edx\n", postmp);
                            fprintf(fasm, "\timull\t%%edx, %%eax\n");
                            fprintf(fasm, "\tmovb\t%%al, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovl\t%s, %%edx\n", postmp);
                            fprintf(fasm, "\timull\t%%edx, %%eax\n");
                            fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_DOUBLE) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            fprintf(fasm, "\tfmulp\t%%st, %%st(1)\n");
                            fprintf(fasm, "\tfstpl\t%s\n", get_esp(node->tmppos));
                        }
                        break;
                    case DIVIDE:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\tmovl\t%%eax, %%edx\n");
                            fprintf(fasm, "\tsarl\t$31, %%edx\n");
                            fprintf(fasm, "\tidivl\t%%ecx\n");
                            fprintf(fasm, "\tmovb\t%%al, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\tmovl\t%%eax, %%edx\n");
                            fprintf(fasm, "\tsarl\t$31, %%edx\n");
                            fprintf(fasm, "\tidivl\t%%ecx\n");
                            fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_DOUBLE) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            fprintf(fasm, "\tfdivrp\t%%st, %%st(1)\n");
                            fprintf(fasm, "\tfstpl\t%s\n", get_esp(node->tmppos));
                        }
                        break;
                    case MOD:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\tmovl\t%%eax, %%edx\n");
                            fprintf(fasm, "\tsarl\t$31, %%edx\n");
                            fprintf(fasm, "\tidivl\t%%ecx\n");
                            fprintf(fasm, "\tmovb\t%%dl, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\tmovl\t%%eax, %%edx\n");
                            fprintf(fasm, "\tsarl\t$31, %%edx\n");
                            fprintf(fasm, "\tidivl\t%%ecx\n");
                            fprintf(fasm, "\tmovl\t%%edx, %s\n", get_esp(node->tmppos));
                        }
                        break;
                    case PLUS:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%edx\n", postmp);
                            fprintf(fasm, "\taddl\t%%edx, %%eax\n");
                            fprintf(fasm, "\tmovb\t%%al, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovl\t%s, %%edx\n", postmp);
                            fprintf(fasm, "\taddl\t%%edx, %%eax\n");
                            fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_DOUBLE) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            fprintf(fasm, "\tfaddp\t%%st, %%st(1)\n");
                            fprintf(fasm, "\tfstpl\t%s\n", get_esp(node->tmppos));
                        }
                        break;
                    case MINUS:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%edx\n", postmp);
                            fprintf(fasm, "\tsubl\t%%edx, %%eax\n");
                            fprintf(fasm, "\tmovb\t%%al, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovl\t%s, %%edx\n", postmp);
                            fprintf(fasm, "\tsubl\t%%edx, %%eax\n");
                            fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_DOUBLE) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            fprintf(fasm, "\tfsubrp\t%%st, %%st(1)\n");
                            fprintf(fasm, "\tfstpl\t%s\n", get_esp(node->tmppos));
                        }
                        break;
                    case SHL:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\tsall\t%%cl, %%eax\n");
                            fprintf(fasm, "\tmovb\t%%al, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\tsall\t%%cl, %%eax\n");
                            fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        }
                        break;
                    case SHR:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\tsarl\t%%cl, %%eax\n");
                            fprintf(fasm, "\tmovb\t%%al, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\tsarl\t%%cl, %%eax\n");
                            fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        }
                        break;
                    case LT:
                        get_pos(node->child[0], TO);
                        fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                        get_pos(node->child[1], TO);
                        fprintf(fasm, "\tcmpl\t%s, %%eax\n", postmp);
                        fprintf(fasm, "\tsetl\t%%al\n");
                        fprintf(fasm, "\tmovzbl\t%%al, %%eax\n");
                        fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        break;
                    case LE:
                        get_pos(node->child[0], TO);
                        fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                        get_pos(node->child[1], TO);
                        fprintf(fasm, "\tcmpl\t%s, %%eax\n", postmp);
                        fprintf(fasm, "\tsetle\t%%al\n");
                        fprintf(fasm, "\tmovzbl\t%%al, %%eax\n");
                        fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        break;
                    case GT:
                        get_pos(node->child[0], TO);
                        fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                        get_pos(node->child[1], TO);
                        fprintf(fasm, "\tcmpl\t%s, %%eax\n", postmp);
                        fprintf(fasm, "\tsetg\t%%al\n");
                        fprintf(fasm, "\tmovzbl\t%%al, %%eax\n");
                        fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        break;
                    case GE:
                        get_pos(node->child[0], TO);
                        fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                        get_pos(node->child[1], TO);
                        fprintf(fasm, "\tcmpl\t%s, %%eax\n", postmp);
                        fprintf(fasm, "\tsetge\t%%al\n");
                        fprintf(fasm, "\tmovzbl\t%%al, %%eax\n");
                        fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        break;
                    case EQ:
                        get_pos(node->child[0], TO);
                        fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                        get_pos(node->child[1], TO);
                        fprintf(fasm, "\tcmpl\t%s, %%eax\n", postmp);
                        fprintf(fasm, "\tsete\t%%al\n");
                        fprintf(fasm, "\tmovzbl\t%%al, %%eax\n");
                        fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        break;
                    case NE:
                        get_pos(node->child[0], TO);
                        fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                        get_pos(node->child[1], TO);
                        fprintf(fasm, "\tcmpl\t%s, %%eax\n", postmp);
                        fprintf(fasm, "\tsetne\t%%al\n");
                        fprintf(fasm, "\tmovzbl\t%%al, %%eax\n");
                        fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        break;
                    case AND:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\tandl\t%%ecx, %%eax\n");
                            fprintf(fasm, "\tmovb\t%%al, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\tandl\t%%ecx, %%eax\n");
                            fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        }
                        break;
                    case XOR:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\txorl\t%%ecx, %%eax\n");
                            fprintf(fasm, "\tmovb\t%%al, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\txorl\t%%ecx, %%eax\n");
                            fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        }
                        break;
                    case OR:
                        if (node->ntype.kind == T_CHAR) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovzbl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\torl\t%%ecx, %%eax\n");
                            fprintf(fasm, "\tmovb\t%%al, %s\n", get_esp(node->tmppos));
                        } else if (node->ntype.kind == T_INT) {
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tmovl\t%s, %%eax\n", postmp);
                            get_pos(node->child[1], TO);
                            fprintf(fasm, "\tmovl\t%s, %%ecx\n", postmp);
                            fprintf(fasm, "\torl\t%%ecx, %%eax\n");
                            fprintf(fasm, "\tmovl\t%%eax, %s\n", get_esp(node->tmppos));
                        }
                        break;
                    case LAND:
                        get_pos(node->child[0], TO);
                        if (postmp[0] == '$') {
                            fprintf(fasm, "\tmovl\t%s, %%edx\n", postmp);
                            sprintf(postmp, "%%edx");
                        }
                        fprintf(fasm, "\tcmpl\t$0, %s\n", postmp);
                        fprintf(fasm, "\tje\t.L%d\n", label_cnt);
                        get_pos(node->child[1], TO);
                        if (postmp[0] == '$') {
                            fprintf(fasm, "\tmovl\t%s, %%edx\n", postmp);
                            sprintf(postmp, "%%edx");
                        }
                        fprintf(fasm, "\tcmpl\t$0, %s\n", postmp);
                        fprintf(fasm, "\tje\t.L%d\n", label_cnt);
                        fprintf(fasm, "\tmovl\t$1, %s\n", get_esp(node->tmppos));
                        fprintf(fasm, "\tjmp\t.L%d\n", label_cnt + 1);
                        fprintf(fasm, ".L%d:\n", label_cnt);
                        fprintf(fasm, "\tmovl\t$0, %s\n", get_esp(node->tmppos));
                        fprintf(fasm, ".L%d:\n", label_cnt + 1);
                        label_cnt += 2;
                        break;
                    case LOR:
                        get_pos(node->child[0], TO);
                        if (postmp[0] == '$') {
                            fprintf(fasm, "\tmovl\t%s, %%edx\n", postmp);
                            sprintf(postmp, "%%edx");
                        }
                        fprintf(fasm, "\tcmpl\t$0, %s\n", postmp);
                        fprintf(fasm, "\tjne\t.L%d\n", label_cnt);
                        get_pos(node->child[1], TO);
                        if (postmp[0] == '$') {
                            fprintf(fasm, "\tmovl\t%s, %%edx\n", postmp);
                            sprintf(postmp, "%%edx");
                        }
                        fprintf(fasm, "\tcmpl\t$0, %s\n", postmp);
                        fprintf(fasm, "\tjne\t.L%d\n", label_cnt);
                        fprintf(fasm, "\tmovl\t$0, %s\n", get_esp(node->tmppos));
                        fprintf(fasm, "\tjmp\t.L%d\n", label_cnt + 1);
                        fprintf(fasm, ".L%d:\n", label_cnt);
                        fprintf(fasm, "\tmovl\t$1, %s\n", get_esp(node->tmppos));
                        fprintf(fasm, ".L%d:\n", label_cnt + 1);
                        label_cnt += 2;
                        break;
                    case ASSIGN:
                        if (node->ntype.kind != T_DOUBLE) {
                            get_pos(node->child[1], FROM);
                            fprintf(fasm, "\t%s\t%s, ", mov_action(node->ntype.kind), postmp);
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "%s\n", postmp);
                        } else {
                            get_pos(node->child[1], FROM);
                            fprintf(fasm, "\tfldl\t%s\n", postmp);
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "\tfstpl\t%s\n", postmp);
                        }
                        break;
                }
            }
    }
}

