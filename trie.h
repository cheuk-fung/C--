#ifndef PARSER_TRIE_H
#define PARSER_TRIE_H

#include "stab.h"

struct Trie_node {
    struct Trie_node *next[53];
    struct Stab *symbol;
};

struct Stab *trie_insert(struct Trie_node **, const char *, int);

#endif /* !PARSER_TRIE_H */

