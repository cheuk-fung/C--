#ifndef CMM_ENV_H
#define CMM_ENV_H

#include "trie.h"

struct Env {
    struct Env *prev;
    struct Trie_node *trie_root;
    int symbol_cnt;
    int envid;
};

struct Env *global_env;
struct Env *curr_env;

struct Env *env_new(struct Env *);
struct Stab *env_insert(struct Env *, const char *, int);
struct Stab *env_lookup(struct Env *, const char *);

#endif /* !CMM_ENV_H */

