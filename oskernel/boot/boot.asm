[ORG 0X7C00]

[SECTION .text]
[BITS 16]
global _start
_start:
    ; 设置屏幕模式为文本模式，并清除屏幕
    mov ax, 3
    int 0x10

    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov si, ax

    ; 设置前景色和背景色
    xor ax, ax
    mov ah, 0x0B   ; AH = 0x0B (Function to set background and foreground colors)
    mov bh, 0x00   ; BH = 0x00 (Background color, in this case, black)
    mov bl, 0x01   ; BL = 0x0E (Foreground color, in this case, blue)
    int 0x10
    ; 调置字体
    xor ax, ax
    mov ah, 0x11   ; AH = 0x11 (Function to set the font)
    mov al, 0x14   ; AL = 0x14 (Font number, in this case, 8x14 font)
    int 0x10       ; Call BIOS video interrupt to set the font

    mov si, msg     ; 传入字符串
    call print

    jmp $

print:
    ; ah 代表欲调用的功能
    mov ah, 0x0e    ; e显示字符，光标随字符移动(al字符，bl前景色)
    mov bh, 0x00
    mov bl, 0x01
.loop
    mov al, [si]
    cmp al, 0
    jz .done        ; 遇到0打印结束
    int 0x10        ; 打印字符并移动光标

    inc si
    jmp .loop
.done
    ret

msg:
    db "hello world!", 10, 13, 0

times 510 - ($ - $$) db 0x00
db 0x55,0xaa
