#include <stdlib.h>
#include <string.h>
#include "trie.h"
#include "stab.h"

static inline int get_index(char c)
{
    return c == '_' ? 52 : c <= 'Z' ? c - 'A' : c - 'a' + 26;
}

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
        int index = get_index(*t);
        if (p->next[index] == NULL) p->next[index] = trie_new_node();
        p = p->next[index];
    }
    if (p->symbol == NULL) {
        p->symbol = stab_new(name, lineno);
        return p->symbol;
    }

    return NULL;
}

struct Stab *trie_lookup(struct Trie_node *root, const char *name)
{
    struct Trie_node *p = root;
    const char *t;
    for (t = name; *t; t++) {
        int index = get_index(*t);
        if (p->next[index] == NULL) return NULL;
        p = p->next[index];
    }
    return p->symbol;
}

size_t trie_size(struct Trie_node *root)
{
    size_t size = 0;
    if (root->symbol) {
        struct Stab *symbol = root->symbol;
        if (!symbol->isfunc && symbol->type->kind != T_STRUCT && symbol->arysize_cnt) {
            size += symbol->size * symbol->arysize_list->size;
        } else {
            size += symbol->size;
        }
    }

    int i;
    for (i = 0; i < 53; i++) {
        if (root->next[i]) {
            size += trie_size(root->next[i]);
        }
    }

    return size;
}
