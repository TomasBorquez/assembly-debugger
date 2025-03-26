#include "base.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/
static intptr_t alignForward(const intptr_t ptr) {
  intptr_t p, a, modulo;

  p = ptr;
  a = (intptr_t)DEFAULT_ALIGNMENT;
  // Same as (p % a) but faster as 'a' is a power of two
  modulo = p & (a - 1);

  if (modulo != 0) {
    // If 'p' address is not aligned, push the address to the
    // next value which is aligned
    p += a - modulo;
  }
  return p;
}

void *ArenaAlloc(Arena *a, const size_t size) {
  // Align 'currPtr' forward to the specified alignment
  intptr_t currPtr = (intptr_t)a->buffer + (intptr_t)a->currOffset;
  intptr_t offset = alignForward(currPtr);
  offset -= (intptr_t)a->buffer; // Change to relative offset

  assert(offset + size <= a->bufferLength && "Arena ran out of space left");

  void *ptr = &a->buffer[offset];
  a->prevOffset = offset;
  a->currOffset = offset + size;

  // Zero new memory by default
  memset(ptr, 0, size);
  return ptr;
}

void ArenaFree(Arena *arena) {
  free(arena->buffer);
}

void ArenaReset(Arena *arena) {
  arena->currOffset = 0;
}

Arena ArenaInit(size_t size) {
  return (Arena){
      .buffer = (int8_t *)malloc(size),
      .bufferLength = size,
      .prevOffset = 0,
      .currOffset = 0,
  };
}

StringVector StringSplit(const char *s, char delimiter) {
  StringVector result = {NULL, 0, 0};
  const char *start = s;
  const char *end = s;
  while (*end) {
    if (*end == delimiter) {
      size_t len = end - start;
      if (len > 0) {
        char *item = (char *)malloc(len + 1);
        memcpy(item, start, len);
        item[len] = '\0';
        vecPush(result, item);
      }
      start = end + 1;
    }
    end++;
  }

  if (end > start) {
    size_t len = end - start;
    if (len > 0) {
      char *item = (char *)malloc(len + 1);
      memcpy(item, start, len);
      item[len] = '\0';
      vecPush(result, item);
    }
  }
  return result;
}

void FreeStringVector(StringVector *sv) {
  for (size_t i = 0; i < sv->length; i++) {
    free(sv->data[i]);
  }

  free(sv->data);

  sv->data = NULL;
  sv->length = 0;
  sv->capacity = 0;
}
