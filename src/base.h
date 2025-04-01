#pragma once
#include "khash.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* HashMap */
// TODO Add:
// - HashInit
// - HashPut
// - HashDel
// - HashExist
#define HashMapGet(type, hashMap, key)                                                                                                                                                                                                         \
  ({                                                                                                                                                                                                                                           \
    khiter_t k = kh_get(type, hashMap, key);                                                                                                                                                                                                   \
    (k == kh_end(hashMap)) ? NULL : &kh_value(hashMap, k);                                                                                                                                                                                     \
  })

#define HashMapSet(type, hashMap, key, value)                                                                                                                                                                                                  \
  ({                                                                                                                                                                                                                                           \
    int32_t ret;                                                                                                                                                                                                                               \
    khiter_t k = kh_put(type, hashMap, key, &ret);                                                                                                                                                                                             \
    kh_value(hashMap, k) = value;                                                                                                                                                                                                              \
    &kh_value(hashMap, k);                                                                                                                                                                                                                     \
  })

/* Vector */
// TODO: Change to having to type Vector yourself
#define VEC_TYPE(typeName, valueType)                                                                                                                                                                                                          \
  typedef struct {                                                                                                                                                                                                                             \
    valueType *data;                                                                                                                                                                                                                           \
    int32_t length;                                                                                                                                                                                                                            \
    int32_t capacity;                                                                                                                                                                                                                          \
  } Vector##typeName;

#define VecPush(vector, value)                                                                                                                                                                                                                 \
  ({                                                                                                                                                                                                                                           \
    if (vector.length >= vector.capacity) {                                                                                                                                                                                                    \
      if (vector.capacity == 0) vector.capacity = 128;                                                                                                                                                                                         \
      else vector.capacity *= 2;                                                                                                                                                                                                               \
      vector.data = realloc(vector.data, vector.capacity * sizeof(*vector.data));                                                                                                                                                              \
    }                                                                                                                                                                                                                                          \
    vector.data[vector.length++] = value;                                                                                                                                                                                                      \
    &vector.data[vector.length - 1];                                                                                                                                                                                                           \
  })

#define VecPop(vector)                                                                                                                                                                                                                         \
  ({                                                                                                                                                                                                                                           \
    assert(vector.length > 0 && "Cannot pop from empty vector");                                                                                                                                                                               \
    typeof(vector.data[0]) value = vector.data[vector.length - 1];                                                                                                                                                                             \
    vector.length--;                                                                                                                                                                                                                           \
    &value;                                                                                                                                                                                                                                    \
  })

#define VecShift(vector)                                                                                                                                                                                                                       \
  ({                                                                                                                                                                                                                                           \
    assert(vector.length != 0 && "Length should at least be >= 1");                                                                                                                                                                            \
    typeof(vector.data[0]) value = vector.data[0];                                                                                                                                                                                             \
    memmove(&vector.data[0], &vector.data[1], (vector.length - 1) * sizeof(*vector.data));                                                                                                                                                     \
    vector.length--;                                                                                                                                                                                                                           \
    &value;                                                                                                                                                                                                                                    \
  })

#define VecUnshift(vector, value)                                                                                                                                                                                                              \
  ({                                                                                                                                                                                                                                           \
    if (vector.length >= vector.capacity) {                                                                                                                                                                                                    \
      if (vector.capacity == 0) vector.capacity = 2;                                                                                                                                                                                           \
      else vector.capacity *= 2;                                                                                                                                                                                                               \
      vector.data = realloc(vector.data, vector.capacity * sizeof(*vector.data));                                                                                                                                                              \
    }                                                                                                                                                                                                                                          \
                                                                                                                                                                                                                                               \
    if (vector.length > 0) {                                                                                                                                                                                                                   \
      memmove(&vector.data[1], &vector.data[0], vector.length * sizeof(*vector.data));                                                                                                                                                         \
    }                                                                                                                                                                                                                                          \
                                                                                                                                                                                                                                               \
    vector.data[0] = value;                                                                                                                                                                                                                    \
    vector.length++;                                                                                                                                                                                                                           \
    &value;                                                                                                                                                                                                                                    \
  })

#define VecInsert(vector, value, index)                                                                                                                                                                                                        \
  ({                                                                                                                                                                                                                                           \
    assert(index <= vector.length && "Index out of bounds for insertion");                                                                                                                                                                     \
    if (vector.length >= vector.capacity) {                                                                                                                                                                                                    \
      if (vector.capacity == 0) vector.capacity = 2;                                                                                                                                                                                           \
      else vector.capacity *= 2;                                                                                                                                                                                                               \
      vector.data = realloc(vector.data, vector.capacity * sizeof(*vector.data));                                                                                                                                                              \
    }                                                                                                                                                                                                                                          \
    memmove(&vector.data[index + 1], &vector.data[index], (vector.length - index) * sizeof(*vector.data));                                                                                                                                     \
    vector.data[index] = value;                                                                                                                                                                                                                \
    vector.length++;                                                                                                                                                                                                                           \
    &value;                                                                                                                                                                                                                                    \
  })

#define VecAt(vector, index)                                                                                                                                                                                                                   \
  ({                                                                                                                                                                                                                                           \
    assert(index >= 0 && index < vector.length && "Index out of bounds");                                                                                                                                                                      \
    &vector.data[index];                                                                                                                                                                                                                       \
  })

#define VecFree(vector)                                                                                                                                                                                                                        \
  ({                                                                                                                                                                                                                                           \
    assert(vector.data != NULL && "Vector data should never be NULL");                                                                                                                                                                         \
    free(vector.data);                                                                                                                                                                                                                         \
    vector.data = NULL;                                                                                                                                                                                                                        \
  })

/* StringVector */
typedef struct {
  char **data;
  size_t length;
  size_t capacity;
} StringVector;

StringVector StringSplit(const char *s, char delimiter);
void FreeStringVector(StringVector *sv);

/* Arena */
typedef struct {
  int8_t *buffer;
  size_t bufferLength;
  size_t prevOffset;
  size_t currOffset;
} Arena;

// This makes sure right alignment on 86/64 bits
#define DEFAULT_ALIGNMENT (2 * sizeof(void *))

Arena ArenaInit(size_t size);
void *ArenaAlloc(Arena *arena, size_t size);
void ArenaFree(Arena *arena);
void ArenaReset(Arena *arena);
