#include <stdint.h>

uint8_t foo(uint8_t x, uint8_t y) {
  uint8_t a = 0;
  uint8_t b = 0b10101000;

  return x + y + a + b;
}
