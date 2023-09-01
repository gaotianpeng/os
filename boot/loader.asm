   %include "boot.inc"
   section loader vstart=LOADER_BASE_ADDR
   LOADER_STACK_TOP  equ LOADER_BASE_ADDR

; 构建GDT及其内部的描述符
   GDT_BASE:         dd    0x00000000
                     dd    0x00000000
   
   CODE_DESC:        dd    0x0000FFFF
                     dd    DESC_CODE_HIGH4
   
   DATA_STACK_DESC:  dd    0x0000FFFF
                     dd    DESC_DATA_HIGH4

   VIDEO_DESC:       dd    0x80000007        ; limit = (0xbffff-0xb8000/4k=0x7
                     dd    DESC_VIDEO_HIGH4 ; DPL为0

   GDT_SIZE          equ   $ - GDT_BASE
   GDT_LIMIT         equ   GDT_SIZE - 1
   times 60 dq 0                             ; 预留60个描述符空位

   SELECTOR_CODE     equ   (0x0001<<3) + TI_GDT + RPL0   ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
   SELECTOR_DATA     equ   (0x0002<<3) + TI_GDT + RPL0
   SELECTOR_VIDEO    equ   (0x0003<<3) + TI_GDT + RPL0

   ; total_mem_bytes用于保存内存容量, 以字节为单位
   ; 0xb00 是total_mem_bytes 加载到内存的地址
   total_mem_bytes   dd    0

   ; 定义gdt，前2字节是gdt界限，后4字节是gdt起始地址
   gdt_ptr     dw       GDT_LIMIT
               dd       GDT_BASE

   ; 人工对齐: (total_mem_bytes 4字节) + (gdt_ptr 6字节) + (ards_buf 244字节) + (ards_nr 2字节) 共256字节
   ards_buf times 244 db 0
   ards_nr dw 0		         ; 用于记录ards结构体数量


; ----------------------------------------------------------------
; int 15h  eax = 0000E820h, edx = 534D4150h ('SMAP') 获取内存布局
; ----------------------------------------------------------------
   loader_start:
      xor ebx, ebx                  ; 第一次调用时，ebx值要为0
      mov edx, 0x534d4150           ; edx 只赋值一次，循环体中不会改变
      mov di, ards_buf              ; ards 结构缓冲区
   .e820_mem_get_loop:              ; 循环获取每个ARDS内存范围描述结构
      mov eax, 0x0000e820           ; 执行int 0x15后，eax值变为0x534d4150, 所以，每次执行前都要更新为子功能号
      mov ecx, 20                   ; ADRS 地址范围描述符结构大小是20字节
      int 0x15
      jc .e820_failed_so_try_e801   ; 若cf位为1则有错误发生，尝试把0xe801子功能、
      and di, cx                    ; 使di增加20字节指向缓冲区中新的ADRS结构位置
      inc word [ards_nr]            ; 记录ADRS数量
      cmp ebx, 0                    ; 若ebx为0，且cf不为1，这说明adrs全部返回，当前已经是最后一个
      jnz .e820_mem_get_loop

   ; 在所有adrs结构中，找出(base_add_low + length_low) 的最大值，即内存的容量
      mov cx, [ards_nr]
      mov ebx, ards_buf
      xor edx, edx                  ; edx为最大的内存容量，在此先清0
   .find_max_mem_area:              ; 无须判断type是否为1，最大的内存块一定是可被使用
      mov eax, [ebx]                ; base_addr_low
      add eax, [ebx + 8]            ; length_low
      add ebx, 20                   ; 指向缓冲区中下一个ADRS结构
      cmp edx, eax                  ; 冒泡排序，找出最大值，edx寄存器始终是最大的内容容量
      jge .next_adrs
      mov edx, eax                  ; edx 为总内存大小

   .next_adrs:
      loop .find_max_mem_area
      jmp .mem_get_ok

   ;------  int 15h ax = E801h 获取内存大小,最大支持4G  ------
   ; 返回后, ax cx 值一样,以KB为单位,bx dx值一样,以64KB为单位
   ; 在ax和cx寄存器中为低16M,在bx和dx寄存器中为16MB到4G。
   .e820_failed_so_try_e801:
      mov ax,0xe801
      int 0x15
      jc .e801_failed_so_try88      ; 若当前e801方法失败,就尝试0x88方法

   ;1 先算出低15M的内存, ax和cx中是以KB为单位的内存数量, 将其转换为以byte为单位
      mov cx,0x400	               ; cx和ax值一样,cx用做乘数
      mul cx 
      shl edx,16
      and eax,0x0000FFFF
      or edx,eax
      add edx, 0x100000             ; ax只是15MB,故要加1MB
      mov esi,edx	                  ; 先把低15MB的内存容量存入esi寄存器备份

   ;2 再将16MB以上的内存转换为byte为单位,寄存器bx和dx中是以64KB为单位的内存数量
      xor eax,eax
      mov ax,bx		
      mov ecx, 0x10000	            ; 0x10000十进制为64KB
      mul ecx		                  ; 32位乘法,默认的被乘数是eax,积为64位,高32位存入edx,低32位存入eax.
      add esi,eax		               ; 由于此方法只能测出4G以内的内存,故32位eax足够了,edx肯定为0,只加eax便可
      mov edx,esi		               ; edx为总内存大小
      jmp .mem_get_ok

   ;-----------------  int 15h ah = 0x88 获取内存大小,只能获取64M之内  ----------
   .e801_failed_so_try88: 
      ; int 15后，ax存入的是以kb为单位的内存容量
      mov  ah, 0x88
      int  0x15
      jc .error_hlt
      and eax,0x0000FFFF
         
      ; 16位乘法，被乘数是ax,积为32位.积的高16位在dx中，积的低16位在ax中
      mov cx, 0x400                 ; 0x400等于1024,将ax中的内存容量换为以byte为单位
      mul cx
      shl edx, 16	                  ; 把dx移到高16位
      or edx, eax	                  ; 把积的低16位组合到edx,为32位的积
      add edx,0x100000              ; 0x88子功能只会返回1MB以上的内存,故实际内存大小要加上1MB

   .mem_get_ok:
      mov [total_mem_bytes], edx	   ; 将内存换为byte单位后存入total_mem_bytes处。
      

; ---------------------- 准备进入保护模式 -----------------------
   
   ; 打开 A20
   in al, 0x92
   or al, 0000_0010B
   out 0x92, al

   ; 加载GDT
   lgdt [gdt_ptr]

   ; cr0 第0位置1
   mov eax, cr0
   or eax, 0x00000001
   mov cr0, eax

   jmp dword SELECTOR_CODE:p_mode_start

.error_hlt:
   hlt

[bits 32]
p_mode_start:
   mov ax, SELECTOR_DATA
   mov ds, ax
   mov es, ax
   mov ss, ax
   mov esp, LOADER_STACK_TOP
   mov ax, SELECTOR_VIDEO
   mov gs, ax

   mov byte [gs:160], 'P'

   jmp $






