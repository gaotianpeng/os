section mbr vstart=0x7c00
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00
    mov ax, 0xb800
    mov gs, ax


; 清屏，利用0x06号功能，上卷全部行来清屏
; int 0x10  功能号:0x06, 功能描述：上卷窗口
; 输入：
; AH 功能号= 0x06
; AL = 上卷的行数(如果为0,表示全部)
; BH = 上卷行属性
; (CL,CH) = 窗口左上角的(X,Y)位置
; (DL,DH) = 窗口右下角的(X,Y)位置
; 无返回值
    xor ax, ax
    mov ah, 0x06
    xor bx, bx
    mov bh, 0x07
    mov cx, 0            ; 左上角: (0, 0)
    mov dx, 0x184f       ; 右下角: (80,25),
    
    int 0x10

; 获取光标位置
    mov ah, 3           ; 功能号3是获取光标位置
    mov bh, 0           ; bh存储待获取光标的页号(屏幕的页数)
    int 0x10            ; 输出 dh = 光标所在行号，dl光标所在列号
    
; 打印字符串
    ; es:bp 为串首地址
    mov ax, message
    mov bp, ax

    mov cx, 5           ; 串长度
    mov ah, 0x13        ; 功能号是0x13,字符及属性存入ah寄存器
    mov al, 0x01        ; al设置写字符方式，01 显示字符串，光标跟随移动
    mov bh, 0x00        ; bh存储要显示的页号,此处是第0页,
    mov bl, 0x02        ; bl中是字符属性, 属性黑底绿字(bl = 02h)
    int 0x10

; 输出背景色绿色，前景色红色，并且跳动的字符串"1 MBR"
    mov byte [gs:0x00], '1'
    mov byte [gs:0x01], 0xA4    ; A表示绿色背景闪烁，4表示前景色为红色

    mov byte [gs:0x02], ' '
    mov byte [gs:0x03], 0xA4

    mov byte [gs:0x04], 'M'
    mov byte [gs:0x05], 0xA4

    mov byte [gs:0x06], 'B'
    mov byte [gs:0x07], 0xA4

    mov byte [gs:0x08], 'R'
    mov byte [gs:0x09], 0xA4

    jmp $

message db '1 MBR'

times 510 - ($-$$) db 0x00
db 0x55, 0xaa