#include "neg2.h"
#include <cstdio>

int main() {
  uint8_t a = 1;
  uint8_t res;

  uint8_t expected = 1 << 1;

  printf("Neg2 before simulating\n");
  simulate(&res, a);

  printf("2 bit not expected = %c\n", expected);
  printf("2 bit not result   = %c\n", res);

  if (res == expected) {
    return 0;
  }

  return 1;
}
