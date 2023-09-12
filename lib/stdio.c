#include "stdio.h"
#include "interrupt.h"
#include "global.h"
#include "string.h"
#include "syscall.h"
#include "print.h"

#define va_start(ap, v) ap = (va_list)&v  // 把ap指向第一个固定参数v
#define va_arg(ap, t) *((t*)(ap += 4))	   // ap指向下一个参数并返回其值
#define va_end(ap) ap = NULL		         // 清除ap

static void itoa(uint32_t value, char** buf_ptr_addr, uint8_t base) {
   uint32_t m = value % base;
   uint32_t i = value / base;  

   if (!i) {
      return;
   }

   if (i) {
      itoa(i, buf_ptr_addr, base);
   }

   if (m < 10) {
      *((*buf_ptr_addr)++) = m + '0';
   } else {
      *((*buf_ptr_addr)++) = m - 10 + 'A';
   }
}


uint32_t vsprintf(char* str, const char* format, va_list ap) {
   char* buf_ptr = str;
   const char* index_ptr = format;
   char index_char = *index_ptr;
   int32_t arg_int;
   while (index_char) {
      if (index_char != '%') {
         *(buf_ptr++) = index_char;
         index_char = *(++index_ptr);
         continue;
      }
      index_char = *(++index_ptr);	 // 得到%后面的字符
      switch(index_char) {
         case 'x':
            arg_int = va_arg(ap, int);
            itoa(arg_int, &buf_ptr, 16); 
            index_char = *(++index_ptr); // 跳过格式字符并更新index_char
            break;
      }
   }

   return strlen(str);
}

uint32_t sprintf(char* buf, const char* format, ...) {
   va_list args;
   uint32_t retval;
   va_start(args, format);
   retval = vsprintf(buf, format, args);
   va_end(args);

   return retval;
}

uint32_t printf(const char* format, ...) {
   va_list args;
   va_start(args, format);
   char buf[1024] = {0};
   vsprintf(buf, format, args);
   va_end(args);

   return write(buf);
}
