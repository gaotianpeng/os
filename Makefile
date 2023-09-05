BUILD:=./build

CFLAGS:= -m32 # 32 位的程序
CFLAGS+= -masm=intel
CFLAGS+= -fno-builtin	# 不需要 gcc 内置函数
CFLAGS+= -fno-stack-protector	# 不需要栈保护
CFLAGS:=$(strip ${CFLAGS})

DEBUG:= -g

HD_IMG_NAME:= "hd.img"

all: ${BUILD}/boot/mbr.o ${BUILD}/boot/loader.o ${BUILD}/kernel.bin
	$(shell rm -rf $(HD_IMG_NAME))
	bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $(HD_IMG_NAME)
	dd if=${BUILD}/boot/mbr.o of=hd.img bs=512 seek=0 count=1 conv=notrunc
	dd if=${BUILD}/boot/loader.o of=hd.img bs=512 seek=2 count=4 conv=notrunc
	dd if=${BUILD}/kernel.bin of=hd.img bs=512 seek=9 count=200 conv=notrunc

${BUILD}/boot/%.o: boot/%.asm
	$(shell mkdir -p ${BUILD}/boot)
	nasm -I boot/ $< -o $@

${BUILD}/lib/print.o: lib/kernel/print.asm
	$(shell mkdir -p ${BUILD}/lib)
	nasm -f elf32 $< -o $@

${BUILD}/kernel/kernel.o: kernel/kernel.asm
	$(shell mkdir -p ${BUILD}/kernel)
	nasm -f elf32 $< -o $@

${BUILD}/kernel.bin: ${BUILD}/kernel/main.o ${BUILD}/lib/print.o ${BUILD}/kernel/kernel.o \
	${BUILD}/kernel/init.o ${BUILD}/kernel/interrupt.o ${BUILD}/device/timer.o
	ld -m elf_i386 $^ -o $@ -Ttext 0xc0001500 -e main

${BUILD}/kernel/init.o: kernel/init.c
	$(shell mkdir -p ${BUILD}/kernel)
	gcc -m32 -I lib/ -fno-builtin  -fno-stack-protector  -c $< -o $@

${BUILD}/kernel/main.o: kernel/main.c
	$(shell mkdir -p ${BUILD}/kernel)
	gcc -m32 -I lib/ -fno-builtin  -fno-stack-protector  -c $< -o $@

${BUILD}/kernel/interrupt.o: kernel/interrupt.c
	$(shell mkdir -p ${BUILD}/kernel)
	gcc -m32 -I lib/ -fno-builtin  -fno-stack-protector  -c $< -o $@

${BUILD}/device/timer.o: device/timer.c
	$(shell mkdir -p ${BUILD}/device)
	gcc -m32 -I lib/ -fno-builtin  -fno-stack-protector  -c $< -o $@


clean:
	$(shell rm -rf ${BUILD})
	$(shell rm -rf bx_enh_dbg.ini)
	$(shell rm -rf hd.img)


bochs: all
	bochs -q -f bochsrc
