%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR
    LOADER_STACK_TOP        equ     LOADER_BASE_ADDR
    jmp loader_start

; 构建gdt及其内部的描述符
    GDT_BASE:           dd  0x00000000      ; 低4字节：低2字节是段界限的0～15位，高2字节是段基址的0～15位
                        dd  0x00000000      ; 高4字节
    CODE_DESC:          dd  0x0000FFFF
                        dd  DESC_CODE_HIGH4
    DATA_STACK_DESC:    dd  0x0000FFFF
                        dd  DESC_DATA_HIGH4
    VIDEO_DESC:         dd  0x80000007
                        dd  DESC_VIDEO_HIGH4
    GDT_SIZE        equ $ - GDT_BASE
    GDT_LIMIT       equ GDT_SIZE - 1
    times 60 dq 0
    SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0    ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
    SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0	 
    SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

    gdt_ptr     dw  GDT_LIMIT
                dd  GDT_BASE
    loadermsg db '2 loader in real.'

loader_start:
    ; 打印字符，"2 LOADER"说明loader已经成功加载
    ; 输出背景色绿色，前景色红色，并且跳动的字符串"1 MBR"
    mov byte [gs:160],'2'
    mov byte [gs:161],0xA4     ; A表示绿色背景闪烁，4表示前景色为红色

    mov byte [gs:162],' '
    mov byte [gs:163],0xA4

    mov byte [gs:164],'L'
    mov byte [gs:165],0xA4   

    mov byte [gs:166],'O'
    mov byte [gs:167],0xA4

    mov byte [gs:168],'A'
    mov byte [gs:169],0xA4

    mov byte [gs:170],'D'
    mov byte [gs:171],0xA4

    mov byte [gs:172],'E'
    mov byte [gs:173],0xA4

    mov byte [gs:174],'R'
    mov byte [gs:175],0xA4


; ----------------------------------------
;  int 0x10        功能号 0x13, 打印字符串
; in:
;   ah = 0x13
;   bh = 页码
;   bl = 属性(若al=00h或01h)
;   cx = 字符串长度
;   (dh, dl) = 光标行、列
;   es:bp = 字符串地址
;   al = 显示输出方式
;   0——字符串中只含显示字符，其显示属性在BL中。显示后，光标位置不变
;   1——字符串中只含显示字符，其显示属性在BL中。显示后，光标位置改变
;   2——字符串中含显示字符和显示属性。显示后，光标位置不变
;   3——字符串中含显示字符和显示属性。显示后，光标位置改变
    mov	 sp, LOADER_BASE_ADDR
    mov	 bp, loadermsg                              ; es:bp = 字符串地址
    mov	 cx, loader_start - loadermsg			    ; cx = 字符串长度
    mov	 ax, 0x1301		                            ; ah = 13,  al = 01h
    mov	 bx, 0x001f		                            ; 页号为0(bh = 0) 蓝底粉红字(bl = 1fh)
    mov	 dx, 0x1800		                            ;
    int	 0x10    

    ; 打开A20
    in al, 0x92
    or al, 0000_0010B
    out 0x92, al

    ; 加载GDT
    lgdt [gdt_ptr]

    ; cr0第0位置1
    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax

    jmp  SELECTOR_CODE:p_mode_start	     ; 刷新流水线，避免分支预测的影响

[bits 32]
p_mode_start:
   xchg cx, cx
   mov ax, SELECTOR_DATA
   mov ds, ax
   mov es, ax
   mov ss, ax
   mov esp, LOADER_STACK_TOP
   mov ax, SELECTOR_VIDEO
   mov gs, ax

   mov byte [gs:320], 'P'
   mov byte [gs:322], 'r'
   mov byte [gs:324], 'o'
   mov byte [gs:326], 't'
   mov byte [gs:328], 'e'
   mov byte [gs:330], 'c'
   mov byte [gs:332], 't'

   jmp $


    
