%include "boot.inc"
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

    mov eax, LOADER_START_SECTOR    ; 起始扇区lba地址
    mov bx, LOADER_BASE_ADDR        ; 写入的地址
    mov cx, 1                       ; 待读入的扇区数
    call rd_disk_m

    jmp LOADER_BASE_ADDR

; ---------------------------------------------------------
; 功能：读取硬盘n个扇区
;   eax = lba 扇区号
;   ebx = 将数据写入的内存地址
;   ecx = 读入的扇区数
; ---------------------------------------------------------
rd_disk_m:
        ; 备份 eax, cx
        mov esi, eax    
        mov di, cx

    ; 开始读写硬盘
    ; step 1: 设置要读取的扇区数
        mov dx, 0x1f2
        mov al, CL
        out dx, al

        mov eax, esi


    ; step 2: 将LBA地址存入 0x1f3 ~ 0x1f6
        ; lba 地址 7~0位写入端口 0x1f3
        mov dx, 0x1f3
        out dx, al
        
        ; lba 地址的15~8位写入0x1f4
        mov cl, 8
        shr eax, cl
        mov dx, 0x1f4
        out dx, al

        ; lba 地址23~16位写入端口0x1f5
        shr eax,cl
        mov dx,0x1f5
        out dx,al

        shr eax,cl
        and al,0x0f	   ; lba第24~27位
        or al,0xe0	   ; 设置7～4位为1110,表示lba模式
        mov dx,0x1f6
        out dx,al

    ; step 3: 向0x1f7端口写入读命令，0x20 
        mov dx, 0x1f7
        mov al, 0x20
        out dx, al
    
    ; step 4: 检测硬盘状态
    .not_ready:
        ; 同一端口，写时表示写入命令字，读时表示读入硬盘状态
        nop
        in al,dx
        and al,0x88	    ; 第4位为1表示硬盘控制器已准备好数据传输，第7位为1表示硬盘忙
        cmp al,0x08
        jnz .not_ready  ; 若未准备好，继续等

    ; step 5：从0x1f0端口读数据
        mov ax, di
        mov dx, 256
        mul dx
        mov cx, ax	    ; di为要读取的扇区数，一个扇区有512字节，每次读入一个字，
                        ; 共需di*512/2次，所以di*256
        mov dx, 0x1f0 
    .go_on_read:
        in ax,dx
        mov [bx],ax
        add bx,2		  
        loop .go_on_read
        ret

    times 510 - ($-$$) db 0x00
    db 0x55, 0xaa