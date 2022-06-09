#include "kor.h"

#define STR(x) #x

#ifdef NDEBUG
#define kor_assert(x, msg)
#else
#define kor_assert(x, msg)					 \
  if(!(x)) {							 \
    kor_eprint(msg "\n");					 \
    kor_eprint("file: " __FILE__ "\n");				 \
    kor_eprint("line: " STR(__LINE__) "\n");			 \
    kor_fatal();						 \
  }
#endif

static char *kor_opcode_name[] = {
#define X(x) #x,
  KOR_OPCODES
#undef X
};

static void push(kor *vm, u32 x)
{
  kor_assert(vm->wst.ptr < KOR_STACK_SIZE, "stack overflow");
  vm->wst.dat[vm->wst.ptr++] = x;
}

static u32 pop(kor *vm)
{
  kor_assert(vm->wst.ptr > 0, "stack underflow");
  return vm->wst.dat[--vm->wst.ptr];
}

void kor_boot(kor *vm)
{
  u32 i;
  u8 *ptr = (u8*)vm;
  kor_assert(sizeof(u8) == 1, "sizeof(u8) should be 1");
  kor_assert(sizeof(u16) == 2, "sizeof(u16) should be 2");
  kor_assert(sizeof(u32) == 4, "sizeof(u16) should be 4");
  kor_assert(KOR_OPCODES_COUNT == 32, "OPCODES_COUNT should be 32");
  for(i = 0; i < sizeof(kor); i++)
    ptr[i] = 0;
}

void kor_load(kor *vm, u8 *src, u32 size)
{
  u32 i;
  for(i = 0; i < size; i++)
    vm->mem[i] = src[i];
}

void kor_start(kor *vm)
{
  u8 opcode = nop, op = nop, mode = 0;
  u32 a, b;
  while(op != halt) {
    opcode = vm->mem[vm->pc++];
    op = opcode & 0x1f;
    mode = opcode &0xe0;
    switch(op) {
    case nop:
      break;
    case lit:
      if(mode == mode_byte)
	kor_print("byte\n");
      else if(mode == mode_short)
	kor_print("short\n");
      else if(mode == mode_word)
	kor_print("word\n");
      push(vm, vm->mem[vm->pc++]);
      break;
    case add:
      b = pop(vm);
      a = pop(vm);
      push(vm, a + b);
      break;
    case trap:
      b = pop(vm);
      if(b == 1) {
	a = pop(vm);
	kor_putc(a);
      }
    case halt:
      break;
    default:
      kor_eprint("invalid instruction");
      kor_fatal();
    }
  }
}
