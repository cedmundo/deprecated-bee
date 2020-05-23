#define _GNU_SOURCE
#include "hashmap.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define hashmap_hash murmur_oaat64
uint64_t murmur_oaat64(const char *key) {
  uint64_t h = 525201411107845655ull;
  for (; *key; ++key) {
    h ^= *key;
    h *= 0x5bd1e9955bd1e995;
    h ^= h >> 47;
  }
  return h;
}

void hashmap_init(struct hashmap *hm, size_t total_rows, size_t max_objects) {
  assert(hm != NULL);
  size_t buffer_size = sizeof(struct kv_entry *) * total_rows;
  hm->total_rows = total_rows;
  hm->max_objects = max_objects;
  hm->total_objects = 0LL;
  hm->rehash_state = REHASH_A;
  hm->rows = malloc(buffer_size);
  memset(hm->rows, 0L, buffer_size);
}

void hashmap_free(struct hashmap *hm) {
  assert(hm != NULL);
  if (hm->rows != NULL) {
    struct kv_entry **rows = hm->rows;
    for (size_t ri = 0LL; ri < hm->total_rows; ri++) {
      struct kv_entry *row = rows[ri];
      struct kv_entry *tmp = NULL;

      while (row != NULL) {
        tmp = row->next;
        if (row->key != NULL) {
          free(row->key);
        }
        free(row);
        row = tmp;
      }
    }

    free(hm->rows);
  }
}

enum hashmap_state hashmap_put(struct hashmap *hm, char *key,
                               struct object *value) {
  assert(hm != NULL);
  assert(key != NULL);

  uint64_t index = hashmap_reduce(hashmap_hash(key), hm->total_rows);
  struct kv_entry *head = hm->rows[index];
  if (head == NULL) {
    head = malloc(sizeof(struct kv_entry));
    head->next = NULL;
    head->key = strdup(key);
    head->value = value;
    head->rehash_state = hm->rehash_state;
    hm->rows[index] = head;
    hm->total_objects++;
  } else {
    struct kv_entry *tmp = head;
    struct kv_entry *place_after = NULL;
    struct kv_entry *replace = NULL;
    while (tmp != NULL) {
      if (strcmp(tmp->key, key) == 0) {
        replace = tmp;
        break;
      }

      place_after = tmp;
      tmp = tmp->next;
    }

    struct kv_entry *new_entry = malloc(sizeof(struct kv_entry));
    new_entry->key = strdup(key);
    new_entry->value = value;
    new_entry->rehash_state = hm->rehash_state;

    if (replace != NULL && place_after != NULL) {
      // replace within body
      new_entry->next = replace->next;
      free(replace);
      place_after->next = new_entry;
    } else if (replace != NULL && place_after == NULL) {
      // replace head
      new_entry->next = replace->next;
      free(replace);
      hm->rows[index] = new_entry;
    } else if (place_after != NULL) {
      // insert within list body or tail
      new_entry->next = place_after->next;
      place_after->next = new_entry;
      hm->total_objects++;
    } else if (replace == NULL && place_after == NULL) {
      // insert within list head
      new_entry->next = NULL;
      hm->rows[index] = new_entry;
      hm->total_objects++;
    } else {
      free(new_entry);
      return HM_INCONSISTENT_STATE;
    }
  }

  // TODO: this is not available yet ...
  // if (hm->max_objects * DEFAULT_HM_LOAD_FACTOR > hm->total_objects) {
  //   return hashmap_grow(hm, DEFAULT_HM_GROW_FACTOR);
  // }

  return HM_OK;
}

enum hashmap_state hashmap_get(struct hashmap *hm, char *key,
                               struct object **value_out) {
  assert(hm != NULL);
  assert(key != NULL);
  assert(value_out != NULL);
  assert(*value_out == NULL);

  uint64_t index = hashmap_reduce(hashmap_hash(key), hm->total_rows);
  struct kv_entry *list = hm->rows[index];
  if (list != NULL) {
    struct kv_entry *cur = list;
    while (cur != NULL) {
      if (strncmp(cur->key, key, strlen(key)) == 0L) {
        *value_out = cur->value;
        return HM_OK;
      }
      cur = cur->next;
    }
  }

  return HM_KEY_NOT_FOUND;
}

enum hashmap_state hashmap_del(struct hashmap *hm, char *key) {
  assert(hm != NULL);
  assert(key != NULL);

  uint64_t index = hashmap_reduce(hashmap_hash(key), hm->total_rows);
  struct kv_entry *list = hm->rows[index];
  if (list != NULL) {
    struct kv_entry *head = list;
    struct kv_entry *last = NULL;
    struct kv_entry *cur = head;
    while (cur != NULL) {
      if (strncmp(cur->key, key, strlen(key)) == 0L) {
        if (cur == head) {
          // delete list head
          hm->rows[index] = cur->next;
          free(cur);
        } else if (cur->next == NULL) {
          // delete list tail
          last->next = NULL;
          free(cur);
        } else {
          // delete item within list body
          last->next = cur->next;
          free(cur);
        }

        break;
      }

      last = cur;
      cur = cur->next;
    }
  }

  return HM_KEY_NOT_FOUND;
}

enum hashmap_state hashmap_grow(struct hashmap *hm, size_t grow_factor) {
  assert(hm != NULL);
  assert(grow_factor > 1);

  printf("total objects: %lu\n", hm->total_objects);
  printf("old total rows: %lu\n", hm->total_rows);
  size_t new_total_rows = hm->total_rows * grow_factor;
  printf("new total rows: %lu\n", new_total_rows);

  printf("old max objects: %lu\n", hm->max_objects);
  size_t new_max_objects = hm->max_objects * grow_factor;
  printf("new max objects: %lu\n", new_max_objects);

  size_t new_size = sizeof(struct kv_entry *) * new_total_rows;

  hm->total_rows = new_total_rows;
  hm->max_objects = new_max_objects;
  hm->rows = realloc(hm->rows, new_size);
  if (hm->rows == NULL) {
    return HM_OUT_OF_MEMORY;
  }

  return hashmap_rehash(hm);
}

enum hashmap_state hashmap_rehash(struct hashmap *hm) {
  assert(hm != NULL);
  printf("rehashing....\n");
  hm->rehash_state = hm->rehash_state == REHASH_A ? REHASH_B : REHASH_A;
  if (hm->rows != NULL) {
    struct kv_entry **rows = hm->rows;
    for (size_t ri = 0LL; ri < hm->total_rows; ri++) {
      struct kv_entry *col = rows[ri];
      while (col != NULL) {
        if (col->rehash_state == hm->rehash_state) {
          col = col->next;
          continue;
        }

        char *key = col->key;
        uint64_t new_index = hashmap_reduce(hashmap_hash(key), hm->total_rows);
        struct kv_entry *tmp = hm->rows[new_index];
        struct kv_entry *place_after = NULL;
        struct kv_entry *replace = NULL;
        while (tmp != NULL) {
          if (strcmp(tmp->key, key) == 0) {
            replace = tmp;
            break;
          }

          place_after = tmp;
          tmp = tmp->next;
        }

        if (replace != NULL && place_after != NULL) {
          // replace within body
          col->next = replace->next;
          free(replace);
          place_after->next = col;
        } else if (replace != NULL && place_after == NULL) {
          // replace head
          col->next = replace->next;
          free(replace);
          hm->rows[new_index] = col;
        } else if (place_after != NULL) {
          // insert within list body or tail
          col->next = place_after->next;
          place_after->next = col;
        } else if (replace == NULL && place_after == NULL) {
          // insert within list head
          col->next = NULL;
          hm->rows[new_index] = col;
        } else {
          return HM_INCONSISTENT_STATE;
        }

        col = col->next;
      }
    }
  }

  return HM_OK;
}
