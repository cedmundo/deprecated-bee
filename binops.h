#pragma once
#include "vm.h"

// i64
struct value handle_i64_u64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_i64_i64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_i64_f64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_i64_str(struct value left, struct value right,
                            enum bin_op op);

// u64
struct value handle_u64_u64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_u64_i64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_u64_f64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_u64_str(struct value left, struct value right,
                            enum bin_op op);

// f64
struct value handle_f64_u64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_f64_i64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_f64_f64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_f64_str(struct value left, struct value right,
                            enum bin_op op);

// str
struct value handle_str_u64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_str_i64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_str_f64(struct value left, struct value right,
                            enum bin_op op);

struct value handle_str_str(struct value left, struct value right,
                            enum bin_op op);

struct value handle_bin_op(struct value left, struct value right,
                           enum bin_op op);
