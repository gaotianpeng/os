#include "string.h"
#include "assert.h"

// 将dst起始的size个字节置为value
void memset(void* dst, uint8_t value, uint32_t size) {
    assert(dst != NULL);
    uint8_t* dst_ = (uint8_t*)dst;
    while (size-- > 0) {
        *dst_++ = value;
    }
}

// 将src起始的size个字节复制到dst
void memcpy(void* dst, const void* src, uint32_t size) {
   assert(dst != NULL && src != NULL);
   assert(dst != src);
   uint8_t* dst_ = dst;
   const uint8_t* src_ = src;
   while (size-- > 0) {
      *dst_++ = *src_++;
   }
}

// 连续比较以地址a和地址b开头的size个字节，若相等则返回0，若a大于b返回+1，否则返回-1
int memcmp(const void* a, const void* b, uint32_t size) {
   const char* a_ = a;
   const char* b_ = b;
   assert(a_ != NULL || b_ != NULL);

   while (size-- > 0) {
      if (*a_ != *b_) {
	      return *a_ > *b_ ? 1 : -1; 
      }
      a_++;
      b_++;
   }

   return 0;
}

// 将字符串从src复制到dst
char* strcpy(char* dst, const char* src) {
   assert(dst != NULL && src != NULL);
   assert(dst != src);

   char* ret = dst;		       // 用来返回目的字符串起始地址
   while ((*dst++ = *src++));

   return ret;
}

// 返回字符串长度
uint32_t strlen(const char* str) {
   assert(str != NULL);
   const char* p = str;
   while (*p++);

   return (p - str - 1);
}

// 比较两个字符串，若a中的字符大于b中的字符返回1，相等时返回0，否则返回-1
int8_t strcmp (const char* a, const char* b) {
   assert(a != NULL && b != NULL);
   while (*a != 0 && *a == *b) {
      a++;
      b++;
   }

   return *a < *b ? -1 : *a > *b;
}

// 从左到右查找字符串str中首次出现字符ch的地址
char* strchr(const char* str, const uint8_t ch) {
   assert(str != NULL);
   while (*str != 0) {
      if (*str == ch) {
	      return (char*)str;
      }
      str++;
   }
   return NULL;
}

// 从后往前查找字符串str中首次出现字符ch的地址
char* strrchr(const char* str, const uint8_t ch) {
   assert(str != NULL);
   const char* last_char = NULL;
   while (*str != 0) {
      if (*str == ch) {
	      last_char = str;
      }
      str++;
   }
   return (char*)last_char;
}

// 将字符串src拼接到dst后，将回拼接的串地址
char* strcat(char* dst, const char* src) {
   assert(dst != NULL && src != NULL);
   assert(dst != src);

   char* str = dst;
   while (*str++);
   --str;

   while ((*str++ = *src++));	 // 当*str被赋值为0时,此时表达式不成立,正好添加了字符串结尾的0

   return dst;
}

// 在字符串str中查找指定字符ch出现的次数
uint32_t strchrs(const char* str, uint8_t ch) {
   assert(str != NULL);
   uint32_t ch_cnt = 0;
   const char* p = str;
   while (*p != 0) {
      if (*p == ch) {
	      ch_cnt++;
      }
      p++;
   }
   return ch_cnt;
}
