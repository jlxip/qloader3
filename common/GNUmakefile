include $(CROSS_FILE)

TARGET ?=
BUILDDIR ?=

override SRCDIR := $(shell pwd -P)

override SPACE := $(subst ,, )

MKESCAPE = $(subst $(SPACE),\ ,$(1))
SHESCAPE = $(subst ','\'',$(1))
OBJESCAPE = $(subst .a ,.a' ',$(subst .o ,.o' ',$(call SHESCAPE,$(1))))

ifeq ($(call MKESCAPE,$(BUILDDIR)),)
    $(error BUILDDIR not specified)
endif

ifeq ($(TARGET),bios)
    override OBJCOPY2ELF_FLAGS := -B i386 -O elf32-i386
else ifeq ($(TARGET),uefi-x86-64)
    override OBJCOPY2ELF_FLAGS := -B i386 -O elf64-x86-64
else ifeq ($(TARGET),uefi-ia32)
    override OBJCOPY2ELF_FLAGS := -B i386 -O elf32-i386
else
    $(error Invalid target)
endif

COM_OUTPUT ?= false
E9_OUTPUT ?= false

override S2CFLAGS := -Os

override BASE_CFLAGS := $(CROSS_CFLAGS)

override CROSS_CFLAGS += \
    -g \
    -std=gnu11 \
    -nostdinc \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-omit-frame-pointer \
    -fno-strict-aliasing \
    -fno-lto \
    -Wshadow \
    -Wvla \
    $(WERROR) \
    -MMD \
    -DCOM_OUTPUT=$(COM_OUTPUT) \
    -DE9_OUTPUT=$(E9_OUTPUT) \
    -I../freestanding-headers \
    -I'$(call SHESCAPE,$(BUILDDIR))/tinf' \
    -I'$(call SHESCAPE,$(BUILDDIR))/..' \
    -I. \
    -I.. \
    -I../stivale \

ifeq ($(TARGET),bios)
    override CROSS_CFLAGS += \
        -fno-pie \
        -fno-pic \
        -m32 \
        -march=i686 \
        -mabi=sysv \
        -mno-80387 \
        -DBIOS
endif

ifeq ($(TARGET),uefi-x86-64)
    override INTERNAL_CFLAGS32 := \
        $(CROSS_CFLAGS) \
        -g0 \
        -fpie \
        -m32 \
        -march=i686 \
        -mabi=sysv \
        -mno-80387 \
        -DUEFI

    override CROSS_CFLAGS += \
        -fpie \
        -m64 \
        -march=x86-64 \
        -mabi=sysv \
        -mno-80387 \
        -mno-mmx \
        -mno-sse \
        -mno-sse2 \
        -mno-red-zone \
        -DUEFI \
        -DGNU_EFI_USE_MS_ABI \
        -I'$(call SHESCAPE,$(BUILDDIR))/limine-efi/inc' \
        -I'$(call SHESCAPE,$(BUILDDIR))/limine-efi/inc/x86_64'
endif

ifeq ($(TARGET),uefi-ia32)
    override CROSS_CFLAGS += \
        -fpie \
        -m32 \
        -march=i686 \
        -mabi=sysv \
        -mno-80387 \
        -DUEFI \
        -I'$(call SHESCAPE,$(BUILDDIR))/limine-efi/inc' \
        -I'$(call SHESCAPE,$(BUILDDIR))/limine-efi/inc/ia32'
endif

override CROSS_LDFLAGS += \
    -nostdlib \
    -z max-page-size=0x1000

ifeq ($(TARGET),bios)
    override CROSS_LDFLAGS += \
        -m elf_i386 \
        -static \
        --build-id=sha1

    ifeq ($(CROSS_LD_HAS_NO_PIE),yes)
        override CROSS_LDFLAGS += -no-pie
    endif
endif

ifeq ($(TARGET),uefi-x86-64)
    override CROSS_LDFLAGS += \
        -m elf_x86_64 \
        -static \
        -pie \
        --no-dynamic-linker \
        -z text
endif

ifeq ($(TARGET),uefi-ia32)
    override CROSS_LDFLAGS += \
        -m elf_i386 \
        -static \
        -pie \
        --no-dynamic-linker \
        -z text
endif

override C_FILES := $(shell find . -type f -name '*.c')
ifeq ($(TARGET),bios)
    override ASMX86_FILES := $(shell find . -type f -name '*.asm_x86')
    override ASM32_FILES := $(shell find . -type f -name '*.asm_ia32')
    override ASMB_FILES := $(shell find . -type f -name '*.asm_bios_ia32')

    override OBJ := $(addprefix $(call MKESCAPE,$(BUILDDIR))/, $(C_FILES:.c=.o) $(ASM32_FILES:.asm_ia32=.o) $(ASMB_FILES:.asm_bios_ia32=.o) $(ASMX86_FILES:.asm_x86=.o))
    override OBJ_S2 := $(filter %.s2.o,$(OBJ))
endif
ifeq ($(TARGET),uefi-x86-64)
    override ASMX86_FILES := $(shell find . -type f -name '*.asm_x86')
    override ASM64_FILES := $(shell find . -type f -name '*.asm_x86_64')
    override ASM64U_FILES := $(shell find . -type f -name '*.asm_uefi_x86_64')

    override OBJ := $(addprefix $(call MKESCAPE,$(BUILDDIR))/, $(C_FILES:.c=.o) $(ASM64_FILES:.asm_x86_64=.o) $(ASM64U_FILES:.asm_uefi_x86_64=.o) $(ASMX86_FILES:.asm_x86=.o))
endif
ifeq ($(TARGET),uefi-ia32)
    override ASMX86_FILES := $(shell find . -type f -name '*.asm_x86')
    override ASM32_FILES := $(shell find . -type f -name '*.asm_ia32')
    override ASM32U_FILES := $(shell find . -type f -name '*.asm_uefi_ia32')

    override OBJ := $(addprefix $(call MKESCAPE,$(BUILDDIR))/, $(C_FILES:.c=.o) $(ASM32_FILES:.asm_ia32=.o) $(ASM32U_FILES:.asm_uefi_ia32=.o) $(ASMX86_FILES:.asm_x86=.o))
endif
override HEADER_DEPS := $(addprefix $(call MKESCAPE,$(BUILDDIR))/, $(C_FILES:.c=.d))

.PHONY: all

ifeq ($(TARGET),bios)
all: $(call MKESCAPE,$(BUILDDIR))/limine.sys $(call MKESCAPE,$(BUILDDIR))/stage2.bin.gz
else ifeq ($(TARGET),uefi-x86-64)
all: $(call MKESCAPE,$(BUILDDIR))/BOOTX64.EFI
else ifeq ($(TARGET),uefi-ia32)
all: $(call MKESCAPE,$(BUILDDIR))/BOOTIA32.EFI
endif

$(call MKESCAPE,$(BUILDDIR))/tinf-copied: ../tinf/*
	rm -rf '$(call SHESCAPE,$(BUILDDIR))/tinf'
	$(MKDIR_P) '$(call SHESCAPE,$(BUILDDIR))'
	cp -r ../tinf '$(call SHESCAPE,$(BUILDDIR))/'
	touch '$(call SHESCAPE,$(BUILDDIR))/tinf-copied'

$(call MKESCAPE,$(BUILDDIR))/tinf/tinfgzip.o: $(call MKESCAPE,$(BUILDDIR))/tinf-copied
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	$(CROSS_CC) $(CROSS_CFLAGS) -c '$(call SHESCAPE,$(@:.o=.c))' -o '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/tinf/tinflate.o: $(call MKESCAPE,$(BUILDDIR))/tinf-copied
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	$(CROSS_CC) $(CROSS_CFLAGS) -c '$(call SHESCAPE,$(@:.o=.c))' -o '$(call SHESCAPE,$@)'

ifeq ($(TARGET),bios)

$(call MKESCAPE,$(BUILDDIR))/stage2.bin.gz: $(call MKESCAPE,$(BUILDDIR))/stage2.bin
	gzip -n -9 < '$(call SHESCAPE,$<)' > '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/stage2.bin: $(call MKESCAPE,$(BUILDDIR))/limine.sys
	dd if='$(call SHESCAPE,$<)' bs=$$(( 0x$$("$(CROSS_READELF)" -S '$(call SHESCAPE,$(BUILDDIR))/limine.elf' | $(GREP) '\.text\.stage3' | $(SED) 's/^.*] //' | $(AWK) '{print $$3}' | $(SED) 's/^0*//') - 0x8000 )) count=1 of='$(call SHESCAPE,$@)' 2>/dev/null

$(call MKESCAPE,$(BUILDDIR))/stage2.map.o: $(call MKESCAPE,$(BUILDDIR))/limine_nomap.elf
	cd '$(call SHESCAPE,$(BUILDDIR))' && \
		'$(call SHESCAPE,$(SRCDIR))/gensyms.sh' '$(call SHESCAPE,$<)' stage2 32 '\.text\.stage2'
	$(CROSS_CC) $(CROSS_CFLAGS) -c '$(call SHESCAPE,$(BUILDDIR))/stage2.map.S' -o '$(call SHESCAPE,$@)'
	rm -f '$(call SHESCAPE,$(BUILDDIR))/stage2.map.S' '$(call SHESCAPE,$(BUILDDIR))/stage2.map.d'

$(call MKESCAPE,$(BUILDDIR))/full.map.o: $(call MKESCAPE,$(BUILDDIR))/limine_nos3map.elf
	cd '$(call SHESCAPE,$(BUILDDIR))' && \
		'$(call SHESCAPE,$(SRCDIR))/gensyms.sh' '$(call SHESCAPE,$<)' full 32 '\.text'
	$(CROSS_CC) $(CROSS_CFLAGS) -c '$(call SHESCAPE,$(BUILDDIR))/full.map.S' -o '$(call SHESCAPE,$@)'
	rm -f '$(call SHESCAPE,$(BUILDDIR))/full.map.S' '$(call SHESCAPE,$(BUILDDIR))/full.map.d'

$(call MKESCAPE,$(BUILDDIR))/limine.sys: $(call MKESCAPE,$(BUILDDIR))/limine_stage2only.elf $(call MKESCAPE,$(BUILDDIR))/limine.elf
	$(CROSS_OBJCOPY) -O binary '$(call SHESCAPE,$(BUILDDIR))/limine.elf' '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/linker_stage2only.ld: linker_bios.ld.in
	$(MKDIR_P) '$(call SHESCAPE,$(BUILDDIR))'
	$(CROSS_CC) -x c -E -P -undef -DLINKER_STAGE2ONLY linker_bios.ld.in -o '$(call SHESCAPE,$(BUILDDIR))/linker_stage2only.ld'

$(call MKESCAPE,$(BUILDDIR))/limine_stage2only.elf: $(OBJ_S2) ../libgcc-binaries/libgcc-i686.a
	$(MAKE) '$(call SHESCAPE,$(BUILDDIR))/linker_stage2only.ld'
	$(CROSS_LD) '$(call OBJESCAPE,$^)' $(CROSS_LDFLAGS) -T'$(call SHESCAPE,$(BUILDDIR))/linker_stage2only.ld' -o '$(call SHESCAPE,$@)' || \
		( echo "This error may mean that stage 2 was trying to use stage 3 symbols before loading stage 3" && \
		  false )

$(call MKESCAPE,$(BUILDDIR))/linker_nos2map.ld: linker_bios.ld.in
	$(MKDIR_P) '$(call SHESCAPE,$(BUILDDIR))'
	$(CROSS_CC) -x c -E -P -undef -DLINKER_NOMAP -DLINKER_NOS2MAP linker_bios.ld.in -o '$(call SHESCAPE,$(BUILDDIR))/linker_nos2map.ld'

$(call MKESCAPE,$(BUILDDIR))/limine_nomap.elf: $(OBJ) ../libgcc-binaries/libgcc-i686.a $(call MKESCAPE,$(BUILDDIR))/tinf/tinfgzip.o $(call MKESCAPE,$(BUILDDIR))/tinf/tinflate.o
	$(MAKE) '$(call SHESCAPE,$(BUILDDIR))/linker_nos2map.ld'
	$(CROSS_LD) '$(call OBJESCAPE,$^)' $(CROSS_LDFLAGS) -T'$(call SHESCAPE,$(BUILDDIR))/linker_nos2map.ld' -o '$(call SHESCAPE,$@)'
	$(CROSS_OBJCOPY) -O binary --only-section=.note.gnu.build-id '$(call SHESCAPE,$@)' '$(call SHESCAPE,$(BUILDDIR))/build-id.s2.bin'
	cd '$(call SHESCAPE,$(BUILDDIR))' && \
		$(CROSS_OBJCOPY) -I binary $(OBJCOPY2ELF_FLAGS) build-id.s2.bin build-id.s2.o
	$(CROSS_OBJCOPY) -O binary --only-section=.note.gnu.build-id '$(call SHESCAPE,$@)' '$(call SHESCAPE,$(BUILDDIR))/build-id.s3.bin'
	cd '$(call SHESCAPE,$(BUILDDIR))' && \
		$(CROSS_OBJCOPY) -I binary $(OBJCOPY2ELF_FLAGS) build-id.s3.bin build-id.s3.o
	$(CROSS_LD) '$(call OBJESCAPE,$^)' '$(call SHESCAPE,$(BUILDDIR))/build-id.s2.o' '$(call SHESCAPE,$(BUILDDIR))/build-id.s3.o' $(CROSS_LDFLAGS) -T'$(call SHESCAPE,$(BUILDDIR))/linker_nos2map.ld' -o '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/linker_nomap.ld: linker_bios.ld.in
	$(MKDIR_P) '$(call SHESCAPE,$(BUILDDIR))'
	$(CROSS_CC) -x c -E -P -undef -DLINKER_NOMAP linker_bios.ld.in -o '$(call SHESCAPE,$(BUILDDIR))/linker_nomap.ld'

$(call MKESCAPE,$(BUILDDIR))/limine_nos3map.elf: $(OBJ) ../libgcc-binaries/libgcc-i686.a $(call MKESCAPE,$(BUILDDIR))/tinf/tinfgzip.o $(call MKESCAPE,$(BUILDDIR))/tinf/tinflate.o $(call MKESCAPE,$(BUILDDIR))/stage2.map.o
	$(MAKE) '$(call SHESCAPE,$(BUILDDIR))/linker_nomap.ld'
	$(CROSS_LD) '$(call OBJESCAPE,$^)' $(CROSS_LDFLAGS) -T'$(call SHESCAPE,$(BUILDDIR))/linker_nomap.ld' -o '$(call SHESCAPE,$@)'
	$(CROSS_OBJCOPY) -O binary --only-section=.note.gnu.build-id '$(call SHESCAPE,$@)' '$(call SHESCAPE,$(BUILDDIR))/build-id.s2.bin'
	cd '$(call SHESCAPE,$(BUILDDIR))' && \
		$(CROSS_OBJCOPY) -I binary $(OBJCOPY2ELF_FLAGS) build-id.s2.bin build-id.s2.o
	$(CROSS_OBJCOPY) -O binary --only-section=.note.gnu.build-id '$(call SHESCAPE,$@)' '$(call SHESCAPE,$(BUILDDIR))/build-id.s3.bin'
	cd '$(call SHESCAPE,$(BUILDDIR))' && \
		$(CROSS_OBJCOPY) -I binary $(OBJCOPY2ELF_FLAGS) build-id.s3.bin build-id.s3.o
	$(CROSS_LD) '$(call OBJESCAPE,$^)' '$(call SHESCAPE,$(BUILDDIR))/build-id.s2.o' '$(call SHESCAPE,$(BUILDDIR))/build-id.s3.o' $(CROSS_LDFLAGS) -T'$(call SHESCAPE,$(BUILDDIR))/linker_nomap.ld' -o '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/linker.ld: linker_bios.ld.in
	$(MKDIR_P) '$(call SHESCAPE,$(BUILDDIR))'
	$(CROSS_CC) -x c -E -P -undef linker_bios.ld.in -o '$(call SHESCAPE,$(BUILDDIR))/linker.ld'

$(call MKESCAPE,$(BUILDDIR))/limine.elf: $(OBJ) ../libgcc-binaries/libgcc-i686.a $(call MKESCAPE,$(BUILDDIR))/tinf/tinfgzip.o $(call MKESCAPE,$(BUILDDIR))/tinf/tinflate.o $(call MKESCAPE,$(BUILDDIR))/stage2.map.o $(call MKESCAPE,$(BUILDDIR))/full.map.o
	$(MAKE) '$(call SHESCAPE,$(BUILDDIR))/linker.ld'
	$(CROSS_LD) '$(call OBJESCAPE,$^)' $(CROSS_LDFLAGS) -T'$(call SHESCAPE,$(BUILDDIR))/linker.ld' -o '$(call SHESCAPE,$@)'
	$(CROSS_OBJCOPY) -O binary --only-section=.note.gnu.build-id '$(call SHESCAPE,$@)' '$(call SHESCAPE,$(BUILDDIR))/build-id.s2.bin'
	cd '$(call SHESCAPE,$(BUILDDIR))' && \
		$(CROSS_OBJCOPY) -I binary $(OBJCOPY2ELF_FLAGS) build-id.s2.bin build-id.s2.o
	$(CROSS_OBJCOPY) -O binary --only-section=.note.gnu.build-id '$(call SHESCAPE,$@)' '$(call SHESCAPE,$(BUILDDIR))/build-id.s3.bin'
	cd '$(call SHESCAPE,$(BUILDDIR))' && \
		$(CROSS_OBJCOPY) -I binary $(OBJCOPY2ELF_FLAGS) build-id.s3.bin build-id.s3.o
	$(CROSS_LD) '$(call OBJESCAPE,$^)' '$(call SHESCAPE,$(BUILDDIR))/build-id.s2.o' '$(call SHESCAPE,$(BUILDDIR))/build-id.s3.o' $(CROSS_LDFLAGS) -T'$(call SHESCAPE,$(BUILDDIR))/linker.ld' -o '$(call SHESCAPE,$@)'

endif

$(call MKESCAPE,$(BUILDDIR))/limine-efi: ../limine-efi/*
	$(MKDIR_P) '$(call SHESCAPE,$(BUILDDIR))'
	cp -r ../limine-efi '$(call SHESCAPE,$(BUILDDIR))/'

ifeq ($(TARGET),uefi-x86-64)

$(call MKESCAPE,$(BUILDDIR))/full.map.o: $(call MKESCAPE,$(BUILDDIR))/limine_nomap.elf
	cd '$(call SHESCAPE,$(BUILDDIR))' && \
		'$(call SHESCAPE,$(SRCDIR))/gensyms.sh' '$(call SHESCAPE,$<)' full 64 '\.text'
	$(CROSS_CC) $(CROSS_CFLAGS) -c '$(call SHESCAPE,$(BUILDDIR))/full.map.S' -o '$(call SHESCAPE,$@)'
	rm -f '$(call SHESCAPE,$(BUILDDIR))/full.map.S' '$(call SHESCAPE,$(BUILDDIR))/full.map.d'

$(call MKESCAPE,$(BUILDDIR))/BOOTX64.EFI: $(call MKESCAPE,$(BUILDDIR))/limine.elf
	$(CROSS_OBJCOPY) -O binary '$(call SHESCAPE,$<)' '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/crt0-efi-x86_64.o $(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/reloc_x86_64.o: $(call MKESCAPE,$(BUILDDIR))/limine-efi
	unset CPPFLAGS; \
	$(MAKE) -C '$(call SHESCAPE,$(BUILDDIR))/limine-efi/gnuefi' \
		CC="$(CROSS_CC)" \
		CFLAGS="$(BASE_CFLAGS)" \
		CPPFLAGS='-nostdinc -I$(call SHESCAPE,$(SRCDIR))/../freestanding-headers' \
		ARCH=x86_64

$(call MKESCAPE,$(BUILDDIR))/linker_nomap.ld: linker_uefi_x86_64.ld.in
	$(MKDIR_P) '$(call SHESCAPE,$(BUILDDIR))'
	$(CROSS_CC) -x c -E -P -undef -DLINKER_NOMAP linker_uefi_x86_64.ld.in -o '$(call SHESCAPE,$(BUILDDIR))/linker_nomap.ld'

$(call MKESCAPE,$(BUILDDIR))/limine_nomap.elf: $(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/crt0-efi-x86_64.o $(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/reloc_x86_64.o $(OBJ) ../libgcc-binaries/libgcc-x86_64-no-red-zone.a $(call MKESCAPE,$(BUILDDIR))/tinf/tinfgzip.o $(call MKESCAPE,$(BUILDDIR))/tinf/tinflate.o
	$(MAKE) '$(call SHESCAPE,$(BUILDDIR))/linker_nomap.ld'
	$(CROSS_LD) \
		-T'$(call SHESCAPE,$(BUILDDIR))/linker_nomap.ld' \
		'$(call OBJESCAPE,$^)' $(CROSS_LDFLAGS) -o '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/linker.ld: linker_uefi_x86_64.ld.in
	$(MKDIR_P) '$(call SHESCAPE,$(BUILDDIR))'
	$(CROSS_CC) -x c -E -P -undef linker_uefi_x86_64.ld.in -o '$(call SHESCAPE,$(BUILDDIR))/linker.ld'

$(call MKESCAPE,$(BUILDDIR))/limine.elf: $(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/crt0-efi-x86_64.o $(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/reloc_x86_64.o $(OBJ) ../libgcc-binaries/libgcc-x86_64-no-red-zone.a $(call MKESCAPE,$(BUILDDIR))/tinf/tinfgzip.o $(call MKESCAPE,$(BUILDDIR))/tinf/tinflate.o $(call MKESCAPE,$(BUILDDIR))/full.map.o
	$(MAKE) '$(call SHESCAPE,$(BUILDDIR))/linker.ld'
	$(CROSS_LD) \
		-T'$(call SHESCAPE,$(BUILDDIR))/linker.ld' \
		'$(call OBJESCAPE,$^)' $(CROSS_LDFLAGS) -o '$(call SHESCAPE,$@)'

endif

ifeq ($(TARGET),uefi-ia32)

$(call MKESCAPE,$(BUILDDIR))/full.map.o: $(call MKESCAPE,$(BUILDDIR))/limine_nomap.elf
	cd '$(call SHESCAPE,$(BUILDDIR))' && \
		'$(call SHESCAPE,$(SRCDIR))/gensyms.sh' '$(call SHESCAPE,$<)' full 32 '\.text'
	$(CROSS_CC) $(CROSS_CFLAGS) -c '$(call SHESCAPE,$(BUILDDIR))/full.map.S' -o '$(call SHESCAPE,$@)'
	rm -f '$(call SHESCAPE,$(BUILDDIR))/full.map.S' '$(call SHESCAPE,$(BUILDDIR))/full.map.d'

$(call MKESCAPE,$(BUILDDIR))/BOOTIA32.EFI: $(call MKESCAPE,$(BUILDDIR))/limine.elf
	$(CROSS_OBJCOPY) -O binary '$(call SHESCAPE,$<)' '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/crt0-efi-ia32.o $(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/reloc_ia32.o: $(call MKESCAPE,$(BUILDDIR))/limine-efi
	unset CPPFLAGS; \
	$(MAKE) -C '$(call SHESCAPE,$(BUILDDIR))/limine-efi/gnuefi' \
		CC="$(CROSS_CC)" \
		CFLAGS="$(BASE_CFLAGS)" \
		CPPFLAGS='-nostdinc -I$(call SHESCAPE,$(SRCDIR))/../freestanding-headers' \
		ARCH=ia32

$(call MKESCAPE,$(BUILDDIR))/linker_nomap.ld: linker_uefi_ia32.ld.in
	$(MKDIR_P) '$(call SHESCAPE,$(BUILDDIR))'
	$(CROSS_CC) -x c -E -P -undef -DLINKER_NOMAP linker_uefi_ia32.ld.in -o '$(call SHESCAPE,$(BUILDDIR))/linker_nomap.ld'

$(call MKESCAPE,$(BUILDDIR))/limine_nomap.elf: $(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/crt0-efi-ia32.o $(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/reloc_ia32.o $(OBJ) ../libgcc-binaries/libgcc-i686.a $(call MKESCAPE,$(BUILDDIR))/tinf/tinfgzip.o $(call MKESCAPE,$(BUILDDIR))/tinf/tinflate.o
	$(MAKE) '$(call SHESCAPE,$(BUILDDIR))/linker_nomap.ld'
	$(CROSS_LD) \
		-T'$(call SHESCAPE,$(BUILDDIR))/linker_nomap.ld' \
		'$(call OBJESCAPE,$^)' $(CROSS_LDFLAGS) -o '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/linker.ld: linker_uefi_ia32.ld.in
	$(MKDIR_P) '$(call SHESCAPE,$(BUILDDIR))'
	$(CROSS_CC) -x c -E -P -undef linker_uefi_ia32.ld.in -o '$(call SHESCAPE,$(BUILDDIR))/linker.ld'

$(call MKESCAPE,$(BUILDDIR))/limine.elf: $(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/crt0-efi-ia32.o $(call MKESCAPE,$(BUILDDIR))/limine-efi/gnuefi/reloc_ia32.o $(OBJ) ../libgcc-binaries/libgcc-i686.a $(call MKESCAPE,$(BUILDDIR))/tinf/tinfgzip.o $(call MKESCAPE,$(BUILDDIR))/tinf/tinflate.o $(call MKESCAPE,$(BUILDDIR))/full.map.o
	$(MAKE) '$(call SHESCAPE,$(BUILDDIR))/linker.ld'
	$(CROSS_LD) \
		-T'$(call SHESCAPE,$(BUILDDIR))/linker.ld' \
		'$(call OBJESCAPE,$^)' $(CROSS_LDFLAGS) -o '$(call SHESCAPE,$@)'

endif

-include $(HEADER_DEPS)

ifeq ($(TARGET),uefi-x86-64)
$(call MKESCAPE,$(BUILDDIR))/%.o: %.c $(call MKESCAPE,$(BUILDDIR))/tinf-copied $(call MKESCAPE,$(BUILDDIR))/limine-efi
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	$(CROSS_CC) $(CROSS_CFLAGS) -c '$(call SHESCAPE,$<)' -o '$(call SHESCAPE,$@)'
endif

ifeq ($(TARGET),uefi-ia32)
$(call MKESCAPE,$(BUILDDIR))/%.o: %.c $(call MKESCAPE,$(BUILDDIR))/tinf-copied $(call MKESCAPE,$(BUILDDIR))/limine-efi
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	$(CROSS_CC) $(CROSS_CFLAGS) -c '$(call SHESCAPE,$<)' -o '$(call SHESCAPE,$@)'
endif

ifeq ($(TARGET),bios)
$(call MKESCAPE,$(BUILDDIR))/%.o: %.c $(call MKESCAPE,$(BUILDDIR))/tinf-copied
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	$(CROSS_CC) $(CROSS_CFLAGS) -c '$(call SHESCAPE,$<)' -o '$(call SHESCAPE,$@)'
endif

-include $(HEADER_DEPS)

ifeq ($(TARGET),bios)
$(call MKESCAPE,$(BUILDDIR))/%.s2.o: %.s2.c
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	$(CROSS_CC) $(CROSS_CFLAGS) $(S2CFLAGS) -c '$(call SHESCAPE,$<)' -o '$(call SHESCAPE,$@)'
endif

-include $(HEADER_DEPS)

ifeq ($(TARGET),uefi-x86-64)
$(call MKESCAPE,$(BUILDDIR))/%.32.o: %.32.c $(call MKESCAPE,$(BUILDDIR))/limine-efi
	mkdir -p "$$(dirname '$(call SHESCAPE,$@)')"
	$(CROSS_CC) $(CROSS_CFLAGS) $(INTERNAL_CFLAGS32) -c '$(call SHESCAPE,$<)' -o '$(call SHESCAPE,$@).32'
	$(CROSS_OBJCOPY) -I elf32-i386 -O elf64-x86-64 '$(call SHESCAPE,$@).32' '$(call SHESCAPE,$@)'
	rm '$(call SHESCAPE,$@).32'
endif

ifeq ($(TARGET),bios)
$(call MKESCAPE,$(BUILDDIR))/%.o: %.asm_ia32
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	nasm '$(call SHESCAPE,$<)' -F dwarf -g $(WERROR) -f elf32 -o '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/%.o: %.asm_bios_ia32
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	nasm '$(call SHESCAPE,$<)' -F dwarf -g $(WERROR) -f elf32 -o '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/%.o: %.asm_x86
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	nasm '$(call SHESCAPE,$<)' -F dwarf -g $(WERROR) -f elf32 -o '$(call SHESCAPE,$@)'
endif

ifeq ($(TARGET),uefi-x86-64)
$(call MKESCAPE,$(BUILDDIR))/%.o: %.asm_x86_64
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	nasm '$(call SHESCAPE,$<)' -F dwarf -g $(WERROR) -f elf64 -o '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/%.o: %.asm_uefi_x86_64
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	nasm '$(call SHESCAPE,$<)' -F dwarf -g $(WERROR) -f elf64 -o '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/%.o: %.asm_x86
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	nasm '$(call SHESCAPE,$<)' -F dwarf -g $(WERROR) -f elf64 -o '$(call SHESCAPE,$@)'
endif

ifeq ($(TARGET),uefi-ia32)
$(call MKESCAPE,$(BUILDDIR))/%.o: %.asm_ia32
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	nasm '$(call SHESCAPE,$<)' -F dwarf -g $(WERROR) -f elf32 -o '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/%.o: %.asm_uefi_ia32
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	nasm '$(call SHESCAPE,$<)' -F dwarf -g $(WERROR) -f elf32 -o '$(call SHESCAPE,$@)'

$(call MKESCAPE,$(BUILDDIR))/%.o: %.asm_x86
	$(MKDIR_P) "$$(dirname '$(call SHESCAPE,$@)')"
	nasm '$(call SHESCAPE,$<)' -F dwarf -g $(WERROR) -f elf32 -o '$(call SHESCAPE,$@)'
endif
