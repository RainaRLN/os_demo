[SECTION .text]
[bits 32]

global in_byte
global out_byte
global in_word
global out_word

in_byte:
    push ebp
    mov ebp, esp

    xor eax, eax

    mov edx, edi  ; port
    in al, dx

    leave  ; mov esp, ebp; pop ebp
    ret

out_byte:
    push ebp
    mov ebp, esp

    mov edx, edi  ; port
    mov eax, esi  ; value
    out dx, al

    leave
    ret

in_word:
    push ebp
    mov ebp, esp

    xor eax, eax

    mov edx, edi  ; port
    in ax, dx

    leave
    ret

out_word:
    push ebp
    mov ebp, esp

    mov edx, edi  ; port
    mov eax, esi  ; value
    out dx, ax

    leave
    ret

