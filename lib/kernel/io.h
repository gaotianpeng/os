#ifndef __LIB_IO_H
#define __LIB_IO_H

#include "stdint.h"

// 向端口port写入一个字节
static inline void outb (uint16_t port, uint8_t data) {
   /*
      a表示用寄存器al或ax或eax，对端口指定N表示0~255, d表示用dx存储端口号, 
      %b0表示对应al，%w1表示对应dx
   */
	asm volatile ("outb %b0, %w1" : : "a" (data), "Nd" (port));
	/******************************************************/
	// 这里是 AT&T 语法的汇编语言，相当于：	mov al. data
	// 					mov dx, port
	// 					out dx, al
}

// 将addr处起始的word_cnt个字写入端口port
static inline void outsw(uint16_t port, const void* addr, uint32_t word_cnt) {  
    /*
        +表示此限制即做输入又做输出.
        outsw是把ds:esi处的16位的内容写入port端口，我们在设置段描述符时, 
        已经将ds、es、ss段的选择子都设置为相同的值了，此时不用担心数据错乱
    */
	asm volatile ("cld; rep outsw":"+S"(addr),"+c"(word_cnt):"d"(port));
	/*********************************************************/
	// 这里是 AT&T 语法的汇编语言，相当于：	cld
	// 					mov esi, addr
	// 					mov ecx, word_cnt
	// 					mov edx, port
	// 					rep outsw
}

// 将从端口port读入的一个字节返回
static inline uint8_t inb(uint16_t port){
	uint8_t data;
	asm volatile ("inb %w1,%b0": "=a"(data):"Nd"(port));
	// 这里是 AT&T 语法的汇编语言，相当于：	mov dx, port
	// 					in  al, dx
	// 					mov [data], al

	return data;
} 

// 将从端口port读入的word_cnt个字写入addr
static inline void insw(uint16_t port, void* addr, uint32_t word_cnt){
    /*
        insw是将从端口port处读入的16位内容写入es:edi指向的内存，
        我们在设置段描述符时，已经将ds,es,ss段的选择子都设置为相同的值了，
        此时不用担心数据错乱。
    */
	asm volatile ("cld; rep insw" : "+D" (addr), "+c" (word_cnt): "d"(port): "memory" );
	/********************************************************/
	// 这里是 AT&T 语法的汇编语言，相当于：	cld
	// 					mov edi, addr
	// 					mov ecx. word_cnt
	// 					mov dx, port
	// 					rep insw
}

#endif // __LIB_IO_H
