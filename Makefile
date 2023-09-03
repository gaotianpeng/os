BUILD:=./build

CFLAGS:= -m32 # 32 位的程序
CFLAGS+= -masm=intel
CFLAGS+= -fno-builtin	# 不需要 gcc 内置函数
CFLAGS+= -nostdinc		# 不需要标准头文件
CFLAGS+= -fno-pic		# 不需要位置无关的代码  position independent code
CFLAGS+= -fno-pie		# 不需要位置无关的可执行程序 position independent executable
CFLAGS+= -nostdlib		# 不需要标准库
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


${BUILD}/kernel.bin:  ${BUILD}/kernel/main.o
	ld -m elf_i386 $^ -o $@ -Ttext 0xc0001500 -e main
	# ld out/kernel/main.o -Ttext 0xc0001500 -e main -o out/kernel/kernel.bin 

clean:
	$(shell rm -rf ${BUILD})
	$(shell rm -rf bx_enh_dbg.ini)
	$(shell rm -rf hd.img)

${BUILD}/kernel/%.o: kernel/%.c
	$(shell mkdir -p ${BUILD}/kernel)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@


bochs: all
	bochs -q -f bochsrc

qemu: all
	qemu-system-x86_64 -hda hd.img
