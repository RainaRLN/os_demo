BUILD:=./build
HD_IMG_NAME:= "hd.img"

CFLAGS:= -m32  # 32 位的程序
CFLAGS+= -masm=intel  # 使用 intel 汇编格式
CFLAGS+= -fno-builtin  # 不需要 gcc 内置函数
CFLAGS+= -nostdinc  # 不需要标准头文件
CFLAGS+= -fno-pic  # 不需要位置无关的代码  position independent code
CFLAGS+= -fno-pie  # 不需要位置无关的可执行程序 position independent executable
CFLAGS+= -nostdlib  # 不需要标准库
CFLAGS+= -fno-stack-protector  # 不需要栈保护
CFLAGS+=-Ioskernel/include
CFLAGS:=$(strip ${CFLAGS})

CFLAGS64 := -m64  # 64 位的程序
CFLAGS64 += -masm=intel -mcmodel=large
CFLAGS64 += -fno-builtin  # 不需要 gcc 内置函数
CFLAGS64 += -nostdinc  # 不需要标准头文件
CFLAGS64 += -fno-pic  # 不需要位置无关的代码  position independent code
CFLAGS64 += -fno-pie  # 不需要位置无关的可执行程序 position independent executable
CFLAGS64 += -nostdlib  # 不需要标准库
CFLAGS64 += -fno-stack-protector  # 不需要栈保护
CFLAGS64+=-Ix64/include

DEBUG:= -g -DDEBUG


all: ${BUILD}/boot/boot.o ${BUILD}/boot/setup.o ${BUILD}/system.bin ${BUILD}/x64/system.bin
	$(shell rm -rf $(BUILD)/$(HD_IMG_NAME))
	bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $(BUILD)/$(HD_IMG_NAME)
	dd if=${BUILD}/boot/boot.o of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=0 count=1 conv=notrunc
	dd if=${BUILD}/boot/setup.o of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=1 count=2 conv=notrunc
	dd if=${BUILD}/system.bin of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=3 count=30 conv=notrunc
	dd if=${BUILD}/x64/system.bin of=$(BUILD)/$(HD_IMG_NAME) bs=512 seek=40 count=255 conv=notrunc

${BUILD}/x64/system.bin: ${BUILD}/x64/kernel.bin
	objcopy -O binary ${BUILD}/x64/kernel.bin ${BUILD}/x64/system.bin
	nm ${BUILD}/x64/kernel.bin | sort > ${BUILD}/x64/system.map

${BUILD}/x64/kernel.bin: ${BUILD}/x64/boot/head.o ${BUILD}/x64/init/main64.o \
	${BUILD}/x64/kernel/asm/printk.o ${BUILD}/x64/kernel/printk.o ${BUILD}/x64/kernel/vsprintf.o \
	${BUILD}/x64/kernel/chr_drv/console.o ${BUILD}/x64/kernel/asm/io.o \
	${BUILD}/x64/lib/string.o ${BUILD}/x64/mm/mm.o ${BUILD}/x64/kernel/bitmap.o \
	${BUILD}/x64/mm/malloc.o ${BUILD}/x64/kernel/idt.o ${BUILD}/x64/kernel/pic.o \
	${BUILD}/x64/kernel/asm/interrupt.o ${BUILD}/x64/kernel/chr_drv/keyboard.o \
	${BUILD}/x64/kernel/exception.o \
	${BUILD}/x64/test/test.o ${BUILD}/x64/test/asm_test.o
	ld -b elf64-x86-64 -o $@ $^ -Ttext 0x100000

${BUILD}/x64/boot/%.o: x64/boot/%.asm
	$(shell mkdir -p ${BUILD}/x64/boot)
	nasm -f elf64 ${DEBUG} $< -o $@

${BUILD}/x64/kernel/asm/%.o: x64/kernel/asm/%.asm
	$(shell mkdir -p ${BUILD}/x64/kernel/asm)
	nasm -f elf64 ${DEBUG} $< -o $@

${BUILD}/x64/init/%.o: x64/init/%.c
	$(shell mkdir -p ${BUILD}/x64/init)
	gcc ${DEBUG} ${CFLAGS64} -c $< -o $@

${BUILD}/x64/kernel/%.o: x64/kernel/%.c
	$(shell mkdir -p ${BUILD}/x64/kernel)
	gcc ${DEBUG} ${CFLAGS64} -c $< -o $@

${BUILD}/x64/test/%.o: x64/test/%.c
	$(shell mkdir -p ${BUILD}/x64/test)
	gcc ${DEBUG} ${CFLAGS64} -c $< -o $@

${BUILD}/x64/test/%.o: x64/test/%.asm
	$(shell mkdir -p ${BUILD}/x64/test)
	nasm -f elf64 ${DEBUG} $< -o $@

${BUILD}/x64/lib/%.o: x64/lib/%.c
	$(shell mkdir -p ${BUILD}/x64/lib)
	gcc ${DEBUG} ${CFLAGS64} -c $< -o $@

${BUILD}/x64/mm/%.o: x64/mm/%.c
	$(shell mkdir -p ${BUILD}/x64/mm)
	gcc ${DEBUG} ${CFLAGS64} -c $< -o $@

${BUILD}/x64/kernel/chr_drv/%.o: x64/kernel/chr_drv/%.c
	$(shell mkdir -p ${BUILD}/x64/kernel/chr_drv)
	gcc ${DEBUG} ${CFLAGS64} -c $< -o $@

${BUILD}/system.bin: ${BUILD}/kernel.bin
	objcopy -O binary ${BUILD}/kernel.bin ${BUILD}/system.bin
	nm ${BUILD}/kernel.bin | sort > ${BUILD}/system.map

${BUILD}/kernel.bin: ${BUILD}/boot/head.o ${BUILD}/init/main.o ${BUILD}/kernel/asm/io.o ${BUILD}/kernel/chr_drv/console.o \
    ${BUILD}/lib/string.o ${BUILD}/kernel/vsprintf.o ${BUILD}/kernel/printk.o ${BUILD}/init/enter_x64.o \
	${BUILD}/kernel/gdt.o
	ld -m elf_i386 $^ -o $@ -Ttext 0x1200

${BUILD}/kernel/%.o: oskernel/kernel/%.c
	$(shell mkdir -p ${BUILD}/kernel)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/lib/%.o: oskernel/lib/%.c
	$(shell mkdir -p ${BUILD}/lib)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/kernel/chr_drv/%.o: oskernel/kernel/chr_drv/%.c
	$(shell mkdir -p ${BUILD}/kernel/chr_drv)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/kernel/asm/%.o: oskernel/kernel/asm/%.asm
	$(shell mkdir -p ${BUILD}/kernel/asm)
	nasm -f elf32 -g $< -o $@

${BUILD}/init/%.o: oskernel/init/%.c
	$(shell mkdir -p ${BUILD}/init)
	gcc ${CFLAGS} ${DEBUG} -c $< -o $@

${BUILD}/boot/head.o: oskernel/boot/head.asm
	nasm -f elf32 -g $< -o $@

${BUILD}/boot/%.o: oskernel/boot/%.asm
	$(shell mkdir -p ${BUILD}/boot)
	nasm $< -o $@

clean:
	$(shell rm -rf ${BUILD})

bochs: all
	bochs -q -f bochsrc

qemug: all
	qemu-system-x86_64 \
	-m 8G \
	-boot d \
	-hda ./build/hd.img -s -S

qemu: all
	qemu-system-x86_64 \
	-m 8G \
	-boot d \
	-hda ./build/hd.img

