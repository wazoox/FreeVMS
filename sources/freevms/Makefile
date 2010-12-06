DESTDIR=../../build/freevms

CFLAGS=-Wall -fno-stack-protector -nostdinc -O2 -g -Wall -Wshadow \
		-Wconversion  -Wno-conversion -fno-exceptions -fno-omit-frame-pointer \
		-mcmodel=large -m64

GCC_TARGET=$(shell gcc -dumpmachine)
GCC_VERSION=$(shell gcc -dumpversion)

INCLUDES=-I./include \
		-I$(DESTDIR)/../kernel \
		-I$(DESTDIR)/../kernel/include \
		-I/usr/lib/gcc/$(GCC_TARGET)/$(GCC_VERSION)/include

OBJ_SYS=\
    $(DESTDIR)/libearly/amd64.o \
	$(DESTDIR)/libearly/get_hex.o \
	$(DESTDIR)/libearly/print.o \
	$(DESTDIR)/sys/dbg_backtrace.o \
    $(DESTDIR)/sys/dbg_sigma0.o \
    $(DESTDIR)/sys/dev_tables.o \
	$(DESTDIR)/sys/sec_caps.o \
	$(DESTDIR)/sys/sec_rand.o \
	$(DESTDIR)/sys/sys_alloc.o \
    $(DESTDIR)/sys/sys_bfl.o \
	$(DESTDIR)/sys/sys_boot.o \
	$(DESTDIR)/sys/sys_btree.o \
	$(DESTDIR)/sys/sys_core.o \
    $(DESTDIR)/sys/sys_elf.o \
	$(DESTDIR)/sys/sys_hash.o \
	$(DESTDIR)/sys/sys_init.o \
	$(DESTDIR)/sys/sys_lists.o \
	$(DESTDIR)/sys/sys_loop.o \
	$(DESTDIR)/sys/sys_main.o \
	$(DESTDIR)/sys/sys_ms_init.o \
	$(DESTDIR)/sys/sys_mutex.o \
	$(DESTDIR)/sys/sys_mutex_amd64.o \
	$(DESTDIR)/sys/sys_objtables.o \
	$(DESTDIR)/sys/sys_pagefault.o \
	$(DESTDIR)/sys/sys_pager.o \
	$(DESTDIR)/sys/sys_parsing.o \
    $(DESTDIR)/sys/sys_pd_init.o \
    $(DESTDIR)/sys/sys_quota.o \
    $(DESTDIR)/sys/sys_rfl.o \
	$(DESTDIR)/sys/sys_sigma0.o \
	$(DESTDIR)/sys/sys_slab.o \
    $(DESTDIR)/sys/sys_thread.o

OBJ_LIBRTL=\
	$(DESTDIR)/librtl/rtl_print.o \
	$(DESTDIR)/librtl/rtl_sprint.o \
	$(DESTDIR)/librtl/rtl_str.o

OBJ_INIT=\
	$(DESTDIR)/init/init.o

OBJ_PAGER=\
	$(DESTDIR)/vm/vm_pagefault.o \
	$(DESTDIR)/vm/vm_pager.o

all: directories \
	$(DESTDIR)/vmskernel.sys \
	$(DESTDIR)/pager.sys \
	$(DESTDIR)/init.exe

clean:
	@echo  [RM] all generated objects
	@rm -f $(OBJ_SYS) $(OBJ_INIT) $(OBJ_LIBRTL) $(DESTDIR)/init.exe \
			$(DESTDIR)/librtl/librtl.a $(DESTDIR)/vmskernel.sys

$(DESTDIR)/vmskernel.sys: $(OBJ_SYS)
	@rm -f sys/symbols.h
	@echo "static const char *symbols[] = { 0 };" > sys/symbols.h
	@echo  [CC] sys/dbg_symbols.c
	@gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c sys/dbg_symbols.c \
			-o $(DESTDIR)/sys/dbg_symbols.o
	@echo  [LD] $@
	@ld -e_start -N -L$(DESTDIR)/../kernel/build/lib/l4 \
			$+ -nostdlib -T linker/elf_x86_64_kernel -ll4 \
			$(DESTDIR)/sys/dbg_symbols.o -static -o $@
	@echo  [MK] sys/symbols.h
	@linker/symbols > sys/symbols.h
	@echo  [CC] sys/dbg_symbols.c
	@gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c sys/dbg_symbols.c \
			-o $(DESTDIR)/sys/dbg_symbols.o
	@echo  [LD] $@
	@ld -e_start -N -L$(DESTDIR)/../kernel/build/lib/l4 \
			$+ -nostdlib -T linker/elf_x86_64_kernel -ll4 \
			$(DESTDIR)/sys/dbg_symbols.o -static -o $@

$(DESTDIR)/librtl/librtl.a: $(OBJ_LIBRTL)
	@echo  [AR] $@
	@ar -crus $@ $+

$(DESTDIR)/init.exe: $(OBJ_INIT) $(DESTDIR)/librtl/librtl.a
	@echo  [LD] $@
	@ld -emain -N -L$(DESTDIR)/../kernel/build/lib/l4\
	   		$+ -nostdlib -T linker/elf_x86_64 -ll4 -static -o $@

$(DESTDIR)/pager.sys: $(OBJ_PAGER) $(DESTDIR)/librtl/librtl.a
	@echo  [LD] $@
	@ld -emain -N -L$(DESTDIR)/../kernel/build/lib/l4\
	   		$+ -nostdlib -T linker/elf_x86_64_pager -ll4 -static -o $@

directories:
	mkdir -p $(DESTDIR)/drv
	mkdir -p $(DESTDIR)/init
	mkdir -p $(DESTDIR)/libearly
	mkdir -p $(DESTDIR)/librtl
	mkdir -p $(DESTDIR)/sys
	mkdir -p $(DESTDIR)/vm

$(DESTDIR)/libearly/%.o: libearly/%.c
	@echo  [CC] $<
	@gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -o $@ -c $<

# ./sys

$(DESTDIR)/sys/%.o: sys/%.S
	@echo  [AS] $<
	@as -o $@ $<

$(DESTDIR)/sys/%.o: sys/%.c
	@echo  [CC] $<
	@gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -o $@ -c $<

# ./librtl

$(DESTDIR)/librtl/%.o: librtl/%.c
	@echo  [CC] $<
	@gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -o $@ -c $<

# ./init

$(DESTDIR)/init/%.o: init/%.c
	@echo  [CC] $<
	@gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -o $@ -c $<

# ./vm

$(DESTDIR)/vm/%.o: vm/%.c
	@echo  [CC] $<
	@gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -o $@ -c $<

# ./test

$(DESTDIR)/drv/test: $(DESTDIR)/drv/test.o
	ld -emain -N -L$(DESTDIR)/../kernel/build/lib/l4 $+ -nostdlib \
			-ll4 -lio -static -o $@

$(DESTDIR)/drv/test.o: drv/test.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@
