#include <stdint.h>
#include <stdio.h>

extern uint8_t foo(uint8_t x);

int main(void) {
  uint8_t i = 0;
  do {
    printf("i = %d\n", i);
    uint8_t res = foo(i);
    printf("0x%x\n", res);
    i++;
  } while (i != 0);
}
