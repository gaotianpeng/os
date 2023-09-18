BUILD_DIR = ./build

DISK_IMG = hd60M.img
ENTRY_POINT = 0xc0001500

AS = nasm
CC = gcc
LD = ld
LIB = -I lib/ -I lib/kernel/ -I lib/user/ -I kernel/ -I device/ -I thread/ -I userprog/ -I fs/ -I shell/

ASFLAGS = -f elf 
ASBINLIB = -I boot/include
CFLAGS = -m32 -Wall $(LIB) -c -fno-builtin -W -Wstrict-prototypes \
         -Wmissing-prototypes -fno-stack-protector
LDFLAGS = -melf_i386 -Ttext $(ENTRY_POINT) -e main -Map $(BUILD_DIR)/kernel.map
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/init.o $(BUILD_DIR)/interrupt.o \
		$(BUILD_DIR)/timer.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/print.o \
		$(BUILD_DIR)/debug.o $(BUILD_DIR)/bitmap.o $(BUILD_DIR)/string.o \
		$(BUILD_DIR)/memory.o $(BUILD_DIR)/thread.o ${BUILD_DIR}/list.o \
    	$(BUILD_DIR)/switch.o $(BUILD_DIR)/console.o $(BUILD_DIR)/sync.o \
		$(BUILD_DIR)/keyboard.o $(BUILD_DIR)/ioqueue.o $(BUILD_DIR)/tss.o \
	   	$(BUILD_DIR)/process.o  $(BUILD_DIR)/syscall.o $(BUILD_DIR)/syscall_init.o \
	   	$(BUILD_DIR)/stdio.o $(BUILD_DIR)/ide.o $(BUILD_DIR)/stdio_kernel.o $(BUILD_DIR)/fs.o \
	   	$(BUILD_DIR)/inode.o $(BUILD_DIR)/file.o $(BUILD_DIR)/dir.o $(BUILD_DIR)/fork.o \
		$(BUILD_DIR)/shell.o $(BUILD_DIR)/assert.o $(BUILD_DIR)/buildin_cmd.o \
		 $(BUILD_DIR)/exec.o $(BUILD_DIR)/wait_exit.o $(BUILD_DIR)/pipe.o

#####################################
$(BUILD_DIR)/mbr.bin: boot/mbr.asm
	$(AS) $(ASBINLIB) $< -o $@

$(BUILD_DIR)/loader.bin: boot/loader.asm
	$(AS) $(ASBINLIB) $< -o $@

$(BUILD_DIR)/main.o: kernel/main.c lib/kernel/print.h \
        lib/stdint.h kernel/init.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/init.o: kernel/init.c kernel/init.h lib/kernel/print.h \
        lib/stdint.h kernel/interrupt.h device/timer.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/interrupt.o: kernel/interrupt.c kernel/interrupt.h \
        lib/stdint.h kernel/global.h lib/kernel/io.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: device/timer.c device/timer.h lib/stdint.h\
         lib/kernel/io.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/debug.o: kernel/debug.c kernel/debug.h \
        lib/kernel/print.h lib/stdint.h kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/string.o: lib/string.c lib/string.h \
        lib/stdint.h  kernel/debug.h  lib/string.h kernel/global.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/bitmap.o: lib/kernel/bitmap.c lib/kernel/bitmap.h \
        lib/kernel/print.h lib/stdint.h kernel/interrupt.h \
		kernel/debug.h  lib/string.h kernel/global.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: kernel/memory.c kernel/memory.h lib/stdint.h lib/kernel/bitmap.h \
	   	kernel/global.h kernel/global.h kernel/debug.h lib/kernel/print.h \
		lib/kernel/io.h kernel/interrupt.h lib/string.h lib/stdint.h
			$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/thread.o: thread/thread.c thread/thread.h lib/stdint.h \
	    kernel/global.h lib/kernel/bitmap.h kernel/memory.h lib/string.h \
		lib/stdint.h lib/kernel/print.h kernel/interrupt.h kernel/debug.h
		$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/list.o: lib/kernel/list.c lib/kernel/list.h kernel/global.h lib/stdint.h \
	kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/console.o: device/console.c device/console.h lib/stdint.h \
	    lib/kernel/print.h thread/sync.h lib/kernel/list.h kernel/global.h \
		thread/thread.h thread/thread.h
		$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/sync.o: thread/sync.c thread/sync.h lib/kernel/list.h kernel/global.h \
	    lib/stdint.h thread/thread.h lib/string.h lib/stdint.h kernel/debug.h \
		kernel/interrupt.h
		$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: device/keyboard.c device/keyboard.h lib/kernel/print.h \
	    lib/stdint.h kernel/interrupt.h lib/kernel/io.h thread/thread.h \
		lib/kernel/list.h kernel/global.h thread/sync.h thread/thread.h
		$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ioqueue.o: device/ioqueue.c device/ioqueue.h lib/stdint.h thread/thread.h \
	lib/kernel/list.h kernel/global.h thread/sync.h thread/thread.h kernel/interrupt.h \
	kernel/debug.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/tss.o: userprog/tss.c userprog/tss.h thread/thread.h lib/stdint.h \
	lib/kernel/list.h kernel/global.h lib/string.h lib/stdint.h \
	lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/process.o: userprog/process.c userprog/process.h thread/thread.h \
	lib/stdint.h lib/kernel/list.h kernel/global.h kernel/debug.h \
	kernel/memory.h lib/kernel/bitmap.h userprog/tss.h kernel/interrupt.h \
	lib/string.h lib/stdint.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall.o: lib/user/syscall.c lib/user/syscall.h lib/stdint.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/syscall_init.o: userprog/syscall_init.c userprog/syscall_init.h \
	lib/stdint.h lib/user/syscall.h lib/kernel/print.h thread/thread.h \
	lib/kernel/list.h kernel/global.h lib/kernel/bitmap.h kernel/memory.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stdio.o: lib/stdio.c lib/stdio.h lib/stdint.h kernel/interrupt.h \
    	lib/stdint.h kernel/global.h lib/string.h lib/user/syscall.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ide.o: device/ide.c device/ide.h lib/stdint.h thread/sync.h \
	lib/kernel/list.h kernel/global.h thread/thread.h lib/kernel/bitmap.h \
	kernel/memory.h lib/kernel/io.h lib/stdio.h lib/stdint.h lib/kernel/stdio_kernel.h\
	kernel/interrupt.h kernel/debug.h device/console.h device/timer.h lib/string.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/stdio_kernel.o: lib/kernel/stdio_kernel.c lib/kernel/stdio_kernel.h lib/stdint.h \
	lib/kernel/print.h lib/stdio.h lib/stdint.h device/console.h kernel/global.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/fs.o: fs/fs.c fs/fs.h lib/stdint.h device/ide.h thread/sync.h lib/kernel/list.h \
	kernel/global.h thread/thread.h lib/kernel/bitmap.h kernel/memory.h fs/super_block.h \
	fs/inode.h fs/dir.h lib/kernel/stdio_kernel.h lib/string.h lib/stdint.h kernel/debug.h \
	kernel/interrupt.h lib/kernel/print.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/inode.o: fs/inode.c fs/inode.h lib/stdint.h lib/kernel/list.h \
	kernel/global.h fs/fs.h device/ide.h thread/sync.h thread/thread.h \
	lib/kernel/bitmap.h kernel/memory.h fs/file.h kernel/debug.h \
	kernel/interrupt.h lib/kernel/stdio_kernel.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/file.o: fs/file.c fs/file.h lib/stdint.h device/ide.h thread/sync.h \
	lib/kernel/list.h kernel/global.h thread/thread.h lib/kernel/bitmap.h \
	kernel/memory.h fs/fs.h fs/inode.h fs/dir.h lib/kernel/stdio_kernel.h \
	kernel/debug.h kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/dir.o: fs/dir.c fs/dir.h lib/stdint.h fs/inode.h lib/kernel/list.h \
	kernel/global.h device/ide.h thread/sync.h thread/thread.h \
	lib/kernel/bitmap.h kernel/memory.h fs/fs.h fs/file.h \
	lib/kernel/stdio_kernel.h kernel/debug.h kernel/interrupt.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/fork.o: userprog/fork.c userprog/fork.h thread/thread.h lib/stdint.h \
	lib/kernel/list.h kernel/global.h lib/kernel/bitmap.h kernel/memory.h \
	userprog/process.h kernel/interrupt.h kernel/debug.h \
	lib/kernel/stdio_kernel.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/shell.o: shell/shell.c shell/shell.h lib/stdint.h fs/fs.h \
	lib/user/syscall.h lib/stdio.h lib/stdint.h kernel/global.h lib/user/assert.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/assert.o: lib/user/assert.c lib/user/assert.h lib/stdio.h lib/stdint.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/buildin_cmd.o: shell/buildin_cmd.c shell/buildin_cmd.h lib/stdint.h \
	lib/user/syscall.h lib/stdio.h lib/stdint.h lib/string.h fs/fs.h
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/exec.o: userprog/exec.c userprog/exec.h thread/thread.h lib/stdint.h \
	lib/kernel/list.h kernel/global.h lib/kernel/bitmap.h kernel/memory.h \
	lib/kernel/stdio_kernel.h fs/fs.h lib/string.h lib/stdint.h
	$(CC) $(CFLAGS) $< -o $@

		
$(BUILD_DIR)/wait_exit.o: userprog/wait_exit.c userprog/wait_exit.h \
    	userprog/../thread/thread.h lib/stdint.h lib/kernel/list.h \
     	kernel/global.h lib/kernel/bitmap.h kernel/memory.h kernel/debug.h \
      	thread/thread.h lib/kernel/stdio_kernel.h
	$(CC) $(CFLAGS) $< -o $@
	
$(BUILD_DIR)/pipe.o: shell/pipe.c shell/pipe.h lib/stdint.h kernel/memory.h \
    	lib/kernel/bitmap.h kernel/global.h lib/kernel/list.h fs/fs.h fs/file.h \
     	device/ide.h thread/sync.h thread/thread.h fs/dir.h fs/inode.h fs/fs.h \
      	device/ioqueue.h thread/thread.h
	$(CC) $(CFLAGS) $< -o $@


#####################################
$(BUILD_DIR)/kernel.o: kernel/kernel.asm
	$(AS) $(ASFLAGS) $< -o $@
$(BUILD_DIR)/print.o: lib/kernel/print.asm
	$(AS) $(ASFLAGS) $< -o $@
$(BUILD_DIR)/switch.o: thread/switch.asm
		$(AS) $(ASFLAGS) $< -o $@

#####################################
$(BUILD_DIR)/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

#####################################

.PHONY : mk_dir hd clean all

mk_dir:
	$(shell rm -rf ${BUILD_DIR})
	$(shell mkdir $(BUILD_DIR))

hd:
	$(shell rm -rf $(DISK_IMG))
	bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $(DISK_IMG)
	dd if=$(BUILD_DIR)/mbr.bin of=$(DISK_IMG) bs=512 count=1  conv=notrunc
	dd if=$(BUILD_DIR)/loader.bin of=$(DISK_IMG) bs=512 count=4 seek=2 conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$(DISK_IMG) bs=512 count=200 seek=9 conv=notrunc

clean:
	$(shell rm -rf ${BUILD_DIR})
	$(shell rm -rf $(DISK_IMG))
	$(shell rm -rf ./bx_enh_dbg.ini)


build: $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/mbr.bin $(BUILD_DIR)/loader.bin

all: mk_dir build hd

bochs: all
	bochs -q -f bochsrc

# bximage -q -hd=50 -func=create -sectsize=512 -imgmode=flat hd50M.img
# echo -e  "n\np\n1\n\n+4M\nn\ne\n2\n\n\nn\n\n+5M\nn\n\n+6M\nn\n\n+7M\nn\n\n+8M\nn\n\n+9M\nn\n\n\nw\n" | fdisk hd50M.img &> /dev/null

# gcc -m32 -Wall -c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes    -Wsystem-headers -I ../lib -o prog_no_arg.o prog_no_arg.c
# ld -melf_i386  -e main prog_no_arg.o ../build/string.o ../build/syscall.o   ../build/stdio.o ../build/assert.o -o prog_no_arg
# dd if=prog_no_arg of=/home/os/work/os/hd60M.img    bs=512 count=10 seek=300 conv=notrunc