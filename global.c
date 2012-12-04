#include <assert.h>
#include "global.h"
#include "stab.h"

static int top = 0;

struct Stab *lastid_push(struct Stab *id)
{
    assert(top + 1 < MAXIDNUM);
    return lastid_stack[top++] = id;
}

struct Stab *lastid_top()
{
    assert(top > 0);
    return lastid_stack[top - 1];
}

struct Stab *lastid_pop()
{
    assert(top > 0);
    return lastid_stack[--top];
}

