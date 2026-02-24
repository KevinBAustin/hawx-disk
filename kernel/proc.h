#ifndef PROC_H
#define PROC_H
#include "types.h"
#include "riscv.h"

// Saved registers for kernel context switches.
struct context {
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

// CPU state.
struct cpu {
  struct proc *proc;      // The process running on this cpu, or null.
  struct context context; // swtch() here to enter scheduler().
};

// per-process data for the trap handling code in trampoline.S.
// sits in a page by itself just under the trampoline page in the
// user page table. not specially mapped in the kernel page table.
// the sscratch register points here.
// uservec in trampoline.S saves user registers in the trapframe,
// then initializes registers from the trapframe's
// kernel_sp, kernel_hartid, kernel_satp, and jumps to kernel_trap.
// usertrapret() and userret in trampoline.S set up
// the trapframe's kernel_*, restore user registers from the
// trapframe, switch to the user page table, and enter user space.
// the trapframe includes callee-saved user registers like s0-s11 because the
// return-to-user path via usertrapret() doesn't return through
// the entire kernel call stack.
struct trapframe {
  /*   0 */ uint64 kernel_satp;   // kernel page table
  /*   8 */ uint64 kernel_sp;     // top of process's kernel stack
  /*  16 */ uint64 kernel_trap;   // usertrap()
  /*  24 */ uint64 epc;           // saved user program counter
  /*  32 */ uint64 kernel_hartid; // saved kernel tp
  /*  40 */ uint64 ra;
  /*  48 */ uint64 sp;
  /*  56 */ uint64 gp;
  /*  64 */ uint64 tp;
  /*  72 */ uint64 t0;
  /*  80 */ uint64 t1;
  /*  88 */ uint64 t2;
  /*  96 */ uint64 s0;
  /* 104 */ uint64 s1;
  /* 112 */ uint64 a0;
  /* 120 */ uint64 a1;
  /* 128 */ uint64 a2;
  /* 136 */ uint64 a3;
  /* 144 */ uint64 a4;
  /* 152 */ uint64 a5;
  /* 160 */ uint64 a6;
  /* 168 */ uint64 a7;
  /* 176 */ uint64 s2;
  /* 184 */ uint64 s3;
  /* 192 */ uint64 s4;
  /* 200 */ uint64 s5;
  /* 208 */ uint64 s6;
  /* 216 */ uint64 s7;
  /* 224 */ uint64 s8;
  /* 232 */ uint64 s9;
  /* 240 */ uint64 s10;
  /* 248 */ uint64 s11;
  /* 256 */ uint64 t3;
  /* 264 */ uint64 t4;
  /* 272 */ uint64 t5;
  /* 280 */ uint64 t6;
};

enum procstate { UNUSED, USED, WAITING, RUNNABLE, RUNNING };

// Per-process state
struct proc {
  enum procstate state; // Process state
  int wait_read;        // If non-zero, waiting for a port read
  int wait_write;       // If non-zero, waiting for a port write
  int pid;              // Process ID

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // Virtual address of kernel stack
  uint64 sz;                   // Size of process memory (bytes)
  pagetable_t pagetable;       // User page table
  struct trapframe *trapframe; // data page for trampoline.S
  struct context context;      // swtch() here to run process
};

// global proc variables
#define NPROC 64
extern struct cpu cpu;
extern struct proc proc[];

///////////////////////////////////////////////////////////////////////////////
// Proc API
///////////////////////////////////////////////////////////////////////////////

/*
 * Initialize the proc table, and allocate a page for each process's
 * kernel stack. Map the stacks in high memory, followed by an invalid guard
 * page. Parameters:
 *  - None
 * Returns:
 *  - None
 */
void proc_init(void);

/*
 * Set up the first user process. Return the process it was allocated to.
 * Parameters:
 *  - None
 * Returns:
 *  - A pointer to the allocated proc (struct proc*)
 */
struct proc *proc_load_user_init(void);

/*
 * Look in the process table for an UNUSED proc.
 * If found, initialize state required to run in the kernel.
 * If there are no free procs, or a memory allocation fails, return 0.
 * Parameters:
 *  - None
 * Returns:
 *  - A pointer to the new process (struct proc*)
 *  - Zero (0) if no free proc is available or on allocation failure
 */
struct proc *proc_alloc(void);

/*
 * Free a proc structure and the data hanging from it,
 * including user pages.
 * Parameters:
 *  - p: A pointer to the proc structure (struct proc*)
 * Returns:
 *  - None
 */
void proc_free(struct proc *p);

/*
 * Load the ELF program image stored in the binary string bin
 * into the specified process. This operation will destroy the
 * pagetable currently in p, and replace it with a page table
 * as indicated by the segments of the elf formatted binary.
 * Parameters:
 *  - p: A pointer to the proc structure (struct proc*)
 *  - bin: A pointer to the binary string containing the ELF binary
 * Returns:
 *  - Zero (0) on success
 *  - Minus one (-1) on failure.
 */
int proc_load_elf(struct proc *p, void *bin);

/*
 * Resize the process so that it occupies newsz bytes of memory.
 * If newsz > oldsz, Allocate PTEs and physical memory to grow process from
 * oldsz to newsz. If newsz < oldsz, Use proc_shrink to decrease the size of the
 * process. newsz need not be page aligned.  Returns new size or 0 on error.
 * Parameters:
 *  - pagetable: The current page table
 *  - oldsz: Current size of the process
 *  - newsz: Desired size of the process
 * Returns:
 *  - New size of the process or zero (0) on error
 */
uint64 proc_resize(pagetable_t pagetable, uint64 oldsz, uint64 newsz);

/*
 * Given a parent process's page table, copy its memory into a
 * child's page table. Copies both the page table and the physical 
 * memory. 
 * Parameters:
 * - old: The parent process's page table
 * - new: The child process's page table
 * - sz: The size of the process
 * Returns:
 * - Zero (0) on success
 * - Minus one (-1) on failure
 */
int proc_vmcopy(pagetable_t old, pagetable_t new, uint64 sz);


/* 
 * Find the process with the given pid and return a pointer to it.
 * If the process is not found, return 0
 * Parameters:
 * - pid: The id of the process to find.
 */
struct proc *proc_find(int pid); 


#endif
