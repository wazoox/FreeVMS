DESTDIR=../../build/freevms

CFLAGS=-Wall -fno-stack-protector -nostdinc -O2 -g -Wall -Wshadow \
		-Wconversion  -Wno-conversion -fno-exceptions
INCLUDES=-I./include \
		-I$(DESTDIR)/../kernel \
		-I$(DESTDIR)/../kernel/include \
		-I/usr/lib/gcc/x86_64-linux-gnu/4.4.5/include

OBJ=$(DESTDIR)/libearly/amd64.o \
	$(DESTDIR)/libearly/get_hex.o \
	$(DESTDIR)/libearly/print.o \
	$(DESTDIR)/root/boot.o \
	$(DESTDIR)/root/init.o \
	$(DESTDIR)/root/parsing.o \
	$(DESTDIR)/vm/vm_init.o \
	$(DESTDIR)/vm/vm_server.o

all: directories $(OBJ) $(DESTDIR)/vmskernel.sys

clean:
	rm -f $(OBJ) freevms

$(DESTDIR)/vmskernel.sys: $(OBJ)
	ld -e_start -N -L$(DESTDIR)/../kernel/build/lib/l4 \
			$(OBJ) -nostdlib -T linker/elf_x86_64 -ll4 -static -o $@

directories:
	mkdir -p $(DESTDIR)/drv
	mkdir -p $(DESTDIR)/libearly
	mkdir -p $(DESTDIR)/root
	mkdir -p $(DESTDIR)/vm

# ./root

$(DESTDIR)/root/init.o: root/init.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/root/parsing.o: root/parsing.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/root/boot.o: root/boot.S
	as $< -o $@

# ./vm

$(DESTDIR)/vm/vm_init.o: vm/vm_init.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/vm/vm_server.o: vm/vm_server.c
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
