DESTDIR=../../build/freevms

CFLAGS=-Wall -fno-stack-protector -nostdinc -O2 -g -Wall -Wshadow \
		-Wconversion  -Wno-conversion -fno-exceptions -fno-omit-frame-pointer
INCLUDES=-I./include \
		-I$(DESTDIR)/../kernel \
		-I$(DESTDIR)/../kernel/include \
		-I/usr/lib/gcc/x86_64-linux-gnu/4.4.5/include

OBJ=$(DESTDIR)/debug/backtrace.o \
    $(DESTDIR)/debug/sigma0.o \
    $(DESTDIR)/jobctl/jobctl_init.o \
    $(DESTDIR)/libearly/amd64.o \
	$(DESTDIR)/libearly/get_hex.o \
	$(DESTDIR)/libearly/print.o \
	$(DESTDIR)/sys/boot.o \
	$(DESTDIR)/sys/init.o \
	$(DESTDIR)/sys/parsing.o \
	$(DESTDIR)/vm/vm_alloc.o \
	$(DESTDIR)/vm/vm_btree.o \
	$(DESTDIR)/vm/vm_core.o \
	$(DESTDIR)/vm/vm_init.o \
	$(DESTDIR)/vm/vm_objtables.o \
	$(DESTDIR)/vm/vm_server.o \
	$(DESTDIR)/vm/vm_sigma0.o \
	$(DESTDIR)/vm/vm_slab.o

all: directories $(OBJ) $(DESTDIR)/vmskernel.sys

clean:
	rm -f $(OBJ) freevms

$(DESTDIR)/vmskernel.sys: $(OBJ)
	@rm -f debug/symbols.h
	@echo "static const char *symbols[] = { 0 };" > debug/symbols.h
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c debug/symbols.c \
			-o $(DESTDIR)/debug/symbols.o
	ld -e_start -N -L$(DESTDIR)/../kernel/build/lib/l4 \
			$+ -nostdlib -T linker/elf_x86_64 -ll4 \
			$(DESTDIR)/debug/symbols.o -static -o $@
	linker/symbols > debug/symbols.h
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c debug/symbols.c \
			-o $(DESTDIR)/debug/symbols.o
	ld -e_start -N -L$(DESTDIR)/../kernel/build/lib/l4 \
			$+ -nostdlib -T linker/elf_x86_64 -ll4 \
			$(DESTDIR)/debug/symbols.o -static -o $@

directories:
	mkdir -p $(DESTDIR)/debug
	mkdir -p $(DESTDIR)/drv
	mkdir -p $(DESTDIR)/jobctl
	mkdir -p $(DESTDIR)/libearly
	mkdir -p $(DESTDIR)/sys
	mkdir -p $(DESTDIR)/vm

# ./debug

$(DESTDIR)/debug/backtrace.o: debug/backtrace.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/debug/sigma0.o: debug/sigma0.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./jobctl

$(DESTDIR)/jobctl/jobctl_init.o: jobctl/jobctl_init.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./sys

$(DESTDIR)/sys/init.o: sys/init.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/sys/parsing.o: sys/parsing.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/sys/boot.o: sys/boot.S
	as $< -o $@

# ./vm

$(DESTDIR)/vm/vm_init.o: vm/vm_init.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_server.o: vm/vm_server.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_alloc.o: vm/vm_alloc.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_btree.o: vm/vm_btree.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_objtables.o: vm/vm_objtables.c
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
