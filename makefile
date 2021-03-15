TARGET = Chip-8
TARGET_ELF = elf.elf
OBJS = main.o callbacks.o graphics.o framebuffer.o\
psp.o CHIP8.o filer.o controller.o

CFLAGS = -O3 -G0 -Wall -std=c99
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -fexceptions
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = -lpsppower -lpspgu -lpspusb -lpspaudiolib -lpspaudio
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP 
PSP_EBOOT_TITLE = Chip-8

PSPSDK=$(shell psp-config --pspsdk-path)

include $(PSPSDK)/lib/build.mak
