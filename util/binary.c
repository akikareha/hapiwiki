#include <stdio.h>

#include "binary.h"

void binary_to_hex_string(const unsigned char *data, const int size,
                          char *result) {
  int i;

  for (i = 0; i < size; i++) {
    sprintf(result + 2 * i, "%02x", *(data + i));
  }
}
