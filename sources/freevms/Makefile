DESTDIR=../../build/freevms

CFLAGS=-Wall -fno-stack-protector -nostdinc -O2 -g -Wall -Wshadow \
		-Wconversion  -Wno-conversion -fno-exceptions
INCLUDES=-I. -I$(DESTDIR)/../kernel/build/include \
		-I$(DESTDIR)/../kernel/build/include/l4 \
		-I/usr/lib/gcc/x86_64-linux-gnu/4.4.5/include

OBJ=$(DESTDIR)/root/boot.o \
	$(DESTDIR)/root/init.o

all: directories $(OBJ) $(DESTDIR)/vmskernel

clean:
	rm -f $(OBJ) freevms

$(DESTDIR)/vmskernel: $(OBJ)
	ld -e_start -N -L$(DESTDIR)/../kernel/build/lib/l4 \
			$(OBJ) -nostdlib -Ttext=0x01000000  -ll4 -lio -static -o $@

directories:
	mkdir -p $(DESTDIR)/root

$(DESTDIR)/root/init.o: root/init.c
	gcc -x c++ -O2 $(INCLUDES) $(MACROS) $(CFLAGS) -c $< -o $@

$(DESTDIR)/root/boot.o: root/boot.S
	as $< -o $@
