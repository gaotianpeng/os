[bits 32]
[section .text]

global in_byte
in_byte:
    push ebp;
    mov ebp, esp

    xor eax, eax
    ; port
    mov edx, [ebp + 8]
    in al, dx

    ; 函数结束前的清理工作
    leave            ; 等价于 mov esp, ebp  和  pop ebp
    ret              ; 返回调用者

global out_byte
out_byte:
    push ebp
    mov ebp, esp

    ; port
    mov edx, [ebp + 8]
    ; value
    mov eax, [ebp + 12]
    out dx, al

    leave
    ret

global in_word
in_word:
    push ebp
    mov ebp, esp

    xor eax, eax

    ; port
    mov edx, [ebp + 8];
    in ax, dx

    leave
    ret

global out_word
out_word:
    push ebp
    mov ebp, esp

    ; port
    mov edx, [ebp + 8]
    ; value
    mov eax, [ebp + 12]
    out dx, ax

    leave
    ret







