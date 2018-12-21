LD = ld
OBJCOPY = objcopy
CC = cc
CFLAGS = -O2 -m16 -Wa,-32 -march=i486 \
	-ffreestanding -fno-stack-protector -fno-pie \
	-I${SRC_PATH}/include
VPATH = ${SRC_PATH}/pc-bios/optionrom

BINS = kvmvapic.bin linuxboot.bin linuxboot_dma.bin multiboot.bin
all: ${BINS}

%.o: %.S
	${CC} ${CFLAGS} -c -o $@ $<
%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<
%.img: %.o
	${LD} -m elf_i386 -T ${SRC_PATH}/pc-bios/optionrom/flat.lds -s -o $@ $<
%.raw: %.img
	${OBJCOPY} -O binary -j .text $< $@
%.bin: %.raw
	python ${SRC_PATH}/scripts/signrom.py $< $@

clean:
	rm -f ${BINS}

.PHONY: all clean
