#pragma once
#include "vm.h"

void setup_builtins(struct enclosing *);

// misc stuff
struct object *bee_print(struct enclosing *);
struct object *bee_typename(struct enclosing *);

// pair stuff
struct object *bee_pair(struct enclosing *);
struct object *bee_head(struct enclosing *);
struct object *bee_tail(struct enclosing *);
