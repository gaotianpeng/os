//
// Created by ziya on 22-6-23.
//

void kernel_main(void) {
    int a = 0;

    char* video = (char*)0xb8000;
    *video = 'G';
}