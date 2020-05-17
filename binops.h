#pragma once
#include "vm.h"

// i64
struct object *handle_i64_u64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_i64_i64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_i64_f64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_i64_str(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

// u64
struct object *handle_u64_u64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_u64_i64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_u64_f64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_u64_str(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

// f64
struct object *handle_f64_u64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_f64_i64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_f64_f64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_f64_str(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

// str
struct object *handle_str_u64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_str_i64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_str_f64(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_str_str(struct vm *vm, struct object *left,
                              struct object *right, enum bin_op op);

struct object *handle_bin_op(struct vm *vm, struct object *left,
                             struct object *right, enum bin_op op);
