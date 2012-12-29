#ifndef CMM_ASM_H
#define CMM_ASM_H

#include "syntree.h"

extern FILE *fasm;
void asm_head();
void asm_translate(struct Syntree_node *);

#endif /* !CMM_ASM_H */
