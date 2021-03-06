#ifndef CMM_TRIE_H
#define CMM_TRIE_H

#include "stab.h"

struct Trie_node {
    struct Trie_node *next[53];
    struct Stab *symbol;
};

struct Trie_node *trie_new_node();
struct Stab *trie_insert(struct Trie_node *, const char *, int);
struct Stab *trie_lookup(struct Trie_node *, const char *);
size_t trie_size(struct Trie_node *);

#endif /* !CMM_TRIE_H */

