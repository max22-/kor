#include <stdio.h>
#include <stdlib.h>
#include "kor.h"

void kor_putc(char c)
{
  putchar(c);
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
    lit | mode_short, 45, lit, 53, add, lit, 1, trap,
    lit, 0x0a, lit, 1, trap,
    lit, 1, lit, 0, divmod, halt
  };
  u8 image2[] = {
    lit, 12, call,
    lit, 98, lit, 1, trap,
    lit, 18, call,
    halt,
    /* 12: */
    lit, 97, lit, 1, trap, ret,
    /* newline */
    lit, 0x0a, lit, 1, trap, ret
    
  };
  kor_print("Kor VM\n");
  kor_print("Booting...\n");
  kor_boot(&vm);
  kor_load(&vm, image2, sizeof(image2));
  kor_start(&vm);
  return 0;
}
