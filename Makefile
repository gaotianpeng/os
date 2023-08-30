BUILD:=./build

HD_IMG_NAME:= "hd.img"

all: ${BUILD}/boot/boot.o
	$(shell rm -rf $(HD_IMG_NAME))
	bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $(HD_IMG_NAME)
	dd if=${BUILD}/boot/boot.o of=hd.img bs=512 seek=0 count=1 conv=notrunc

${BUILD}/boot/%.o: boot/%.asm
	$(shell mkdir -p ${BUILD}/boot)
	nasm $< -o $@

clean:
	$(shell rm -rf ${BUILD})
	$(shell rm -rf bx_enh_dbg.ini)


bochs: all
	bochs -q -f bochsrc

qemu: all
	qemu-system-x86_64 -hda hd.img
