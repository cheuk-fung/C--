#ifndef PARSER_GLOBAL_H
#define PARSER_GLOBAL_H

#ifndef YYSTYPE
#define YYSTYPE struct Syntree_node *
#endif

#define BOOL unsigned int
#define TRUE 1
#define FALSE 0

#define MAXIDNUM 1024

#include "env.h"
#include "syntree.h"
#include "stab.h"

struct Env *curr_env;
enum Type_kind lasttype;
struct ID_node *lastid_list;
int lastval;
double lastdval;

struct Stab *lastid_stack[MAXIDNUM];
struct Stab *lastid_push(struct Stab *);
struct Stab *lastid_top();
struct Stab *lastid_pop();

#endif /* !PARSER_GLOBAL_H */

