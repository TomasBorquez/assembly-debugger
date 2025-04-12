#include "base.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

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
        VecPush(result, item);
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
      VecPush(result, item);
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
