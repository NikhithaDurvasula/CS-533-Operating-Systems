# void thread_start(struct thread * old, struct thread * new);
.globl thread_start

thread_start:
pushq %rbx
pushq %rbp
pushq %r12
pushq %r13
pushq %r14
pushq %r15
movq %rsp,(%rdi)
movq (%rsi),%rsp
jmp thread_wrap


#void_switch(struct thread * old, struct thread * new);
.globl thread_switch

thread_switch:
 pushq %rbx
 pushq %rbp
 pushq %r12
 pushq %r13
 pushq %r14
 pushq %r15
 movq %rsp, (%rdi)
 movq (%rsi), %rsp
 popq %r15
 popq %r14
 popq %r13
 popq %r12
 popq %rbp
 popq %rbx
ret

