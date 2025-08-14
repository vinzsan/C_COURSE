%define EAGAIN 11
%define EINT 4
%define MAX_STACK_SHARED 0x40000

%define PROT_WRITE 0x02
%define PROT_READ 0x01

CLONE_MAX equ 0x00000100 | 0x00000200 | 0x00000400 | 0x00000800 | 0x00010000 | 0x00040000 | 0x01000000 | 0x00200000

FUTEX_WAIT equ 0
FUTEX_WAKE equ 1
FUTEX_PRIVATE_FLAG equ 0x80

MAX_BUFFER equ 4096 * 4
MAP_PRIVATE equ 0x02
MAP_ANONYMOUS equ 0x20
MAP_GROWSDOWN equ 0x100

MAP_SHARED equ 0x01
MAP_STACK equ 0x20000

section .rodata
    L1 db "[INFO] : Main/Parents stream",0x0A,0
    E1 equ $ - L1
    L2 db "[INFO] : Thread clone mutex initializer (futex)",0x0A,0
    E2 equ $ - L2
    L3 db "Warning! Race condition and undifined behavior ret addr",0x0A,0
    E3 equ $ - L3
    L4 db "Error allocated memory",0x0A,0
    E4 equ $ - L4
    
    timeval:
        dd 1
        dd 0

section .bss
    futex_hand resd 1
    futex_shared_memory_flags resd 2
    child_pid resq 8
    pid resq 8
    
section .text
    global _start

_start:

    xor rbp,rbp
    and rsp,0xFFFFFFFFFFFFFFFF
    sub rsp,128
    push rbx
    push rdx
    push rcx

    mov dword [rsp],handler_sigsegv
    mov qword [rsp + 8], 0x10000000 | 0x04000000
    mov dword [rsp + 12],restorer
    mov qword [rsp + 128],0

    mov rbx,rsp
    
    mov rax,13
    mov rdi,11
    lea rsi,[rbx]
    xor rdx,rdx
    mov r10,8
    syscall

    pop rbx
    
    push rbp
    mov rbp,rsp
    sub rsp,64
    
    push rbx

    mov rax,9
    xor rdi,rdi
    mov rsi,MAX_STACK_SHARED
    mov rdx,PROT_READ | PROT_WRITE
    mov r10,MAP_SHARED | MAP_ANONYMOUS
    mov r8,-1
    xor r9,r9
    syscall
    
    mov rbx,rax
    cmp rax,-1
    je error_alloc
    
    lea rax,[rbx]
    mov [rbp - 8],rax
    
    lea rdi,[rbx]
    lea rsi,[rel L1]
    call STRCPY

    lea rdi,[rbx + 64]
    lea rsi,[rel L2]
    call STRCPY

    lea rdi,[rbx]
    call PRINT
    
    lea rdi,[rbx + 64]
    call PRINT
    
    mov dword [rbx + 72],0
    mov dword [rbx + 80],1
    
    mov rax,9
    xor rdi,rdi
    mov rsi,MAX_BUFFER
    mov rdx,PROT_READ | PROT_WRITE
    mov r10,MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN
    mov r8,-1
    xor r9,r9
    syscall
    
    push r12
    mov r12,rax
    mov r13,rax
    cmp rax,-1
    je error_alloc

    mov rax,9
    xor rdi,rdi
    mov rsi,MAX_STACK_SHARED
    mov rdx,PROT_READ | PROT_WRITE
    mov r10,MAP_SHARED | MAP_ANONYMOUS
    mov r8,-1
    xor r9,r9
    syscall

    push r15
    mov r15,rax
    cmp rax,-1
    je error_alloc
    
    ;mov dword [r15],0
    ;mov dword [r15 + 8],1
    mfence
    
    mov rax,56
    mov rdi,CLONE_MAX
    lea rsi,[r13 + MAX_STACK_SHARED - 8]
    ;and rsi,0xFFFFFFFFFFFFFFF0
    mov rdx,child_pid
    lea r10,[child_pid]
    syscall

    mov [pid],rax

    cmp rax,0
    jz THREAD
    mov r12,rax

JOIN:

    
EXIT_GLOBAL:
    
    leave

    mov rax,11
    mov rdi,rbx
    mov rsi,MAX_STACK_SHARED
    syscall

    mov dword [r15],0
    
    mov rax,202
    lea rdi,[r15]
    mov esi,FUTEX_WAIT | FUTEX_PRIVATE_FLAG
    mov edx,0
    xor r10,r10
    syscall

    mfence

    mov rax,11
    mov rdi,r13
    mov rsi,MAX_BUFFER
    syscall

    mov rax,11
    mov rdi,r15
    mov rsi,MAX_STACK_SHARED
    syscall

    mov rax,60
    xor rdi,rdi
    syscall
    
;-----------------------

error_alloc:
    
    mov rax,1
    mov rdi,1
    lea rsi,[rel L4]
    mov rdx,E4
    syscall

    leave
    
    mov rax,60
    mov rdi,-1
    syscall

handler_sigsegv:
    
    mov rax,1
    mov rdi,1
    lea rsi,[rel L3]
    mov rdx,E3
    syscall
    
    mov rax,60
    mov rdi,-1
    syscall

restorer:
    
    mov rax,15
    syscall
    
ITOA:

    push rbp
    mov rbp,rsp
    sub rsp,64

    ;push rax
    push rcx
    push rdx
    
    mov rax,rdi
    lea rsi,[rbp - 8]
    mov rcx,10

.itoa:
    
    xor rdx,rdx
    div rcx
    add dl,'0'
    dec rsi
    mov [rsi],dl
    inc rcx

    test al,al
    jnz .itoa
    
    mov rax,rsi
    pop rdx
    pop rcx
    leave
    ret
    
STRCPY:
    
    push rbp
    mov rbp,rsp
    
.strcpy:
    
    mov al,[rsi]
    mov [rdi],al
    test al,al
    jz .done
    inc rsi
    inc rdi
    jmp .strcpy

.done:

    mov rax,rdi
    leave
    ret

STRLEN:

    push rbp
    mov rbp,rsp
    
    push rcx
    push rdx
    
    lea rdx,[rdi]

    xor rcx,rcx
    
.strlen:
    
    mov al,[rdx]
    test al,al
    jz .done
    inc rdx
    inc rcx
    jmp .strlen
    
.done:

    mov rax,rcx
    pop rdx
    pop rcx
    leave
    ret

PRINT:
    
    push rbp
    mov rbp,rsp
    
    push rcx
    mov rcx,rdi
    call STRLEN

    mov rdx,rax

    mov rax,1
    mov rdi,1
    mov rsi,rcx
    syscall
    
    mov rax,rdx
    pop rcx
    leave
    ret

;--------------------
    
THREAD:
    
    push rbp
    mov rbp,rsp
    
    lea rdi,[rel L2]
    call PRINT

    
    mov dword [r15],1
    mfence
    
    mov rax,202
    lea rdi,[r15]
    mov esi,FUTEX_WAKE | FUTEX_PRIVATE_FLAG
    mov edx,1
    xor r10,r10
    syscall
    leave
    
    mov rax,60
    xor rdi,rdi
    syscall
