/* User provided functions */
void kor_putc(char);
void kor_print(const char*);
void kor_eprint(const char*);
void kor_fatal();
/* *********************** */

#define KOR_MEM_SIZE 65536
#define KOR_STACK_SIZE 256
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;

typedef struct {
  u32 dat[KOR_STACK_SIZE];
  i32 ptr;
} kor_stack;

typedef struct {
  u8 mem[KOR_MEM_SIZE];
  u32 pc;
  kor_stack wst, rst;
} kor;

#define KOR_OPCODES					\
  X(nop)					\
    X(lit)					\
    X(dup)					\
    X(drop)					\
    X(swap)					\
    X(over)					\
    X(rot)					\
    X(nip)					\
    X(call)					\
    X(ccall)					\
    X(ret)					\
    X(cret)					\
    X(jmp)					\
    X(cjmp)					\
    X(wtr)					\
    X(rtw)					\
    X(eq)					\
    X(neq)					\
    X(lt)					\
    X(gt)					\
    X(and)					\
    X(or)					\
    X(xor)					\
    X(shift)					\
    X(add)					\
    X(sub)					\
    X(mul)					\
    X(divmod)					\
  X(fetch)					\
  X(store)					\
  X(trap)					\
  X(halt)

enum kor_opcodes {
#define X(x) x,
  KOR_OPCODES
#undef X
  KOR_OPCODES_COUNT
};

enum kor_modes {
  mode_byte = 0, mode_short = 1<<5, mode_word = 1<<6,
  mode_relative = 1 << 7
};

#define KOR_INTERRUPTS				\
  X(DIV_BY_ZERO)				\
    X(WST_UNDERFLOW)				\
    X(WST_OVERFLOW)				\
    X(RST_UNDERFLOW)				\
    X(RST_OVERFLOW)				\
    X(MEMORY_ACCESS_ERROR)			\
    X(INVALID_INSTRUCTION)
    
enum kor_interrupts {
#define X(x) x,
  KOR_INTERRUPTS
#undef X
  INTERRUPTS_COUNT
};

void kor_boot(kor*);
void kor_load(kor *, u8 *src, u32 size);
void kor_interrupt(kor *vm, int n);
void kor_start(kor*);
