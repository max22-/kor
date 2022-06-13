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
  X(op_nop)						\
    X(op_lit)						\
    X(op_dup)						\
    X(op_drop)						\
    X(op_swap)						\
    X(op_over)						\
    X(op_rot)						\
    X(op_nip)						\
    X(op_call)						\
    X(op_ccall)						\
    X(op_ret)						\
    X(op_cret)						\
    X(op_jmp)						\
    X(op_cjmp)						\
    X(op_wtr)						\
    X(op_rtw)						\
    X(op_eq)						\
    X(op_neq)						\
    X(op_lt)						\
    X(op_gt)						\
    X(op_and)						\
    X(op_or)						\
    X(op_xor)						\
  X(op_shift)						\
  X(op_add)						\
  X(op_sub)						\
  X(op_mul)						\
  X(op_divmod)						\
  X(op_fetch)						\
  X(op_store)						\
  X(op_sext)						\
  X(op_trap)					

enum kor_opcodes {
#define X(x) x,
  KOR_OPCODES
#undef X
  KOR_OPCODES_COUNT
};

enum kor_mode {
  mode_byte = 1 << 5, mode_short = 1 << 6, mode_word = 0,
  mode_relative = 1 << 7
};

#define KOR_INTERRUPTS				\
  X(DIV_BY_ZERO)				\
    X(WST_UNDERFLOW)				\
    X(WST_OVERFLOW)				\
    X(RST_UNDERFLOW)				\
    X(RST_OVERFLOW)				\
    X(MEMORY_ACCESS_ERROR)			\
    X(INVALID_INSTRUCTION)			\
    X(INVALID_TRAP)
    
enum kor_interrupts {
#define X(x) x,
  KOR_INTERRUPTS
#undef X
  INTERRUPTS_COUNT
};

void kor_push(kor*, u32);
u32 kor_pop(kor*);
void kor_boot(kor*);
void kor_load_from_mem(kor *, u8 *src, u32 size);
void kor_interrupt(kor *vm, int n);
void kor_exec(kor*, u32 limit);
void kor_run(kor*);

/* User provided functions */
void kor_putc(kor*);
void kor_halt(kor*);
void kor_print(const char*);
void kor_eprint(const char*);
void kor_fatal();
/* *********************** */
