#include <stdlib.h>
#include "env.h"
#include "trie.h"
#include "stab.h"

static int envid_count = 0;

struct Env *env_new(struct Env *prev)
{
    struct Env *e = (struct Env *)malloc(sizeof(struct Env));
    e->prev = prev;
    e->trie_root = NULL;
    e->symbol_cnt = 0;
    e->envid = envid_count++;

    return e;
}

struct Stab *env_insert(struct Env *e, const char *name, int lineno)
{
    if (e->trie_root == NULL) e->trie_root = trie_new_node();
    struct Stab *symbol = trie_insert(e->trie_root, name, lineno);
    if (symbol) e->symbol_cnt++;
    return symbol;
}

struct Stab *env_lookup(struct Env *e, const char *name)
{
    for ( ; e; e = e->prev) {
        if (e->trie_root != NULL) {
            struct Stab *symbol = trie_lookup(e->trie_root, name);
            if (symbol) return symbol;
        }
    }
    return NULL;
}

