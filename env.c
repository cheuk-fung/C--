#include <stdlib.h>
#include "env.h"
#include "trie.h"
#include "stab.h"

static int envid_count = 0;

struct Env *env_new(struct Env *prev)
{
    struct Env *e = (struct Env*)malloc(sizeof(struct Env));
    e->prev = prev;
    e->trie_root = 0;
    e->envid = envid_count++;

    return e;
}

struct Stab *env_insert(struct Env **e, const char *name, int lineno)
{
    if (*e == 0) *e = env_new(0);
    return trie_insert(&(*e)->trie_root, name, lineno);
}

