section .data
    L1 db "Heap initializer",0x0A,0
    E1 equ $ - L1
    
    L2 db "Thread initializer",0x0A,0
    E2 equ $ - L2

    ctid dd 0

    FUTEX_WAKE equ 1
    FUTEX_PRIVATE_FLAG equ 128
    FUTEX_WAIT equ 0

    timeval dd 1

section .bss
    flag resd 1
    it resd 8

section .text
    global _start

_start:

    xor rbp,rbp
    and rsp,-16
    push rbp
    mov rbp,rsp
    sub rsp,64
    
    mov rax,9
    xor rdi,rdi
    mov rsi,4096 * 4
    mov rdx,0x02
    mov r10,0x22
    mov r8,-1
    xor r9,r9
    syscall
    
    push rbx
    mov rbx,rax

    mov rdi,0x00000100 | 0x00000200 | 0x00000300 | 0x00000400 | 0x00000800 | 0x00010000 | 0x00040000 | 0x001000000 | 0x00200000
    mov rax,56
    lea rsi,[rbx + (4096 * 4) - 8]
    mov qword [rsi],0
    mov rdx,ctid
    lea r10,[rel ctid]
    syscall

    mov ecx,eax

    test rax,rax
    jz thread

    mov dword [it],10

for1:

    mov eax,[it]
    cmp eax,0
    je .done1

    mov rax,1
    mov rdi,1
    lea rsi,[rel L1]
    mov rdx,E1
    syscall

    dec dword [it]
    call sleep
    jmp for1

.done1:

    mov dword [flag],0
    mfence

mutex_lock_initializer:
    
    mfence
    mov eax,[flag]
    cmp eax,1
    jne thread_check

    jmp exit

thread_check:
    
    mov rax,202
    lea rdi,[rel flag]
    mov esi,FUTEX_WAIT
    mov edx,0
    xor r10,r10
    xor r8,r8
    xor r9,r9
    syscall
    

    jmp mutex_lock_initializer

exit:

    mov rax,11
    mov rdi,rbx
    mov rsi,4096 * 4
    syscall

    pop rbx
    leave

    mov rax,231
    xor rdi,rdi
    syscall

thread:

    push rbp
    mov rbp,rsp

    mov r8d,10

for:

    cmp r8d,0
    jz .done2
    mov rax,1
    mov rdi,1
    lea rsi,[rel L2]
    mov rdx,E2
    syscall

    dec r8d
    call sleep
    jmp for

.done2:

    mov dword [flag],1
    mfence

    mov rax,202
    lea rdi,[rel flag]
    mov esi,FUTEX_WAKE
    mov edx,1
    xor r10,r10
    syscall

    mov rax,60
    xor rdi,rdi
    syscall

sleep:

    push rbp
    mov rbp,rsp

    mov rax,35
    lea rdi,[timeval]
    xor rsi,rsi
    syscall

    leave
    ret
