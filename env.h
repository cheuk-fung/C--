#ifndef PARSER_ENV_H
#define PARSER_ENV_H

#include "trie.h"
#include "stab.h"

struct Env {
    struct Env *prev;
    struct Trie_node *trie_root;
    int envid;
};

struct Env *curr_env;

struct Env *env_new(struct Env *);
struct Stab *env_insert(struct Env *, const char *, int);

#endif /* !PARSER_ENV_H */

