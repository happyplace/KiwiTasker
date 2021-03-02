section .text
bits 64

;export get_context function
global get_context

get_context:
			; Save the return address and stack pointer.
			mov qword r8, [rsp]
			mov qword 8*0[rcx], r8 ; RIP
			lea qword r8, 8[rsp]
			mov qword 8*1[rcx], r8 ; RSP

			; Save preserved registers.

			mov qword 8*2[rcx], rbx
			mov qword 8*3[rcx], rbp
			mov qword 8*4[rcx], r12
			mov qword 8*5[rcx], r13
			mov qword 8*6[rcx], r14
			mov qword 8*7[rcx], r15
			mov qword 8*8[rcx], rdi
			mov qword 8*9[rcx], rsi

			movups 8*10+16*0[rcx], xmm6
			movups 8*10+16*1[rcx], xmm7
			movups 8*10+16*2[rcx], xmm8
			movups 8*10+16*3[rcx], xmm9
			movups 8*10+16*4[rcx], xmm10
			movups 8*10+16*5[rcx], xmm11
			movups 8*10+16*6[rcx], xmm12
			movups 8*10+16*7[rcx], xmm13
			movups 8*10+16*8[rcx], xmm14
			movups 8*10+16*9[rcx], xmm15

			; Return.
			ret			

;export set_context function
global set_context

set_context:
			; Should return to the address set with {get, swap}_context.
			mov qword r8, 8*0[rcx]

			; Load new stack pointer.
			mov qword rsp, 8*1[rcx]

			; Load preserved registers.
			mov qword rbx, 8*2[rcx]
			mov qword rbp, 8*3[rcx]
			mov qword r12, 8*4[rcx]
			mov qword r13, 8*5[rcx]
			mov qword r14, 8*6[rcx]
			mov qword r15, 8*7[rcx]
			mov qword rdi, 8*8[rcx]
			mov qword rsi, 8*9[rcx]

			movups xmm6, 8*10+16*0[rcx] 
			movups xmm7, 8*10+16*1[rcx]
			movups xmm8, 8*10+16*2[rcx]
			movups xmm9, 8*10+16*3[rcx]
			movups xmm10, 8*10+16*4[rcx]
			movups xmm11, 8*10+16*5[rcx]
			movups xmm12, 8*10+16*6[rcx]
			movups xmm13, 8*10+16*7[rcx]
			movups xmm14, 8*10+16*8[rcx]
			movups xmm15, 8*10+16*9[rcx]

			; Push RIP to stack for RET.
			push r8

			; Load pointer for second parameter
			mov qword rdx, 8*11+16*10[rcx]
			
			; Load pointer for third parameter
			mov qword r8, 8*12+16*10[rcx]

			; The last thing we do is load the first parameter because we overwrite rcx
			mov qword rcx, 8*10+16*10[rcx]

			; Return.
			ret	

;export swap_context function
global swap_context

swap_context:
			; Save the return address.
			mov qword r8, [rsp]
			mov qword 8*0[rcx], r8 ; RIP
			lea qword r8, 8[rsp]
			mov qword 8*1[rcx], r8 ; RSP

			; Save preserved registers.
			mov qword 8*2[rcx], rdx
			mov qword 8*3[rcx], rbp
			mov qword 8*4[rcx], rbp
			mov qword 8*5[rcx], r13
			mov qword 8*6[rcx], r14
			mov qword 8*7[rcx], r15
			mov qword 8*8[rcx], rdi
			mov qword 8*9[rcx], rsi

			movups 8*10+16*0[rcx], xmm6
			movups 8*10+16*1[rcx], xmm7
			movups 8*10+16*2[rcx], xmm8
			movups 8*10+16*3[rcx], xmm9
			movups 8*10+16*4[rcx], xmm10
			movups 8*10+16*5[rcx], xmm11
			movups 8*10+16*6[rcx], xmm12
			movups 8*10+16*7[rcx], xmm13
			movups 8*10+16*8[rcx], xmm14
			movups 8*10+16*9[rcx], xmm15

			; Should return to the address set with {get, swap}_context.
			mov qword r8, 8*0[rdx]

			; Load new stack pointer.
			mov qword rsp, 8*1[rdx]

			; Load preserved registers.
			mov qword rbx, 8*2[rdx]
			mov qword rbp, 8*3[rdx]
			mov qword r12, 8*4[rdx]
			mov qword r13, 8*5[rdx]
			mov qword r14, 8*6[rdx]
			mov qword r15, 8*7[rdx]
			mov qword rdi, 8*8[rdx]
			mov qword rsi, 8*9[rdx]

			movups xmm6, 8*10+16*0[rdx] 
			movups xmm7, 8*10+16*1[rdx]
			movups xmm8, 8*10+16*2[rdx]
			movups xmm9, 8*10+16*3[rdx]
			movups xmm10, 8*10+16*4[rdx]
			movups xmm11, 8*10+16*5[rdx]
			movups xmm12, 8*10+16*6[rdx]
			movups xmm13, 8*10+16*7[rdx]
			movups xmm14, 8*10+16*8[rdx]
			movups xmm15, 8*10+16*9[rdx]

			; Push RIP to stack for RET.
			push qword r8

			; we load the first parameter now because it's we've already preserved the registers in the first context
			mov qword rcx, 8*10+16*10[rdx]

			; Load pointer for third parameter
			mov qword r8, 8*12+16*10[rdx]

			; Load pointer for second parameter last because that's what were using to load the new context
			mov qword rdx, 8*11+16*10[rdx]

			; Return.
			ret
