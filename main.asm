section .text
    global _start

_start:

    xor rbp, rbp
    and rsp, -16
    push rbp
    mov rbp, rsp
    sub rsp, 16

    ; ioperm(0x60, 1, 1)
    mov rax, 173         ; syscall number for ioperm (x86_64)
    mov rdi, 0x60        ; from port
    mov rsi, 1           ; num ports
    mov rdx, 1           ; enable
    syscall

    ; baca dari keyboard (port 0x60)
    mov dx, 0x60         ; port 0x60
    in al, dx            ; baca 1 byte dari keyboard
    ; hasil scan code dari keyboard masuk ke AL

    ; keluar program dengan return code = scan code
    movzx rdi, al        ; pindahkan ke rdi (exit code)
    mov rax, 60          ; syscall: exit
    syscall
