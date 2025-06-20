# One of:
# libgultra_rom, libgultra_d, libgultra
# libultra_rom, libultra_d, libultra
TARGET ?= libgultra_rom
VERSION ?= L
VERBOSE ?= 0

include util.mk

ifeq ($(VERBOSE), 0)
V=@
else
V=
endif
# detect hackerchain
ifneq ($(call find-command, $(HACKERCHAIN)/mips64-elf-ld),)
    CROSS := $(HACKERCHAIN)/mips64-elf-
else
    $(error Unable to detect a hackerchain toolchain installation.)
endif

BUILD_ROOT := build
BUILD_DIR := $(BUILD_ROOT)/$(VERSION)/$(TARGET)
BUILD_AR := $(BUILD_DIR)/$(TARGET).a

WORKING_DIR := $(shell pwd)

CPP := cpp -P
AR := $(CROSS)ar

VERSION_DEFINE := -DBUILD_VERSION=VERSION_$(VERSION) -DBUILD_VERSION_STRING=\"2.0$(VERSION)\"

ifeq ($(findstring _d,$(TARGET)),_d)
DEBUGFLAG := -D_DEBUG
else
DEBUGFLAG := -DNDEBUG
endif

AS := $(CROSS)gcc -x assembler-with-cpp
CC := $(CROSS)gcc

WARNINGS := -Wall -Wextra -Wno-format-security -Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-builtin-declaration-mismatch
WARNINGS += -Wno-int-conversion -Wno-incompatible-pointer-types -Wno-implicit-function-declaration # TODO: Try adjusting code to remove these
CFLAGS := -G 0 -c -nostdinc -march=vr4300 -mfix4300 -mabi=32 -mno-abicalls -mdivide-breaks -fno-PIC -fno-common -ffreestanding -fbuiltin -fno-builtin-sinf -fno-builtin-cosf -funsigned-char $(WARNINGS)
CFLAGS += -fno-strict-aliasing # TODO: Try adjusting code to remove this
ASFLAGS := -w -nostdinc -c -G 0 -march=vr4300 -mabi=32 -mgp32 -mfp32 -DMIPSEB -D_LANGUAGE_ASSEMBLY -D_MIPS_SIM=1 -D_ULTRA64
CPPFLAGS = -DMODERN_CC -D_MIPS_SZLONG=32 -D__USE_ISOC99 $(GBIDEFINE) $(VERSION_DEFINE) $(DEBUGFLAG)
IINC = -I . -I $(WORKING_DIR)/include -I $(WORKING_DIR)/include/compiler/modern_gcc -I $(WORKING_DIR)/include/PR
MIPS_VERSION := -mips3
ASOPTFLAGS :=

ifeq ($(findstring _d,$(TARGET)),_d)
OPTFLAGS := -Og -ggdb3 -ffast-math -fno-unsafe-math-optimizations
else
OPTFLAGS := -Os -ggdb3 -ffast-math -fno-unsafe-math-optimizations
endif

ifeq ($(findstring _rom,$(TARGET)),_rom)
CPPFLAGS += -D_FINALROM
endif

SRC_DIRS := $(shell find src -type d)
C_FILES  := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
S_FILES  := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.s))

# Versions J and below used the C matrix math implementations
MGU_MATRIX_FILES := mtxcatf normalize scale translate
ifneq ($(filter $(VERSION),D E F G H I J),)
S_FILES := $(filter-out $(addprefix src/mgu/,$(MGU_MATRIX_FILES:=.s)),$(S_FILES))
else
C_FILES := $(filter-out $(addprefix src/gu/,$(MGU_MATRIX_FILES:=.c)),$(C_FILES))
endif

C_O_FILES := $(foreach f,$(C_FILES:.c=.o),$(BUILD_DIR)/$f)
S_O_FILES := $(foreach f,$(S_FILES:.s=.o),$(BUILD_DIR)/$f)
O_FILES   := $(S_O_FILES) $(C_O_FILES)

AR_OBJECTS := $(shell cat base/$(VERSION)/$(TARGET).txt)

$(shell mkdir -p src $(foreach dir,$(SRC_DIRS),$(BUILD_DIR)/$(dir)))

.PHONY: all clean distclean setup
all: $(BUILD_AR)

$(BUILD_AR): $(O_FILES)
	@printf "    [AR] $@\n"
	$(V)$(AR) rcs $@ $^

clean:
	$(RM) -rf $(BUILD_DIR)

distclean:
	$(RM) -rf extracted/ $(BUILD_ROOT)

GBIDEFINE := -DF3DEX_GBI

$(BUILD_DIR)/src/gu/parse_gbi.o: GBIDEFINE := -DF3D_GBI
$(BUILD_DIR)/src/gu/us2dex_emu.o: GBIDEFINE :=
$(BUILD_DIR)/src/gu/us2dex2_emu.o: GBIDEFINE :=
$(BUILD_DIR)/src/sp/sprite.o: GBIDEFINE := -DF3D_GBI
$(BUILD_DIR)/src/sp/spriteex.o: GBIDEFINE :=
$(BUILD_DIR)/src/sp/spriteex2.o: GBIDEFINE :=
$(BUILD_DIR)/src/voice/%.o: OPTFLAGS += -DLANG_JAPANESE -I$(WORKING_DIR)/src -I$(WORKING_DIR)/src/voice
$(BUILD_DIR)/src/voice/%.o: CC := $(WORKING_DIR)/tools/compile_sjis.py -D__CC=$(CC) -D__BUILD_DIR=$(BUILD_DIR)

$(BUILD_DIR)/%.o: %.c
	@printf "    [CC] $<\n"
	$(V)$(CC) $(CFLAGS) $(MIPS_VERSION) $(CPPFLAGS) $(OPTFLAGS) $< $(IINC) -o $@
	$(V)tools/set_o32abi_bit.py $@

$(BUILD_DIR)/%.o: %.s
	@printf "    [AS] $<\n"
	$(V)$(AS) $(ASFLAGS) $(MIPS_VERSION) $(CPPFLAGS) $(ASOPTFLAGS) $< $(IINC) -o $@
	$(V)tools/set_o32abi_bit.py $@

# Disable built-in rules
.SUFFIXES:
print-% : ; $(info $* is a $(flavor $*) variable set to [$($*)]) @true
