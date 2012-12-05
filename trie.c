#include <stdlib.h>
#include <string.h>
#include "trie.h"
#include "stab.h"

struct Trie_node *trie_new_node()
{
    struct Trie_node *node = (struct Trie_node *)malloc(sizeof(struct Trie_node));
    memset(node, 0, sizeof(struct Trie_node));
    return node;
}

struct Stab *trie_insert(struct Trie_node *root, const char *name, int lineno)
{
    struct Trie_node *p = root;
    const char *t;
    for (t = name; *t; t++) {
        int index = *t == '_' ? 52 : *t <= 'Z' ? *t - 'A' : *t - 'a' + 26;
        if (p->next[index] == 0) p->next[index] = trie_new_node();
        p = p->next[index];
    }
    if (p->symbol == NULL) {
        p->symbol = stab_new_symbol(name, lineno);
    }

    return p->symbol;
}

