DESTDIR=../../build/freevms

CFLAGS=-Wall -fno-stack-protector -nostdinc -O2 -g -Wall -Wshadow \
		-Wconversion  -Wno-conversion -fno-exceptions
INCLUDES=-I. -I./lib \
		-I$(DESTDIR)/../kernel \
		-I$(DESTDIR)/../kernel/build/include \
		-I$(DESTDIR)/../kernel/build/include/l4 \
		-I/usr/lib/gcc/x86_64-linux-gnu/4.4.5/include

OBJ=$(DESTDIR)/root/boot.o \
	$(DESTDIR)/root/init.o \
	$(DESTDIR)/lib/amd64.o \
	$(DESTDIR)/lib/get_hex.o \
	$(DESTDIR)/lib/print.o

all: directories $(OBJ) $(DESTDIR)/vmskernel.sys

clean:
	rm -f $(OBJ) freevms

$(DESTDIR)/vmskernel.sys: $(OBJ)
	ld -e_start -N -L$(DESTDIR)/../kernel/build/lib/l4 \
			$(OBJ) -nostdlib -Ttext=0x01000000  -ll4 -static -o $@

directories:
	mkdir -p $(DESTDIR)/root
	mkdir -p $(DESTDIR)/drv
	mkdir -p $(DESTDIR)/lib

# ./root

$(DESTDIR)/root/init.o: root/init.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/root/boot.o: root/boot.S
	as $< -o $@

# ./lib

$(DESTDIR)/lib/amd64.o: lib/amd64.cc
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/lib/get_hex.o: lib/get_hex.cc
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

$(DESTDIR)/lib/print.o: lib/print.cc
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@

# ./test

$(DESTDIR)/drv/test: $(DESTDIR)/drv/test.o
	ld -emain -N -L$(DESTDIR)/../kernel/build/lib/l4 $+ -nostdlib \
			-ll4 -lio -static -o $@

$(DESTDIR)/drv/test.o: drv/test.c
	gcc -x c++ -O2 $(INCLUDES) $(CFLAGS) -c $< -o $@
