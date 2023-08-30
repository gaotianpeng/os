; 0柱面0磁道1扇區
[ORG  0x7c00]

[SECTION .data]
BOOT_MAIN_ADDR equ 0x500

[SECTION .text]
[BITS 16]
global _start
_start:
    mov ax, 0x03
    int 0x10

    xor ax, ax
    mov ax, 0xb800
    mov es, ax
    mov byte [es:0x00],'L'
    mov byte [es:0x01],0x07

    jmp $

times 510 - ($ - $$) db 0
db 0x55, 0xaa