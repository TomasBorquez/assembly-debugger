#pragma once
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* Vector */
#define vecPush(vector, value)                                                                                                                                                                                                                 \
  ({                                                                                                                                                                                                                                           \
    if (vector.length >= vector.capacity) {                                                                                                                                                                                                    \
      if (vector.capacity == 0) vector.capacity = 128;                                                                                                                                                                                           \
      else vector.capacity *= 2;                                                                                                                                                                                                               \
      vector.data = realloc(vector.data, vector.capacity * sizeof(*vector.data));                                                                                                                                                              \
    }                                                                                                                                                                                                                                          \
    vector.data[vector.length++] = value;                                                                                                                                                                                                      \
  })

#define vecPop(vector)                                                                                                                                                                                                                         \
  ({                                                                                                                                                                                                                                           \
    assert(vector.length > 0 && "Cannot pop from empty vector");                                                                                                                                                                               \
    typeof(vector.data[0]) value = vector.data[vector.length - 1];                                                                                                                                                                             \
    vector.length--;                                                                                                                                                                                                                           \
    value;                                                                                                                                                                                                                                     \
  })

#define vecShift(vector)                                                                                                                                                                                                                       \
  ({                                                                                                                                                                                                                                           \
    assert(vector.length != 0 && "Length should at least be >= 1");                                                                                                                                                                            \
    typeof(vector.data[0]) value = vector.data[0];                                                                                                                                                                                             \
    memmove(&vector.data[0], &vector.data[1], (vector.length - 1) * sizeof(*vector.data));                                                                                                                                                     \
    vector.length--;                                                                                                                                                                                                                           \
    value;                                                                                                                                                                                                                                     \
  })

#define vecUnshift(vector, value)                                                                                                                                                                                                              \
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
    value;                                                                                                                                                                                                                                     \
  })

#define vecInsert(vector, value, index)                                                                                                                                                                                                        \
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
    value;                                                                                                                                                                                                                                     \
  })

#define vecAt(vector, index)                                                                                                                                                                                                                  \
  ({                                                                                                                                                                                                                                           \
    assert(index < vector.length && "Index out of bounds for insertion");                                                                                                                                                                      \
    vector.data[index];                                                                                                                                                                                                                        \
  })

#define vecFree(vector)                                                                                                                                                                                                                  \
  ({                                                                                                                                                                                                                                           \
    assert(vector.data != NULL && "Vector data should never be NULL");                                                                                                                                                                      \
    free(vector.data);                                                                                                                                                                                                                        \
    vector.data = NULL; \
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

