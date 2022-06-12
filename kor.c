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

void kor_push(kor *vm, u32 x)
{
  iassert(vm->wst.ptr < KOR_STACK_SIZE, vm, WST_OVERFLOW);
  vm->wst.dat[vm->wst.ptr++] = x;
}

#define PUSH(x) kor_push(vm, x)

u32 kor_pop(kor *vm)
{
  iassert(vm->wst.ptr > 0, vm, WST_UNDERFLOW);
  return vm->wst.dat[--vm->wst.ptr];
}

#define POP(x) do { x = kor_pop(vm); } while (0)

static void kor_rpush(kor *vm, u32 x)
{
  iassert(vm->rst.ptr < KOR_STACK_SIZE, vm, RST_OVERFLOW);
  vm->rst.dat[vm->rst.ptr++] = x;
}

#define RPUSH(x) kor_rpush(vm, x)

static u32 kor_rpop(kor *vm)
{
  iassert(vm->rst.ptr > 0, vm, RST_UNDERFLOW);
  return vm->rst.dat[--vm->rst.ptr];
}

#define RPOP(x) do { x = kor_rpop(vm); } while(0)

void kor_boot(kor *vm)
{
  u32 i;
  u8 *ptr = (u8*)vm;
  kassert(sizeof(u8) == 1, "sizeof(u8) should be 1");
  kassert(sizeof(u16) == 2, "sizeof(u16) should be 2");
  kassert(sizeof(u32) == 4, "sizeof(u32) should be 4");
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

#define store_short(x, a)					\
  do {								\
    iassert(a < KOR_MEM_SIZE - 1, vm, MEMORY_ACCESS_ERROR);	\
    vm->mem[a] = x & 0xff;					\
    vm->mem[a+1] = (x & 0xff00) >> 8;				\
  } while(0)

#define store_word(x, a)					\
  do {								\
    iassert(a < KOR_MEM_SIZE - 3, vm, MEMORY_ACCESS_ERROR);	\
    vm->mem[a] = x & 0xff;					\
    vm->mem[a+1] = (x & 0xff00) >> 8;				\
    vm->mem[a+2] = (x & 0xff0000) >> 16;			\
    vm->mem[a+3] = (x & 0xff000000) >> 24;			\
  } while(0)

void kor_exec(kor *vm, u32 limit)
{
  u8 opcode = nop, op = nop, operand_size, rel;
  u32 a, b, c;
  i32 sb;
  while(limit--) {
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
      PUSH(a);
      break;
    case dup:
      POP(a);
      PUSH(a);
      PUSH(a);
      break;
    case drop:
      POP(a);
      break;
    case swap:
      POP(b);
      POP(a);
      PUSH(b);
      PUSH(a);
      break;
    case over:
      POP(b);
      POP(a);
      PUSH(a);
      PUSH(b);
      PUSH(a);
      break;
    case rot:
      POP(c);
      POP(b);
      POP(a);
      PUSH(b);
      PUSH(c);
      PUSH(a);
      break;
    case nip:
      POP(b);
      POP(a);
      PUSH(b);
      break;

    case call:
      RPUSH(vm->pc);
      POP(a);
      if(rel) vm->pc += a;
      else vm->pc = a;
      break;
    case ccall:
      POP(b);
      POP(a);
      if(a) {
	RPUSH(vm->pc);
	if(rel) vm->pc += b;
	else vm->pc = b;
      }
      break;
    case ret:
      RPOP(vm->pc);
      break;
    case cret:
      POP(a);
      if(a) {
	RPOP(a);
	vm->pc = a;
      }
      break;
    case jmp:
      POP(a);
      if(rel) vm->pc += a;
      else vm->pc = a;
      break;
    case cjmp:
      POP(b);
      POP(a);
      if(a) {
	if(rel) vm->pc += b;
	else vm->pc = b;
      }
      break;
    case wtr:
      POP(a);
      RPUSH(a);
      break;
    case rtw:
      RPOP(a);
      PUSH(a);
      break;
      
    case eq:
      POP(b);
      POP(a);
      PUSH(a == b);
      break;
    case neq:
      POP(b);
      POP(a);
      PUSH(a != b);
      break;
    case lt:
      POP(b);
      POP(a);
      PUSH(a < b);
      break;
    case gt:
      POP(b);
      POP(a);
      PUSH(a > b);
      break;
    case and:
      POP(b);
      POP(a);
      PUSH(a & b);
      break;
    case or:
      POP(b);
      POP(a);
      PUSH(a | b);
      break;
    case xor:
      POP(b);
      POP(a);
      PUSH(a ^ b);
      break;
    case shift:
      POP(sb);
      POP(a);
      if(sb >= 0)
	PUSH(a << sb);
      else
	PUSH(a >> (-sb));
      break;
    
    case add:
      POP(b);
      POP(a);
      PUSH(a + b);
      break;
    case sub:
      POP(b);
      POP(a);
      PUSH(a - b);
      break;
    case mul:
      POP(b);
      POP(a);
      PUSH(a * b);
      break;
    case divmod:
      POP(b);
      iassert(b != 0, vm, DIV_BY_ZERO);
      POP(a);
      PUSH(a % b);
      PUSH(a / b);
      break;
    case sext:
      POP(b);
      if(operand_size == mode_byte) {
	if(b & 0x80)
	  b |= 0xffffff00;
      } else if (operand_size == mode_short) {
	if(b & 0x8000)
	  b |= 0xffff0000;
      }
      else kor_interrupt(vm, INVALID_INSTRUCTION);
      PUSH(b);
      break;
    case trap:
      POP(b);
      switch(b) {
      case 0:
	kor_halt(vm);
	break;
      case 1:
	kor_putc(vm);
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

void kor_run(kor *vm)
{
  while(1)
    kor_exec(vm, 0xffffffff);
}
