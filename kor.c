#include "kor.h"

#define STR(x) #x

/* assertions used for debugging the virtual machine core */
#ifdef NDEBUG
#define kassert(x, msg)
#else
#define kassert(x, msg)						 \
  do {								 \
    if(!(x)) {							 \
      kor_eprint(msg "\n");					 \
      kor_eprint("file: " __FILE__ "\n");			 \
      kor_eprint("line: " STR(__LINE__) "\n");			 \
      kor_fatal();						 \
    }								 \
  }								 \
  while (0) 
#endif

/* assertions that trigger interrupts
 * x : assertion
 * vm : the virtual machine instance
 * n : the interrupt number */
#define iassert(x, vm, n)			\
  do {						\
    if(!(x)) {					\
      kor_interrupt(vm, n);			\
    }						\
  }						\
  while (0)

static char *kor_opcode_name[] = {
#define X(x) #x,
  KOR_OPCODES
#undef X
};

static char *kor_interrupt_name[] = {
#define X(x) #x,
  KOR_INTERRUPTS
#undef X
};

static void push(kor *vm, u32 x)
{
  iassert(vm->wst.ptr < KOR_STACK_SIZE, vm, WST_OVERFLOW);
  vm->wst.dat[vm->wst.ptr++] = x;
}

static u32 pop(kor *vm)
{
  iassert(vm->wst.ptr > 0, vm, WST_UNDERFLOW);
  return vm->wst.dat[--vm->wst.ptr];
}

static void rpush(kor *vm, u32 x)
{
  iassert(vm->rst.ptr < KOR_STACK_SIZE, vm, RST_OVERFLOW);
  vm->rst.dat[vm->rst.ptr++] = x;
}

static u32 rpop(kor *vm)
{
  iassert(vm->rst.ptr > 0, vm, RST_UNDERFLOW);
  return vm->rst.dat[--vm->rst.ptr];
}

void kor_boot(kor *vm)
{
  u32 i;
  u8 *ptr = (u8*)vm;
  kassert(sizeof(u8) == 1, "sizeof(u8) should be 1");
  kassert(sizeof(u16) == 2, "sizeof(u16) should be 2");
  kassert(sizeof(u32) == 4, "sizeof(u16) should be 4");
  kassert(KOR_OPCODES_COUNT == 32, "OPCODES_COUNT should be 32");
  for(i = 0; i < sizeof(kor); i++)
    ptr[i] = 0;
}

void kor_load(kor *vm, u8 *src, u32 size)
{
  u32 i;
  for(i = 0; i < size; i++)
    vm->mem[i] = src[i];
}

void kor_interrupt(kor *vm, int n)
{
  if(n < INTERRUPTS_COUNT) {
    kor_print("interrupt: ");
    kor_print(kor_interrupt_name[n]);
    kor_print("\n");
}
  else
    kor_eprint("unknown interrupt\n");
  
}

#define fetch_byte(vm, x)						\
  do {									\
    iassert(vm->pc < KOR_MEM_SIZE, vm, MEMORY_ACCESS_ERROR);		\
    x = vm->mem[vm->pc++];						\
  }									\
  while(0)

void kor_start(kor *vm)
{
  u8 opcode = nop, op = nop, mode = 0;
  u32 a, b, c;
  while(op != halt) {
    fetch_byte(vm, opcode);
    op = opcode & 0x1f;
    mode = opcode &0xe0;
    switch(op) {
    case nop:
      break;
    case lit:
      /*if(mode == mode_byte)
	kor_print("byte\n");
      else if(mode == mode_short)
	kor_print("short\n");
      else if(mode == mode_word)
      kor_print("word\n");*/
      fetch_byte(vm, a);
      push(vm, a);
      break;
    case dup:
      a = pop(vm);
      push(vm, a);
      push(vm, a);
      break;
    case drop:
      pop(vm);
      break;
    case swap:
      b = pop(vm);
      a = pop(vm);
      push(vm, b);
      push(vm, a);
      break;
    case over:
      b = pop(vm);
      a = pop(vm);
      push(vm, a);
      push(vm, b);
      push(vm, a);
      break;
    case rot:
      c = pop(vm);
      b = pop(vm);
      a = pop(vm);
      push(vm, b);
      push(vm, c);
      push(vm, a);
      break;
    case nip:
      b = pop(vm);
      pop(vm);
      push(vm, b);
      break;

    case call:
      rpush(vm, vm->pc);
      vm->pc = pop(vm);
      break;
    case ccall:
      b = pop(vm);
      a = pop(vm);
      if(a) {
	rpush(vm, vm->pc);
	vm->pc = a;
      }
      break;
    case ret:
      vm->pc = rpop(vm);
      break;
    case cret:
      a = pop(vm);
      if(a)
	vm->pc = rpop(vm);
      break;
    
    case add:
      b = pop(vm);
      a = pop(vm);
      push(vm, a + b);
      break;
    case sub:
      b = pop(vm);
      a = pop(vm);
      push(vm, a - b);
      break;
    case mul:
      b = pop(vm);
      a = pop(vm);
      push(vm, a * b);
      break;
    case divmod:
      b = pop(vm);
      iassert(b != 0, vm, DIV_BY_ZERO);
      a = pop(vm);
      push(vm, a % b);
      push(vm, a / b);
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
      kor_interrupt(vm, INVALID_INSTRUCTION);
      break;
    }
  }
}
