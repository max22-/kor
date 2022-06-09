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
    lit | mode_short, 45, lit, 53, add, lit, 1, trap, halt
  };
  kor_print("Kor VM\n");
  kor_print("Booting...\n");
  kor_boot(&vm);
  kor_load(&vm, image, sizeof(image));
  kor_start(&vm);
  return 0;
}
