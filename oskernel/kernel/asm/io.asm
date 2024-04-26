[bits 32]
[SECTION .text]

global in_byte
global out_byte
global in_word
global out_word

in_byte:
    push ebp
    mov ebp, esp

    xor eax, eax

    mov edx, [ebp + 8]
    in al, dx

    leave  ; mov esp, ebp; pop ebp
    ret

out_byte:
    push ebp
    mov ebp, esp

    mov edx, [ebp + 8]
    mov eax, [ebp + 12]
    out dx, al

    leave
    ret

in_word:
    push ebp
    mov ebp, esp

    xor eax, eax

    mov edx, [ebp + 8]
    in ax, dx

    leave
    ret

out_word:
    push ebp
    mov ebp, esp

    mov edx, [ebp + 8]
    mov eax, [ebp + 12]
    out dx, ax

    leave
    ret

