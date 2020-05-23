#pragma once
#include <stddef.h>
#include <stdint.h>

struct object;
enum rehash_state { REHASH_A, REHASH_B };
struct kv_entry {
  struct kv_entry *next;
  char *key;
  struct object *value;
  enum rehash_state rehash_state;
};

struct hashmap {
  struct kv_entry **rows;
  size_t total_rows;
  size_t total_objects;
  size_t max_objects;
  enum rehash_state rehash_state;
};

enum hashmap_state {
  HM_OK,
  HM_KEY_NOT_FOUND,
  HM_OUT_OF_MEMORY,
  HM_INCONSISTENT_STATE,
};

#define hashmap_reduce(h, n) (h % n)
#define DEFAULT_HM_TOTAL_ROWS 20
#define DEFAULT_HM_MAX_OBJECTS 200
#define DEFAULT_HM_LOAD_FACTOR 0.75f
#define DEFAULT_HM_GROW_FACTOR 10

void hashmap_init(struct hashmap *hm, size_t total_rows, size_t max_objects);
void hashmap_free(struct hashmap *hm);
enum hashmap_state hashmap_put(struct hashmap *hm, char *key,
                               struct object *value);
enum hashmap_state hashmap_get(struct hashmap *hm, char *key,
                               struct object **value_out);
enum hashmap_state hashmap_del(struct hashmap *hm, char *key);
enum hashmap_state hashmap_grow(struct hashmap *hm, size_t grow_factor);
enum hashmap_state hashmap_rehash(struct hashmap *hm);
