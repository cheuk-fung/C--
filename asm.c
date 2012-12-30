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
char postmp[1024];
char eptmp[32];

static void asm_global_var(struct Trie_node *node)
{
    if (node->symbol) {
        struct Stab *symbol = node->symbol;
        if (!symbol->isfunc) {
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
            /* TODO */
        case K_SYM:
            {
                int offset = sn->info.symbol->offset;
                if (offset == -1) {
                    sprintf(eptmp, "%s", sn->info.symbol->name);
                } else if (offset < sn->env->param_size) {
                    get_ebp(offset);
                } else {
                    get_esp(offset + sn->env->tmp_size + sn->env->call_size - sn->env->param_size);
                }
                if (dir == TO) {
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
            fprintf(fasm, "\t%s\t%s, %s\n", mov_action(sn->ntype.kind), get_esp(sn->tmppos), type_register(sn->ntype.kind));
            sprintf(postmp, "%s", type_register(sn->ntype.kind));
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
    fprintf(fasm, "\n");
    fprintf(fasm, "\t.globl\t%s\n", node->info.symbol->name);
    fprintf(fasm, "%s:\n", node->info.symbol->name);
    fprintf(fasm, "\tpushl\t%%ebp\n");
    fprintf(fasm, "\tmovl\t%%esp, %%ebp\n");
    size_t es = env_size(node->child[0]->env);
    if (es) {
        fprintf(fasm, "\tsubl\t$%zd, %%esp\n", env_size(node->child[0]->env));
    }
    asm_translate(node->child[0]);
    if (es) {
        fprintf(fasm, "\taddl\t$%zd, %%esp\n", env_size(node->child[0]->env));
    }
    fprintf(fasm, "\tpopl\t%%ebp\n");
    fprintf(fasm, "\tret\n");
}

void translate_statement(struct Syntree_node *node)
{
    switch (node->se.stmt) {
    }
}

void translate_expression(struct Syntree_node *node)
{
    int i;
    for (i = 0; i < node->child_count; i++) {
        asm_translate(node->child[i]);
    }
    node->tmppos += node->env->call_size;
    switch (node->se.expr) {
        case K_CALL:
            {
                size_t offset = 0;
                struct Syntree_node *sn = node->child[0];
                if (sn) {
                    while (sn->se.expr == K_OPR && sn->info.token == COMMA) {
                        get_pos(sn->child[0], FROM);
                        get_esp(offset);
                        fprintf(fasm, "\t%s\t%s, %s\n", mov_action(sn->child[0]->ntype.kind), postmp, eptmp);
                        offset += type_size(&sn->child[0]->ntype);
                        sn = sn->child[1];
                    }
                    get_pos(sn, FROM);
                    get_esp(offset);
                    fprintf(fasm, "\t%s\t%s, %s\n", mov_action(sn->ntype.kind), postmp, eptmp);
                    offset += type_size(&sn->ntype);
                }
                fprintf(fasm, "\tcalll\t%s\n", node->info.symbol->name);
                if (node->ntype.kind <= T_DOUBLE) {
                    get_esp(node->tmppos);
                    fprintf(fasm, "\tmovl\t%%eax, %s\n", eptmp);
                }
                break;
            }
        case K_ARY:
        case K_OPR:
            {
                switch (node->info.token) {
                    case ASSIGN:
                        {
                            get_pos(node->child[1], FROM);
                            fprintf(fasm, "\t%s\t%s, ", mov_action(node->ntype.kind), postmp);
                            get_pos(node->child[0], TO);
                            fprintf(fasm, "%s\n", postmp);
                            break;
                        }
                }
            }
        default:
            break;
    }
}

