DESTDIR=../../build/freevms

CFLAGS=-Wall -fno-stack-protector -nostdinc -O2 -g -Wall -Wshadow \
		-Wconversion  -Wno-conversion -fno-exceptions -fno-omit-frame-pointer
INCLUDES=-I./include \
		-I$(DESTDIR)/../kernel \
		-I$(DESTDIR)/../kernel/include \
		-I/usr/lib/gcc/x86_64-linux-gnu/4.4.5/include

OBJ_SYS=$(DESTDIR)/debug/backtrace.o \
    $(DESTDIR)/debug/sigma0.o \
    $(DESTDIR)/dev/dev_tables.o \
    $(DESTDIR)/elf/elf_loader.o \
    $(DESTDIR)/jobctl/jobctl_bfl.o \
    $(DESTDIR)/jobctl/jobctl_init.o \
    $(DESTDIR)/jobctl/jobctl_quota.o \
    $(DESTDIR)/jobctl/jobctl_thread.o \
    $(DESTDIR)/jobctl/jobctl_rfl.o \
    $(DESTDIR)/libearly/amd64.o \
	$(DESTDIR)/libearly/get_hex.o \
	$(DESTDIR)/libearly/print.o \
	$(DESTDIR)/list/hash.o \
	$(DESTDIR)/list/lists.o \
	$(DESTDIR)/lock/lock_mutex.o \
	$(DESTDIR)/lock/lock_mutex_amd64.o \
	$(DESTDIR)/sec/sec_caps.o \
	$(DESTDIR)/sec/sec_rand.o \
	$(DESTDIR)/sys/sys_boot.o \
	$(DESTDIR)/sys/sys_init.o \
	$(DESTDIR)/sys/sys_loop.o \
	$(DESTDIR)/sys/sys_main.o \
	$(DESTDIR)/sys/sys_parsing.o \
	$(DESTDIR)/vm/vm_alloc.o \
	$(DESTDIR)/vm/vm_btree.o \
	$(DESTDIR)/vm/vm_core.o \
	$(DESTDIR)/vm/vm_init.o \
	$(DESTDIR)/vm/vm_objtables.o \
	$(DESTDIR)/vm/vm_pagefault.o \
	$(DESTDIR)/vm/vm_sigma0.o \
	$(DESTDIR)/vm/vm_slab.o

OBJ_INIT=$(DESTDIR)/init/init.o

OBJ_LIBRTL=$(DESTDIR)/librtl/rtl_print.o

all: directories $(DESTDIR)/vmskernel.sys $(DESTDIR)/init.exe

clean:
	rm -f $(OBJ_SYS) $(OBJ_INIT) $(OBJ_LIBRTL) $(DESTDIR)/init.exe \
			$(DESTDIR)/librtl/librtl.a $(DESTDIR)/vmskernel.sys

$(DESTDIR)/vmskernel.sys: $(OBJ_SYS)
	@rm -f debug/symbols.h
	@echo "static const char *symbols[] = { 0 };" > debug/symbols.h
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c debug/symbols.c \
			-o $(DESTDIR)/debug/symbols.o
	ld -e_start -N -L$(DESTDIR)/../kernel/build/lib/l4 \
			$+ -nostdlib -T linker/elf_x86_64_kernel -ll4 \
			$(DESTDIR)/debug/symbols.o -static -o $@
	linker/symbols > debug/symbols.h
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c debug/symbols.c \
			-o $(DESTDIR)/debug/symbols.o
	ld -e_start -N -L$(DESTDIR)/../kernel/build/lib/l4 \
			$+ -nostdlib -T linker/elf_x86_64_kernel -ll4 \
			$(DESTDIR)/debug/symbols.o -static -o $@

$(DESTDIR)/librtl/librtl.a: $(OBJ_LIBRTL)
	ar -crus $@ $+

$(DESTDIR)/init.exe: $(OBJ_INIT) $(DESTDIR)/librtl/librtl.a
	ld -emain -N -L$(DESTDIR)/../kernel/build/lib/l4\
	   		$+ -nostdlib -T linker/elf_x86_64 -ll4 -static -o $@

directories:
	mkdir -p $(DESTDIR)/debug
	mkdir -p $(DESTDIR)/dev
	mkdir -p $(DESTDIR)/drv
	mkdir -p $(DESTDIR)/elf
	mkdir -p $(DESTDIR)/init
	mkdir -p $(DESTDIR)/jobctl
	mkdir -p $(DESTDIR)/libearly
	mkdir -p $(DESTDIR)/list
	mkdir -p $(DESTDIR)/lock
	mkdir -p $(DESTDIR)/librtl
	mkdir -p $(DESTDIR)/sec
	mkdir -p $(DESTDIR)/sys
	mkdir -p $(DESTDIR)/vm

# ./debug

$(DESTDIR)/debug/backtrace.o: debug/backtrace.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/debug/sigma0.o: debug/sigma0.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./dev

$(DESTDIR)/dev/dev_tables.o: dev/dev_tables.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./elf

$(DESTDIR)/elf/elf_loader.o: elf/elf_loader.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./init

$(DESTDIR)/init/init.o: init/init.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./jobctl

$(DESTDIR)/jobctl/jobctl_init.o: jobctl/jobctl_init.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/jobctl/jobctl_bfl.o: jobctl/jobctl_bfl.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/jobctl/jobctl_rfl.o: jobctl/jobctl_rfl.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/jobctl/jobctl_quota.o: jobctl/jobctl_quota.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/jobctl/jobctl_thread.o: jobctl/jobctl_thread.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./list

$(DESTDIR)/list/hash.o: list/hash.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/list/lists.o: list/lists.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./lock

$(DESTDIR)/lock/lock_mutex.o: lock/lock_mutex.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/lock/lock_mutex_amd64.o: lock/lock_mutex_amd64.S
	as $< -o $@

# ./rtl

$(DESTDIR)/librtl/rtl_print.o: librtl/rtl_print.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./sec

$(DESTDIR)/sec/sec_caps.o: sec/sec_caps.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/sec/sec_rand.o: sec/sec_rand.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./sys

$(DESTDIR)/sys/sys_init.o: sys/sys_init.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/sys/sys_main.o: sys/sys_main.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/sys/sys_loop.o: sys/sys_loop.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/sys/sys_parsing.o: sys/sys_parsing.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/sys/sys_boot.o: sys/sys_boot.S
	as $< -o $@

# ./vm

$(DESTDIR)/vm/vm_init.o: vm/vm_init.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_alloc.o: vm/vm_alloc.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_btree.o: vm/vm_btree.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_objtables.o: vm/vm_objtables.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_pagefault.o: vm/vm_pagefault.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_sigma0.o: vm/vm_sigma0.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_slab.o: vm/vm_slab.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_core.o: vm/vm_core.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./libearly

$(DESTDIR)/libearly/amd64.o: libearly/amd64.cc
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/libearly/get_hex.o: libearly/get_hex.cc
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/libearly/print.o: libearly/print.cc
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./test

$(DESTDIR)/drv/test: $(DESTDIR)/drv/test.o
	ld -emain -N -L$(DESTDIR)/../kernel/build/lib/l4 $+ -nostdlib \
			-ll4 -lio -static -o $@

$(DESTDIR)/drv/test.o: drv/test.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@
