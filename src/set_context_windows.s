.type set_context, @function
.global set_context
set_context:
    # Should return to the address set with {get, swap}_context.
    movq 8*0(%rcx), %r9

    # Load new stack pointer.
    movq 8*1(%rcx), %rsp

    # Load preserved registers.
    movq 8*2(%rcx), %rbx
    movq 8*3(%rcx), %rbp
    movq 8*4(%rcx), %r12
    movq 8*5(%rcx), %r13
    movq 8*6(%rcx), %r14
    movq 8*7(%rcx), %r15
    movq 8*8(%rcx), %rdi
    movq 8*9(%rcx), %rsi
    movups 8*10+16*0(%rcx), %xmm6
    movups 8*10+16*1(%rcx), %xmm7
    movups 8*10+16*2(%rcx), %xmm8
    movups 8*10+16*3(%rcx), %xmm9
    movups 8*10+16*4(%rcx), %xmm10
    movups 8*10+16*5(%rcx), %xmm11
    movups 8*10+16*6(%rcx), %xmm12
    movups 8*10+16*7(%rcx), %xmm13
    movups 8*10+16*8(%rcx), %xmm14
    movups 8*10+16*9(%rcx), %xmm15

    # Push RIP to stack for RET.
    pushq %r9

    # Load pointer for second parameter
    movq 8*9(%rcx), %rdx

    # Load pointer for third parameter
    movq 8*0xa(%rcx), %r8

    # The last thing we do is load the first parameter because we overwrite rdi
    movq 8*8(%rcx), %rcx

    # Return.
    xorl %eax, %eax
    ret
