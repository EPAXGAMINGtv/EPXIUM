# Nuke built-in rules and variables
MAKEFLAGS += -rR
.SUFFIXES:

# Name des Kernels
OUTPUT := kernel

# Installationspfad
PREFIX := /usr/local

# Compiler
CC := cc
CFLAGS := -g -O2 -pipe
CPPFLAGS :=
NASMFLAGS := -F dwarf -g
LDFLAGS :=

# Überprüfe Abhängigkeiten
ifneq ($(shell ( test '$(MAKECMDGOALS)' = clean || test '$(MAKECMDGOALS)' = distclean ); echo $$?),0)
    ifeq ($(shell ( ! test -d freestnd-c-hdrs || ! test -d src/cc-runtime || ! test -f src/limine.h ); echo $$?),0)
        $(error Bitte zuerst ./get-deps ausführen)
    endif
endif

# Clang Target
CC_IS_CLANG := $(shell ! $(CC) --version 2>/dev/null | grep 'clang' >/dev/null 2>&1; echo $$?)
ifeq ($(CC_IS_CLANG),1)
    CC += -target x86_64-unknown-none
endif

# Interne C Flags
CFLAGS += \
    -Wall -Wextra -std=gnu11 -nostdinc -ffreestanding \
    -fno-stack-protector -fno-stack-check -fno-PIC \
    -ffunction-sections -fdata-sections \
    -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone \
    -mcmodel=kernel

CPPFLAGS += -I src -isystem freestnd-c-hdrs -DLIMINE_API_REVISION=3 -MMD -MP
NASMFLAGS += -Wall -f elf64
LDFLAGS += -Wl,-m,elf_x86_64 -Wl,--build-id=none -nostdlib -static -z max-page-size=0x1000 -Wl,--gc-sections -T linker.ld

# Alle Quell-Dateien rekursiv
SRCFILES := $(shell find src -type f | LC_ALL=C sort)
CFILES := $(filter %.c,$(SRCFILES))
ASFILES := $(filter %.S,$(SRCFILES))
NASMFILES := $(filter %.asm,$(SRCFILES))

# Objekt-Dateien inklusive Unterordner
OBJ := $(patsubst src/%.c,obj/%.c.o,$(CFILES)) \
       $(patsubst src/%.S,obj/%.S.o,$(ASFILES)) \
       $(patsubst src/%.asm,obj/%.asm.o,$(NASMFILES))

# Header-Abhängigkeiten
HEADER_DEPS := $(OBJ:.o=.d)

# Default Target
.PHONY: all
all: bin/$(OUTPUT)

-include $(HEADER_DEPS)

# Linken
bin/$(OUTPUT): $(OBJ) linker.ld GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o $@

# C-Dateien kompilieren
obj/%.c.o: src/%.c GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Assembly-Dateien kompilieren
obj/%.S.o: src/%.S GNUmakefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

obj/%.asm.o: src/%.asm GNUmakefile
	mkdir -p "$(dir $@)"
	nasm $(NASMFLAGS) $< -o $@

# Clean
.PHONY: clean
clean:
	rm -rf bin obj

# Distclean
.PHONY: distclean
distclean: clean
	rm -rf freestnd-c-hdrs src/cc-runtime src/limine.h

# Install
.PHONY: install
install: all
	install -d "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)"
	install -m 644 bin/$(OUTPUT) "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/"

# Uninstall
.PHONY: uninstall
uninstall:
	rm -f "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)/$(OUTPUT)"
	-rmdir "$(DESTDIR)$(PREFIX)/share/$(OUTPUT)"
