#include <stdio.h>
#include <stdlib.h>
#include "kor.h"

void kor_putc(kor* vm)
{
  putchar(kor_pop(vm));
}

void kor_halt(kor *vm)
{
  exit(kor_pop(vm));
}

void kor_print(const char *msg)
{
  printf(msg);
}

void kor_eprint(const char *msg)
{
  fprintf(stderr, msg);
}

void kor_fatal()
{
  exit(1);
}

int main(int argc, char *argv[])
{
  kor vm;
  u8 image[] = {
    lit | mode_short, 45, 00, lit, 53, add, lit, 1, trap,
    lit, 0x0a, lit, 1, trap,
    lit, 1, lit, 0, divmod, drop, drop,
    lit, 0, lit, 0, trap /* halt */
  };
  u8 image2[] = {
    lit, 16, call,
    lit, 98, lit, 1, trap,
    lit, 22, call,
    lit, 0, lit, 0, trap, /* halt */
    /* 16: */
    lit, 97, lit, 1, trap, ret,
    /* newline */
    lit, 0x0a, lit, 1, trap, ret
    
  };
  kor_print("Kor VM\n");
  kor_print("Booting...\n");
  kor_boot(&vm);
  kor_load(&vm, image2, sizeof(image2));
  kor_run(&vm);
  return 0;
}
