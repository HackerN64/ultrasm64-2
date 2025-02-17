ifeq      ($(VERSION),jp)
  LIBULTRA ?= D
else ifeq ($(VERSION),us)
  LIBULTRA ?= D
  LIBULTRA_REVISION ?= 1
else ifeq ($(VERSION),eu)
  LIBULTRA ?= F
else ifeq ($(VERSION),sh)
  LIBULTRA ?= H
else ifeq ($(VERSION),cn)
  LIBULTRA ?= BB
endif

# Libultra number revision (only used on 2.0D)
LIBULTRA_REVISION ?= 0

# Special define for exclusive libultra files (driverominit)
LIBULTRA_EXCLUSIVE ?= 0

# LIBULTRA - sets the libultra OS version to use
$(eval $(call validate-option,LIBULTRA,D F H I K L BB))

ULTRA_VER_D := 1
ULTRA_VER_E := 2
ULTRA_VER_F := 3
ULTRA_VER_G := 4
ULTRA_VER_H := 5
ULTRA_VER_I := 6
ULTRA_VER_J := 7
ULTRA_VER_K := 8
ULTRA_VER_L := 9

ifeq ($(LIBULTRA),BB)
  ULTRA_SRC_DIRS := $(shell find lib/ultra -type d)
  ULTRA_VER_DEF  := -DLIBULTRA_VERSION=$(ULTRA_VER_L) -DBBPLAYER -DLIBULTRA_STR_VER=\"L\"
  LIBGULTRA ?= 1
else
  ULTRA_SRC_DIRS := $(shell find lib/ultra -type d -not -path "lib/ultra/bb/*")
  ULTRA_VER_DEF  := -DLIBULTRA_VERSION=$(ULTRA_VER_$(LIBULTRA)) -DLIBULTRA_REVISION=$(LIBULTRA_REVISION) -DLIBULTRA_STR_VER=\"$(LIBULTRA)\"
endif

DEF_INC_CFLAGS += $(ULTRA_VER_DEF)

ifeq ($(NON_MATCHING),1)
  LIBULTRA_EXCLUSIVE := 1
  DEF_INC_CFLAGS += -DLIBULTRA_EXCLUSIVE
endif

# LIBGULTRA - whenever to compile libultra using IDO or GNU
#   1 - uses egcs to match iQue (uses gcc if COMPILER is gcc)
#   0 - uses ido, used to match JP, US, EU and Shindou
LIBGULTRA ?= 1
$(eval $(call validate-option,LIBGULTRA,0 1))

ULTRA_C_FILES := $(foreach dir,$(ULTRA_SRC_DIRS),$(wildcard $(dir)/*.c))
ULTRA_S_FILES := $(foreach dir,$(ULTRA_SRC_DIRS),$(wildcard $(dir)/*.s))

ULTRA_O_C_FILES  := $(foreach file,$(ULTRA_C_FILES),$(BUILD_DIR)/$(file:.c=.o))
ULTRA_O_AS_FILES := $(foreach file,$(ULTRA_S_FILES),$(BUILD_DIR)/$(file:.s=.o))

ULTRA_O_FILES := $(ULTRA_O_C_FILES) $(ULTRA_O_AS_FILES)

DEP_FILES +=  $(ULTRA_O_FILES:.o=.d)

LIBULTRA_AR := $(BUILD_DIR)/libultra.a

libultra: $(LIBULTRA_AR)

REG_SIZES := -mgp32
NOABICALL := -mno-abicalls

OPT_FLAGS_LIBC :=
ifeq ($(LIBULTRA),BB)
  OPT_FLAGS_LIBC := -O2
else 
  ifneq ($(LIBULTRA),D)
    OPT_FLAGS_LIBC := -O3
  endif
endif

ULTRA_CC     := $(CC)
ULTRA_CFLAGS  = -non_shared -Wab,-r4300_mul -Xcpluscomm -Xfullwarn -G 0 -signed -32
ULTRA_ASFLAGS = -non_shared -Wab,-r4300_mul -Xcpluscomm -Xfullwarn -G 0 -nostdinc -o32 -c

GULTRA_CC     := COMPILER_PATH=$(EGCS_PATH) $(EGCS_PATH)/gcc
GULTRA_CFLAGS  = -mcpu=r4300 -fno-pic -Wa,--strip-local-absolute -G 0 -fno-common
GULTRA_ASFLAGS = -mcpu=r4300 -fno-pic -x assembler-with-cpp -c -DEGCS_GCC

ifeq ($(LIBGULTRA),1)
  $(ULTRA_O_C_FILES): CC := $(GULTRA_CC)
  $(ULTRA_O_C_FILES): CFLAGS = $(GULTRA_CFLAGS) $(NOABICALL) $(REG_SIZES)

  $(ULTRA_O_AS_FILES): AS := $(GULTRA_CC)
  $(ULTRA_O_AS_FILES): ASFLAGS = $(GULTRA_ASFLAGS)
  $(ULTRA_O_AS_FILES): MIPSISET :=
else
  $(ULTRA_O_C_FILES): CC := $(ULTRA_CC)
  $(ULTRA_O_C_FILES): CFLAGS = $(ULTRA_CFLAGS)

  $(ULTRA_O_AS_FILES): AS := $(ULTRA_CC)
  $(ULTRA_O_AS_FILES): ASFLAGS = $(ULTRA_ASFLAGS)
  $(ULTRA_O_AS_FILES): OPT_FLAGS :=
  $(ULTRA_O_AS_FILES): MIPSISET := -mips2
endif

# Libultra specific flags
ifneq ($(LIBULTRA),BB)
  $(BUILD_DIR)/lib/ultra/os/exceptasm.o:   MIPSISET    := -mips3
  $(BUILD_DIR)/lib/ultra/libc/%.o:         ASOPT_FLAGS := -O2
  $(BUILD_DIR)/lib/ultra/libc/ll.o:        MIPSISET    := -mips3 -32
  $(BUILD_DIR)/lib/ultra/libc/ll%.o:       MIPSISET    := -mips3 -32
endif

ifeq ($(NON_MATCHING),0)
  $(BUILD_DIR)/lib/ultra/%.o:                OPT_FLAGS :=

  $(BUILD_DIR)/lib/ultra/audio/bnkf.o:       OPT_FLAGS := -O3
  $(BUILD_DIR)/lib/ultra/gu/%.o:             OPT_FLAGS := -O3

  $(BUILD_DIR)/lib/ultra/libc/ldiv.o:        OPT_FLAGS := -O2
  $(BUILD_DIR)/lib/ultra/libc/string.o:      OPT_FLAGS := -O2

  $(BUILD_DIR)/lib/ultra/libc/sprintf.o:     OPT_FLAGS := $(OPT_FLAGS_LIBC)
  $(BUILD_DIR)/lib/ultra/libc/syncprintf.o:  OPT_FLAGS := $(OPT_FLAGS_LIBC)
  $(BUILD_DIR)/lib/ultra/libc/xlitob.o:      OPT_FLAGS := $(OPT_FLAGS_LIBC)
  $(BUILD_DIR)/lib/ultra/libc/xldtob.o:      OPT_FLAGS := $(OPT_FLAGS_LIBC)
  $(BUILD_DIR)/lib/ultra/libc/xprintf.o:     OPT_FLAGS := $(OPT_FLAGS_LIBC)

  ifeq ($(LIBULTRA),BB)
    $(BUILD_DIR)/lib/ultra/%.o:              MIPSISET  :=

    $(BUILD_DIR)/lib/ultra/bb/os/%.o:        MIPSISET  := -mips3
    $(BUILD_DIR)/lib/ultra/bb/os/%.o:        REG_SIZES := -mgp64

    $(BUILD_DIR)/lib/ultra/bb/wrapper.o:     NOABICALL :=
    $(BUILD_DIR)/lib/ultra/bb/wrapper.o:     OPT_FLAGS := -O2

    $(BUILD_DIR)/lib/ultra/gu/%.o:           MIPSISET  := -mips2
    $(BUILD_DIR)/lib/ultra/gu/%.o:           OPT_FLAGS := -O2

    $(BUILD_DIR)/lib/ultra/io/%.o:           MIPSISET  := -mips2
    $(BUILD_DIR)/lib/ultra/io/%.o:           OPT_FLAGS := -O2

    $(BUILD_DIR)/lib/ultra/libc/%.o:         MIPSISET  := -mips2

    $(BUILD_DIR)/lib/ultra/os/%.o:           MIPSISET    := -mips2
    $(BUILD_DIR)/lib/ultra/os/%.o:           ASOPT_FLAGS := -O2
  endif
endif

$(BUILD_DIR)/lib/ultra/%.o: lib/ultra/%.c
	$(call print,Compiling:,$<,$@)
	$(V)$(CC_CHECK) $(CC_CHECK_CFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/lib/ultra/$*.d $<
	$(V)$(CC) -c $(CFLAGS) $(TARGET_CFLAGS) $(OPT_FLAGS) $(MIPSISET) $(DEF_INC_CFLAGS) -o $@ $<
ifeq ($(LIBGULTRA),1)
	$(V)$(TOOLS_DIR)/patch_elf_32bit $@
endif

$(BUILD_DIR)/lib/ultra/%.o: lib/ultra/%.s
	$(call print,Assembling:,$<,$@)
	$(V)$(AS) $(ASFLAGS) $(ASOPT_FLAGS) $(MIPSISET) $(DEF_INC_CFLAGS) -o $@ $<

ifeq ($(LIBULTRA_EXCLUSIVE),0)
# Exclusive file due to being in audio alongside using IDO and different optimization flags
$(BUILD_DIR)/lib/ultra/io/driverominit.o: lib/ultra/io/driverominit.c
	$(call print,Compiling:,$<,$@)
	$(V)$(CC_CHECK) $(CC_CHECK_CFLAGS) -MMD -MP -MT $@ -MF $(BUILD_DIR)/lib/ultra/io/$*.d $<
	$(V)$(ULTRA_CC) -c $(ULTRA_CFLAGS) $(TARGET_CFLAGS) $(DEF_INC_CFLAGS) -mips2 -g -o $@ $<
endif

# Link libultra
$(LIBULTRA_AR): $(ULTRA_O_FILES)
	@$(PRINT) "$(GREEN)Linking libultra:  $(BLUE)$@ $(NO_COL)\n"
	$(V)$(AR) rcs -o $@ $(ULTRA_O_FILES)
	$(V)$(TOOLS_DIR)/patch_elf_32bit $@
