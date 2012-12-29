#ifndef CMM_ENV_H
#define CMM_ENV_H

#include "trie.h"
#include "stab.h"

struct Env {
    struct Env *prev;
    struct Trie_node *trie_root;
    int symbol_cnt;
    int envid;
    size_t var_size;
    size_t tmp_size;
    size_t call_size;
};

struct Env *global_env;
struct Env *curr_env;

struct Env *env_new(struct Env *);
struct Stab *env_insert(struct Env *, const char *, int);
struct Stab *env_lookup(struct Env *, const char *);
size_t env_size(struct Env *);

void load_std_func(char *, enum Type_kind);

#endif /* !CMM_ENV_H */

