#include "print.h"
#include "init.h"
#include "debug.h"
#include "string.h"

int main(void) {
   char* str = "hello world\n";
   put_str(str);
   int len = strlen(str);
   put_int(len);
   put_char('\n');

   put_str("I am kernel\n");

   init_all();
   ASSERT(1==2);

   while (1);

   return 0;
}
