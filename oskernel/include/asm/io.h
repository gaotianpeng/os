//
// Created by ziya on 22-6-26.
//

#ifndef ZIYA_OSKERNEL_TEACH_IO_H
#define ZIYA_OSKERNEL_TEACH_IO_H

char in_byte(int port);
short in_word(int port);

void out_byte(int port, int v);
void out_word(int port, int v);

#endif //ZIYA_OSKERNEL_TEACH_IO_H
