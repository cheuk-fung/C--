#include <stdio.h>
#include "asm.h"
#include "global.h"
#include "stab.h"
#include "env.h"
#include "syntree.h"

FILE *fasm;

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
    if (kind == T_CHAR) return "movb";
    if (kind == T_INT) return "movl";
    return "kidding";
}
static char *type_register(enum Type_kind kind)
{
    if (kind == T_CHAR) return "%cl";
    if (kind == T_INT) return "%ecx";
    return "kidding";
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
    fprintf(fasm, "\tsubl\t$%zd, %%esp\n", env_size(node->child[0]->env));
    asm_translate(node->child[0]);
    fprintf(fasm, "\taddl\t$%zd, %%esp\n", env_size(node->child[0]->env));
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
    node->tmppos += node->env->call_size;
    switch (node->se.expr) {
        case K_CHAR:
            if (node->ntype.kind == T_CHAR) {
                fprintf(fasm, "\t%s\t$%d, %zd(%%esp)\n", mov_action(node->ntype.kind), node->info.c, node->tmppos);
            } else if (node->ntype.kind == T_INT) {
                fprintf(fasm, "\t%s\t$%d, %zd(%%esp)\n", mov_action(node->ntype.kind), node->info.c, node->tmppos);
            }
            break;
        case K_CALL:
            {
                asm_translate(node->child[0]);
                size_t offset = 0;
                struct Syntree_node *sn = node->child[0];
                while (sn->se.expr == K_OPR && sn->info.token == COMMA) {
                    fprintf(fasm, "\t%s\t%zd(%%esp), %s\n", mov_action(sn->ntype.kind), sn->tmppos, type_register(sn->ntype.kind));
                    fprintf(fasm, "\t%s\t%s, %zd(%%esp)\n", mov_action(sn->ntype.kind), type_register(sn->ntype.kind), offset);
                    offset += type_size(&sn->ntype);
                    sn = sn->child[1];
                }
                fprintf(fasm, "\t%s\t%zd(%%esp), %s\n", mov_action(sn->ntype.kind), sn->tmppos, type_register(sn->ntype.kind));
                fprintf(fasm, "\t%s\t%s, %zd(%%esp)\n", mov_action(sn->ntype.kind), type_register(sn->ntype.kind), offset);
                fprintf(fasm, "\tcalll\t%s\n", node->info.symbol->name);
                break;
            }
        default:
            {
                int i;
                for (i = 0; i < node->child_count; i++) {
                    asm_translate(node->child[i]);
                }
            }
    }
}

