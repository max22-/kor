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
  FILE *f;
  if(argc < 2) {
    kor_eprint("usage: kor file.img\n");
    return 1;
  }
  kor_print("Kor VM\n");
  kor_print("booting...\n");
  kor_boot(&vm);
  kor_print("loading image...\n");
  f = fopen(argv[1], "r");
  if(!f) {
    kor_eprint("error: image not found\n");
    return 1;
  }
  fread(vm.mem, sizeof(vm.mem), 1, f);
  fclose(f);
  kor_run(&vm);
  return 0;
}
