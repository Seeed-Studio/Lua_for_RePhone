

# USE_CUSTOM_TOUCHPAD = 1

# GCC_BIN = ../LinkIt-1.1-1.60/hardware/tools/gcc-arm-none-eabi-4.8.3-2014q1/bin/
PROJECT = lua
OBJECTS = main.o console.o lua/lua.o lua/linenoise.o
OBJECTS += lua/lapi.o lua/lcode.o lua/ldebug.o lua/ldo.o lua/ldump.o lua/lfunc.o lua/lgc.o lua/llex.o lua/lmem.o \
	lua/lobject.o lua/lopcodes.o lua/lparser.o lua/lstate.o lua/lstring.o lua/ltable.o lua/ltm.o  \
	lua/lundump.o lua/lvm.o lua/lzio.o lua/lrotable.o \
	lua/lauxlib.o lua/lbaselib.o lua/ldblib.o lua/liolib.o lua/lmathlib.o lua/loslib.o lua/ltablib.o \
	lua/lstrlib.o lua/loadlib.o lua/linit.o
OBJECTS += laudiolib.o
SYS_OBJECTS = ./linkit/lib/LINKIT10/src/gccmain.o
INCLUDE_PATHS = -I. -I. -I./linkit/include -I./common -I./lua
LIBRARY_PATHS = -L./linkit/lib
LIBRARIES = ./linkit/lib/LINKIT10/armgcc/percommon.a -lm
# LIBRARIES = -lmtk
LINKER_SCRIPT = ./linkit/lib/LINKIT10/armgcc/scat.ld

###############################################################################
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy
OBJDUMP = $(GCC_BIN)arm-none-eabi-objdump
SIZE 	= $(GCC_BIN)arm-none-eabi-size
PACK    = ./tools/PackTag
PUSH    = ./tools/PushTool

CPU = -mcpu=arm7tdmi-s -mthumb -mlittle-endian
CC_FLAGS = $(CPU) -c -fvisibility=hidden -fpic -O2
CC_SYMBOLS = -D__HDK_LINKIT_ASSIST_2502__ -D__COMPILER_GCC__

ifeq ($(USE_CUSTOM_TOUCHPAD), 1)
	CC_SYMBOLS += -DCUSTOM_TOUCHPAD
endif

LD_FLAGS = $(CPU) -O2 -Wl,--gc-sections --specs=nosys.specs -fpic -pie -Wl,-Map=$(PROJECT).map  -Wl,--entry=gcc_entry -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-unresolved-symbols
LD_SYS_LIBS =

all: $(PROJECT).vxp size

clean:
	rm -f $(PROJECT).vxp $(PROJECT).bin $(PROJECT).elf $(PROJECT).hex $(PROJECT).map $(PROJECT).lst $(OBJECTS) $(DEPS)

.s.o:
	$(AS) $(CPU) -o $@ $<

.c.o:
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

.cpp.o:
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98 $(INCLUDE_PATHS) -o $@ $<


$(PROJECT).elf: $(OBJECTS) $(SYS_OBJECTS)
	$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ -Wl,--start-group $^ $(LIBRARIES) $(LD_SYS_LIBS) -Wl,--end-group

$(PROJECT).bin: $(PROJECT).elf
	@$(OBJCOPY) -O binary $< $@

$(PROJECT).hex: $(PROJECT).elf
	@$(OBJCOPY) -O ihex $< $@

$(PROJECT).vxp: $(PROJECT).elf
	@$(OBJCOPY) --strip-debug $<
	@$(PACK) $< $@

$(PROJECT).lst: $(PROJECT).elf
	@$(OBJDUMP) -Sdh $< > $@

lst: $(PROJECT).lst

size:
	$(SIZE) $(PROJECT).elf

flash: $(PROJECT).vxp
	$(PUSH) -v -v -v -v -t arduino -clear -port $(PORT) -app $(PROJECT).vxp

DEPS = $(OBJECTS:.o=.d) $(SYS_OBJECTS:.o=.d)
-include $(DEPS)
