# CRITICAL: RISC-V Implementation Is Mostly Stubs

## Severity: HIGH

The initial RISC-V kernel implementation created file structure but left critical
functions as empty stubs. This means the kernel would fail immediately if executed.

## Evidence

### File Size Comparison (Lines of Code)

| File | i386 | RISC-V | Ratio | Status |
|------|------|--------|-------|--------|
| pmap.c | 2,122 | 220 | **10.6x smaller** | STUB |
| pcb.c | 1,277 | 107 | **11.9x smaller** | STUB |
| trap.c | 805 | 173 | **4.7x smaller** | PARTIAL |
| machdep.c | 333 | 62 | **5.4x smaller** | STUB |
| vm_machdep.c | 104 | 105 | Similar | STUB |
| **TOTAL** | **4,641** | **667** | **7.0x smaller** | **CRITICAL** |

## Critical Stub Functions

### 1. pcb.c - Thread/Process Control (107 lines total)

```c
void stack_attach(thread_t thread, vm_offset_t stack, void (*continuation)(void))
{
    thread->kernel_stack = stack;
    /* Set up stack pointer and continuation */
    /* This would set up RISC-V sp and ra registers */
}
```
**ISSUE**: No actual stack setup! Just assigns pointer.

```c
void switch_context(thread_t old, void (*continuation)(void), thread_t new)
{
    /* Save old thread state */
    /* Load new thread state */
    /* This would save/restore RISC-V registers */
}
```
**ISSUE**: COMPLETELY EMPTY! Context switching won't work at all!

```c
void pcb_init(thread_t thread)
{
    /* Initialize PCB for new thread */
}
```
**ISSUE**: Empty - new threads won't be initialized!

### 2. pmap.c - Memory Management (220 lines total)

```c
pmap_t pmap_create(vm_size_t size)
{
    /* Allocate and initialize a new pmap */
    return PMAP_NULL;
}
```
**ISSUE**: Always returns NULL - can't create address spaces!

```c
void pmap_enter(pmap_t pmap, vm_offset_t va, vm_offset_t pa, 
                vm_prot_t prot, boolean_t wired)
{
    /* Insert page table entry */
}
```
**ISSUE**: Empty - can't map any memory!

```c
void pmap_remove(pmap_t pmap, vm_offset_t sva, vm_offset_t eva)
{
    /* Remove page table entries */
}
```
**ISSUE**: Empty - can't unmap memory!

All pmap functions are empty stubs.

### 3. vm_machdep.c - VM Machine-Dependent (105 lines)

```c
void cpu_fork(struct proc *p1, struct proc *p2)
{
    /* Set up child process registers */
}
```
**ISSUE**: Empty - fork() will create broken child processes!

```c
void setregs(struct proc *p, u_long entry, u_long stack)
{
    /* Set up initial user registers */
}
```
**ISSUE**: Empty - exec() won't set up program entry point!

### 4. trap.c - Exception Handling (173 lines)

Actually has some code for exception mapping, BUT:
- No AST (Asynchronous System Trap) checking
- No signal delivery
- No syscall handling beyond basic exception dispatch

### 5. machdep.c - Machine-Dependent Core (62 lines)

Only implements:
- `cpu_type()` - returns CPU_TYPE_RISCV
- `cpu_subtype()` - returns CPU_SUBTYPE_RISCV64_G

Missing many critical machine-dependent functions that i386 has.

## Impact Assessment

### What Works: NOTHING

1. **Context Switching**: Broken (switch_context is empty)
2. **Memory Management**: Broken (all pmap_* functions empty)
3. **Process Creation**: Broken (cpu_fork empty)
4. **Program Execution**: Broken (setregs empty)
5. **Thread Creation**: Broken (pcb_init empty)

### Boot Prediction

The kernel would likely:
1. Start boot process
2. Try to create first thread → FAIL (pcb_init empty)
3. Try to set up page tables → FAIL (pmap_* empty)
4. **Immediate crash or hang**

## Critical Functions That MUST Be Implemented

### Priority 1 - Kernel Can't Boot Without These

1. **pmap_bootstrap()** - Set up initial page tables
2. **pmap_enter()** - Map kernel memory
3. **pcb_init()** - Initialize first thread
4. **switch_context()** - Actually implemented in locore.s, but needs C setup

### Priority 2 - Needed for Basic Functionality

5. **pmap_create()** - Create user address spaces
6. **pmap_destroy()** - Free address spaces
7. **pmap_remove()** - Unmap memory
8. **cpu_fork()** - Set up child process state
9. **setregs()** - Set up program entry point
10. **stack_attach()** - Properly set up kernel stacks

### Priority 3 - Needed for Full Functionality

11. **pmap_protect()** - Change memory permissions
12. **pmap_extract()** - Translate virtual to physical
13. **pmap_copy()** - Copy address space regions
14. **pmap_page_protect()** - Protect physical pages
15. **AST handling in trap.c**
16. **Signal delivery**
17. **Syscall entry/exit**

## Missing Additional Files

From i386 analysis, still missing:

1. **in_cksum.c** - TCP/IP checksum (needed for networking)
2. Possibly platform-specific timer/clock code
3. FPU save/restore (mentioned in PCB structure but not implemented)

## Recommendation

**IMMEDIATE ACTION REQUIRED**: Implement the core functions before this can be
considered a functional kernel port. The current state is a skeleton only.

Priority order:
1. Implement pmap.c completely (page table management)
2. Implement pcb.c completely (thread management)  
3. Complete trap.c (syscalls, signals, AST)
4. Implement vm_machdep.c (fork, exec support)
5. Enhance machdep.c with missing functions
6. Add in_cksum.c for networking

Estimated work: 2000-3000 additional lines of core kernel code needed.
