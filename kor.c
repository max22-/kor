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
  kor_fatal(); /* there are only error handling exceptions for the moment */
}

#define fetch_byte(x, a)						\
  do {									\
    iassert(a < KOR_MEM_SIZE, vm, MEMORY_ACCESS_ERROR);			\
    x = vm->mem[a];							\
  }									\
  while(0)

#define fetch_short(x, a)						\
  do {									\
    iassert(a < KOR_MEM_SIZE - 1, vm, MEMORY_ACCESS_ERROR);		\
    x = vm->mem[a];							\
    x |= vm->mem[a+1] << 8;						\
  } while(0)

#define fetch_word(x, a)						\
  do {									\
    iassert(a < KOR_MEM_SIZE - 3, vm, MEMORY_ACCESS_ERROR);		\
    x = vm->mem[a];							\
    x |= vm->mem[a+1] << 8;						\
    x |= vm->mem[a+2] << 16;						\
    x |= vm->mem[a+3] << 24;						\
  } while(0)

#define store_byte(x, a)					\
  do {								\
    iassert(a < KOR_MEM_SIZE, vm, MEMORY_ACCESS_ERROR);		\
    vm->mem[a] = x;						\
  } while(0)

#define store_short(x, a)				\
  do {							\
    iassert(a < KOR_MEM_SIZE - 1, vm, MEMORY_ACCESS_ERROR);	\
    vm->mem[a] = x & 0xff;				\
    vm->mem[a+1] = (x & 0xff00) >> 8;			\
  } while(0)

#define store_word(x, a)					\
  do {								\
    iassert(a < KOR_MEM_SIZE - 3, vm, MEMORY_ACCESS_ERROR);	\
    vm->mem[a] = x & 0xff;					\
    vm->mem[a+1] = (x & 0xff00) >> 8;				\
    vm->mem[a+2] = (x & 0xff0000) >> 16;			\
    vm->mem[a+3] = (x & 0xff000000) >> 24;			\
  } while(0)

void kor_start(kor *vm)
{
  u8 opcode = nop, op = nop, operand_size, rel;
  u32 a, b, c;
  i32 sb; /* signed b */
  while(1) {
    fetch_byte(opcode, vm->pc);
    vm->pc++;
    op = opcode & 0x1f;
    operand_size = opcode & 0x60;
    rel = opcode & 0x80;
    switch(op) {
    case nop:
      break;
    case lit:
      if(operand_size == mode_byte) {
	fetch_byte(a, vm->pc);
	vm->pc++;
      }
      else if(operand_size == mode_short) {
	fetch_short(a, vm->pc);
	vm->pc += 2;
      }
      else if(operand_size == mode_word) {
	fetch_word(a, vm->pc);
	vm->pc += 4;
      }
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
    case jmp:
      vm->pc = pop(vm);
      break;
    case cjmp:
      b = pop(vm);
      a = pop(vm);
      if(a)
	vm->pc = b;
      break;
    case wtr:
      rpush(vm, pop(vm));
      break;
    case rtw:
      push(vm, rpop(vm));
      break;
      
    case eq:
      b = pop(vm);
      a = pop(vm);
      push(vm, a == b);
      break;
    case neq:
      b = pop(vm);
      a = pop(vm);
      push(vm, a != b);
      break;
    case lt:
      b = pop(vm);
      a = pop(vm);
      push(vm, a < b);
      break;
    case gt:
      b = pop(vm);
      a = pop(vm);
      push(vm, a > b);
      break;
    case and:
      b = pop(vm);
      a = pop(vm);
      push(vm, a & b);
      break;
    case or:
      b = pop(vm);
      a = pop(vm);
      push(vm, a | b);
      break;
    case xor:
      b = pop(vm);
      a = pop(vm);
      push(vm, a ^ b);
      break;
    case shift:
      sb = pop(vm);
      a = pop(vm);
      if(sb >= 0)
	push(vm, a << sb);
      else
	push(vm, a >> (-sb));
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
    case sext:
      b = pop(vm);
      if(operand_size == mode_byte) {
	if(b & 0x80)
	  b |= 0xffffff00;
      } else if (operand_size == mode_short) {
	if(b & 0x8000)
	  b |= 0xffff0000;
      }
      else kor_interrupt(vm, INVALID_INSTRUCTION);
      push(vm, b);
      break;
    case trap:
      b = pop(vm);
      switch(b) {
      case 0:
	a = pop(vm);
	kor_halt(a);
	break;
      case 1:
	a = pop(vm);
	kor_putc(a);
	break;
      default:
	kor_interrupt(vm, INVALID_TRAP);
	break;
      }
      break;
    default:
      kor_interrupt(vm, INVALID_INSTRUCTION);
      break;
    }
  }
}
