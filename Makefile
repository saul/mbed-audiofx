PKG=../arm-2013.05
BIN=$(PKG)/bin

SHELL=bash
ARCH=arm-none-eabi
CC=$(BIN)/$(ARCH)-gcc
OBJCOPY=$(BIN)/$(ARCH)-objcopy

SOURCERY=$(PKG)/lib/gcc
GNU_VERSION=4.7.3
THUMB2LIB=$(SOURCERY)/$(ARCH)/$(GNU_VERSION)/thumb2

# "Cortex Microcontroller Software Interface Standard" Startup files
CMSIS=/usr/local/pkg/lpc1700-cmsis-lite-2011.01.26-i686-1
CMSISINCLUDES=-I$(CMSIS)/include
CMSISFL=$(CMSIS)/lib/core_cm3.o \
	$(CMSIS)/lib/system_LPC17xx.o \
	$(CMSIS)/lib/startup_LPC17xx.o
LDSCRIPT=$(CMSIS)/lib/ldscript_rom_gnu.ld

SHAREDFLAGS=-mcpu=cortex-m3 -march=armv7-m -mthumb -mthumb-interwork \
	-Wall -Wextra -Werror -pedantic -Wformat=2 -Wno-unused-parameter \
	-Wpointer-arith -Wredundant-decls -Warray-bounds \
	-Wdouble-promotion -Wno-error=double-promotion

CFLAGS=$(SHAREDFLAGS) -mtune=cortex-m3 -O3 -std=c99 -fno-hosted \
	-msoft-float -ffunction-sections -fdata-sections \
	-D__thumb2__=1 -D__RAM_MODE__=0 $(CMSISINCLUDES) \
	-I. -I../common

LDFLAGS=$(SHAREDFLAGS) $(CMSISFL) -static \
	-Wl,--start-group -L$(THUMB2LIB) \
	-lc -lg -lstdc++ -lsupc++ -lgcc -lm -Wl,--end-group \
	-L$(CMSIS)/lib -lDriversLPC17xxgnu \
	-Xlinker -Map -Xlinker bin/lpc1700.map -Xlinker -T $(LDSCRIPT)

EXECNAME=bin/audiofx

OBJ=sercom.o \
	packets.o \
	bytebuffer.o \
	ticktime.o \
	led.o \
	dbg.o \
	i2c.o \
	lcd.o \
	keypad.o \
	adc.o \
	dac.o \
	microtimer.o \
	chain.o \
	filters.o \
	filters/delay.o \
	samples.o \
	main.o

# Colours
CLR_RESET=\033[m
CLR_RED=\033[31m
CLR_GREEN=\033[32m
CLR_YELLOW=\033[33m
CLR_BLUE=\033[34m
CLR_MAGENTA=\033[35m
CLR_CYAN=\033[36m
CLR_BRIGHT=\033[1m
FMT_UNDERLINE=\033[4m
FMT_NO_UNDERLINE=\033[24m
FMT_REVERSE=\033[7m
FMT_NO_REVERSE=\033[27m

LINE_PREFIX=$(CLR_GREEN)* $(CLR_RESET)

all: $(EXECNAME).bin
	@echo -e "$(LINE_PREFIX)$(CLR_GREEN)Build finished$(CLR_RESET)"

# copy program to binary (bare metal) target
$(EXECNAME).bin: $(EXECNAME).elf
	@echo -e "$(LINE_PREFIX)Converting ELF to binary..."
	@$(OBJCOPY) -I elf32-little -O binary $< $@

# generate symbol table and link files to an ELF
$(EXECNAME).elf: $(OBJ)
	@mkdir -p bin
	@echo -e "$(LINE_PREFIX)Linking..."
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)

# generate assembly and object file
%.o: %.c
	@echo -e "$(LINE_PREFIX)Compiling $(CLR_BRIGHT)$(CLR_BLUE)$<$(CLR_RESET)..."
	@$(CC) -c $(CFLAGS) -o $@ $<

# clean out source tree
clean:
	@rm -f *~ *.o
	@rm -rf bin/
	@echo -e "$(LINE_PREFIX)$(CLR_GREEN)Cleaned source tree$(CLR_RESET)"

# copy software to board
install:
	@echo -e "$(LINE_PREFIX)Copying $(CLR_BRIGHT)$(CLR_BLUE)$(EXECNAME)$(CLR_RESET) to the MBED file system..."
	@cp $(EXECNAME).bin /media/MBED
	@echo -e "$(LINE_PREFIX)Synchronising$(CLR_RESET)..."
	@sync
	@echo -e "$(LINE_PREFIX)$(CLR_GREEN)Installed! Reset MBED board$(CLR_RESET)"
