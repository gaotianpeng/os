#include "keyboard.h"
#include "print.h"
#include "interrupt.h"
#include "io.h"
#include "global.h"
#include "ioqueue.h"

/*
    8042 输入和输出缓冲区寄存器的端口
*/
#define KBD_BUF_PORT    0X60

/*
    定义控制键的ASCII码，由于它们都是不可见的，都用转义字符定义
    esc和delete没有一般转义字符的形式，只能考虑八进制或十六进制形式
*/ 
#define esc		    '\033'	        // 八进制表示字符，也可以用十六进制'\x1b'
#define backspace	'\b'
#define tab		    '\t'
#define enter		'\r'
#define delete		'\177'	        // 八进制表示字符，十六进制为'\x7f'

/*
    定义操作控制键的"ASCII"码，这类键是没有ASCII的，在控制键中只有字符控制键才有ASCII码
    一律定义为0
*/
#define char_invisible	    0
#define ctrl_l_char	        char_invisible
#define ctrl_r_char	        char_invisible
#define shift_l_char	    char_invisible
#define shift_r_char	    char_invisible
#define alt_l_char	        char_invisible
#define alt_r_char	        char_invisible
#define caps_lock_char	    char_invisible

/* 
    定义操作控制键的通码和断码
    操作控制键与其他键配合时是先被按下的，
    因此，每次在接收一个按键时，需要查看上一次是否有按下相关的操作控制键
    也就是将操作控制键的当前状态记录在某个全局变量中
*/
#define shift_l_make	    0x2a
#define shift_r_make 	    0x36 
#define alt_l_make   	    0x38
#define alt_r_make   	    0xe038
#define alt_r_break   	    0xe0b8
#define ctrl_l_make  	    0x1d
#define ctrl_r_make  	    0xe01d
#define ctrl_r_break 	    0xe09d
#define caps_lock_make 	    0x3a

struct ioqueue kbd_buf;

/* 
    操作控制键是与其他键配合时是先被按下的，因此，每次在接收一下按键时，需要查看上一次是
    否有按下盯关的操作控制键

    定义以下变量记录相应键是否按下的状态,
    ext_scancode用于记录makecode是否以0xe0开头
*/
static bool ctrl_status, shift_status, alt_status, caps_lock_status, ext_scancode;

/* 
    此数组定义了与shift组合时的字符效果
    以通码make_code为索引的二维数组
*/
static char keymap[][2] = {
/* 扫描码   未与shift组合  与shift组合*/
/* ---------------------------------- */
/* 0x00 */	{0,	0},		
/* 0x01 */	{esc,	esc},		
/* 0x02 */	{'1',	'!'},		
/* 0x03 */	{'2',	'@'},		
/* 0x04 */	{'3',	'#'},		
/* 0x05 */	{'4',	'$'},		
/* 0x06 */	{'5',	'%'},		
/* 0x07 */	{'6',	'^'},		
/* 0x08 */	{'7',	'&'},		
/* 0x09 */	{'8',	'*'},		
/* 0x0A */	{'9',	'('},		
/* 0x0B */	{'0',	')'},		
/* 0x0C */	{'-',	'_'},		
/* 0x0D */	{'=',	'+'},		
/* 0x0E */	{backspace, backspace},	
/* 0x0F */	{tab,	tab},		
/* 0x10 */	{'q',	'Q'},		
/* 0x11 */	{'w',	'W'},		
/* 0x12 */	{'e',	'E'},		
/* 0x13 */	{'r',	'R'},		
/* 0x14 */	{'t',	'T'},		
/* 0x15 */	{'y',	'Y'},		
/* 0x16 */	{'u',	'U'},		
/* 0x17 */	{'i',	'I'},		
/* 0x18 */	{'o',	'O'},		
/* 0x19 */	{'p',	'P'},		
/* 0x1A */	{'[',	'{'},		
/* 0x1B */	{']',	'}'},		
/* 0x1C */	{enter,  enter},
/* 0x1D */	{ctrl_l_char, ctrl_l_char},
/* 0x1E */	{'a',	'A'},		
/* 0x1F */	{'s',	'S'},		
/* 0x20 */	{'d',	'D'},		
/* 0x21 */	{'f',	'F'},		
/* 0x22 */	{'g',	'G'},		
/* 0x23 */	{'h',	'H'},		
/* 0x24 */	{'j',	'J'},		
/* 0x25 */	{'k',	'K'},		
/* 0x26 */	{'l',	'L'},		
/* 0x27 */	{';',	':'},		
/* 0x28 */	{'\'',	'"'},		
/* 0x29 */	{'`',	'~'},		
/* 0x2A */	{shift_l_char, shift_l_char},	
/* 0x2B */	{'\\',	'|'},		
/* 0x2C */	{'z',	'Z'},		
/* 0x2D */	{'x',	'X'},		
/* 0x2E */	{'c',	'C'},		
/* 0x2F */	{'v',	'V'},		
/* 0x30 */	{'b',	'B'},		
/* 0x31 */	{'n',	'N'},		
/* 0x32 */	{'m',	'M'},		
/* 0x33 */	{',',	'<'},		
/* 0x34 */	{'.',	'>'},		
/* 0x35 */	{'/',	'?'},
/* 0x36	*/	{shift_r_char, shift_r_char},	
/* 0x37 */	{'*',	'*'},    	
/* 0x38 */	{alt_l_char, alt_l_char},
/* 0x39 */	{' ',	' '},		
/* 0x3A */	{caps_lock_char, caps_lock_char}
/*其它按键暂不处理*/
};

/*
    键盘中断处理程序，始终是每次处理一个字节，当扫描码中是多字节时，或者有组合键时，要定义额外的
    全局变量来记录它们曾经被按下过
*/
static void intr_keyboard_handler(void) {
    // 这次中断发生前的上一次中断，以下任意三个键是否有按下
    bool ctrl_down_last = ctrl_status;
    bool shift_down_last = shift_status;
    bool caps_lock_last = caps_lock_status;

    // 必须要读取输出缓冲区寄存器，否则8042不再继续响应键盘中断
    uint8_t break_code;
    uint16_t scan_code = inb(KBD_BUF_PORT);

    /*
        若扫描码是e0开头的，表示此键的按下将产生多个扫描码，
        所以马上结束此次中断处理函数，等待下一个扫描码进来
    */
    if (scan_code == 0xe0) {
        ext_scancode = true;    // 打开e0标记
    }

    // 如果上次是以0xe0开头，将扫描码合并
    if (ext_scancode) {
        scan_code = ((0xe000) | scan_code);
        ext_scancode = false;   // 关闭e0标记
    }

    /*
        判断扫描码是否为断码，断码第8位为1
        用扫描码scancode和0x0080进行位与操作，此时bread_code的值为true或false
     */
    break_code = ((scan_code & 0x0080) != 0);

    /* 
        判断此次按键(扫描码)对应的字符是什么
        通码，则是keymap中的索引，断码，则需要先将其还原为通码，然后检索keymap
    */
    if (break_code) {
        /* 
            由于ctrl_r 和alt_r的make_code和break_code都是两字节，
            所以可用下面的方法取make_code，多字节的扫描码暂不处理 
        */
        uint16_t make_code = (scan_code &= 0xff7f);
        
        if (make_code == ctrl_l_make || make_code == ctrl_r_make) {
            ctrl_status = false;
        } else if (make_code == shift_l_make || make_code == shift_r_make) {
            shift_status = false;
        } else if (make_code == alt_l_make || make_code == alt_r_make) {
            alt_status = false;
        }

        return;
    /*
        根据通码和shift键是否按下的情况，在keymap中找到按键对应的字符
    */
    } else if ((scan_code > 0x00 && scan_code < 0x3b) || 
            (scan_code == alt_r_make) || 
            (scan_code == ctrl_r_make)) {
        bool shift = false;
        if ((scan_code < 0x0e) || (scan_code == 0x29) || 
                (scan_code == 0x1a) || (scan_code == 0x1b) || 
                (scan_code == 0x2b) || (scan_code == 0x27) ||
                (scan_code == 0x28) || (scan_code == 0x33) ||
                (scan_code == 0x34) || (scan_code == 0x35)) {
            /****** 代表两个字母的键 ********
		     0x0e 数字'0'~'9',字符'-',字符'='
		     0x29 字符'`'
		     0x1a 字符'['
		     0x1b 字符']'
		     0x2b 字符'\\'
		     0x27 字符';'
		     0x28 字符'\''
		     0x33 字符','
		     0x34 字符'.'
		     0x35 字符'/' 
	        *******************************/
            if (shift_down_last) {
                shift = true;
            }
        } else {
        	if (shift_down_last && caps_lock_last) {  // 如果shift和capslock同时按下
                shift = false;
            } else if (shift_down_last || caps_lock_last) { // 如果shift和capslock任意被按下
                shift = true;
            } else {
                shift = false;
            }
        }

        uint8_t index = (scan_code &= 0x00ff);  // 将扫描码的高字节置0，主要是针对高字节是e0的扫描码
        char cur_char = keymap[index][shift];  // 在数组中找到对应的字符

        // 只处理ascii码不为0的键
        if (cur_char) {
            /*****************  快捷键ctrl+l和ctrl+u的处理 *********************
             * 下面是把ctrl+l和ctrl+u这两种组合键产生的字符置为:
             * cur_char的asc码-字符a的asc码，此差值比较小，
             * 属于asc码表中不可见的字符部分。故不会产生可见字符。
             * 我们在shell中将ascii值为l-a和u-a的分别处理为清屏和删除输入的快捷键
             */
            if ((ctrl_down_last && cur_char == 'l') || (ctrl_down_last && cur_char == 'u')) {
                cur_char -= 'a';
            }

            if (!ioq_full(&kbd_buf)) {
                ioq_putchar(&kbd_buf, cur_char);
            }
            return;
        }

        // 记录本次是否按下了下面几类控制键之一，供下次键入时判断组合键
        if (scan_code == ctrl_l_make || scan_code == ctrl_r_make) {
            ctrl_status = true;
        } else if (scan_code == shift_l_make || scan_code == shift_r_make) {
            shift_status = true;
        } else if (scan_code == alt_l_make || scan_code == alt_r_make) {
            alt_status = true;
        } else if (scan_code == caps_lock_make) {
            /* 
                不管之前是否有按下caps_lock键，当再次按下时则状态取反，
                即:已经开启时，再按下同样的键是关闭。关闭时按下表示开启。
            */
            caps_lock_status = !caps_lock_status;
        }
    } else {
        put_str("unknown key\n");
    }

    return;
}

// 键盘初始化
void keyboard_init() {
    put_str("keyboard init start\n");
    // ioqueue_init(&kbd_buf);
    ioqueue_init(&kbd_buf);

    register_handler(0x21, intr_keyboard_handler);
    put_str("keyboard init donw\n");
}