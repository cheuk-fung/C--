#ifndef PARSER_GLOBAL_H
#define PARSER_GLOBAL_H

#ifndef YYSTYPE
#define YYSTYPE struct Syntree_node *
#endif

#define BOOL unsigned int
#define TRUE 1
#define FALSE 0

#define MAX_STACK_SIZE 1024

#define STACK_PUSH(s, top, item) s[top++] = item
#define STACK_POP(s, top) s[--top]
#define STACK_TOP(s, top) s[top - 1]

int lastval;
double lastdval;

#endif /* !PARSER_GLOBAL_H */

