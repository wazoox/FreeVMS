PWD=$(shell pwd)
BUILDDIR=$(PWD)/build
SRCDIR=$(PWD)/sources

all:
	@echo "This is main makefile required to build FreeVMS."
	@echo "Today, only 64 bits x86 are supported."
	@echo
	@echo "This makefile is able to build FreeVMS distribution that"
	@echo "contains kernel, system and userland."
	@echo
	@echo "Available targets:"
	@echo "make clean              : purge build tree"
	@echo "make kernelconfig       : configure kernel"
	@echo "make kernel             : build kernel"
	@echo "make bootstrap          : build bootstrap code (sigma0 + kickstart)"
	@echo "make bootstrap-clean    : clean bootstrap code"
	@echo "make bootstrap-install  : install bootstrap code"
	@echo "make freevms            : build system"
	@echo "make freevms-clean      : clean system"
	@echo "make freevms-install    : install system"
	@echo "make userland           : build userland"
	@echo "make image              : install system in ../mnt (loop device)"
	@echo
	@echo "To build FreeVMS, run kernelconfig, kernel, bootstrap, freevms "
	@echo "and userland."

clean:
	@rm -rf $(BUILDDIR)œ

kernelconfig:
	@rm -rf $(BUILDDIR)/pal
	@mkdir -p $(BUILDDIR)
	@make -C sources/pal BUILDDIR=$(BUILDDIR)/pal
	@make -C $(BUILDDIR)/pal menuconfig

kernel:
	@make -C $(BUILDDIR)/pal

$(SRCDIR)/kernel/config.h.in:
	@(cd $(SRCDIR)/kernel && autoheader && autoconf)

$(BUILDDIR)/kernel/Makefile: Makefile
	@mkdir -p $(BUILDDIR)/kernel
	@(cd $(BUILDDIR)/kernel && $(SRCDIR)/kernel/configure \
			--with-kickstart-linkbase=0x00020000 \
			--with-s0-linkbase=0x00040000 \
			--with-roottask-linkbase=0x01000000 \
			--prefix=$(BUILDDIR)/kernel/build)

$(BUILDDIR)/bootloader/Makefile: Makefile
	@mkdir -p $(BUILDDIR)/bootloader
	@(cd $(BUILDDIR)/bootloader && $(SRCDIR)/bootloader/configure \
			--prefix=$(BUILDDIR)/bootloader/build)

bootstrap-clean: $(BUILDDIR)/kernel/Makefile $(BUILDDIR)/bootloader/Makefile
	@make -C $(BUILDDIR)/kernel clean
	@make -C $(BUILDDIR)/bootloader clean

bootstrap: $(SRCDIR)/kernel/config.h.in $(BUILDDIR)/kernel/Makefile \
		$(BUILDDIR)/bootloader/Makefile
	@make -C $(BUILDDIR)/kernel
	@make -C $(BUILDDIR)/bootloader

bootstrap-install: freevms
	@make -C $(BUILDDIR)/kernel install

freevms-clean:
	@make -C $(SRCDIR)/freevms clean

freevms:
	@make -C $(SRCDIR)/freevms

image:
	mkdir -p ../mnt/boot/grub
	cp -f build/bootloader/stage1/stage1 ../mnt/boot/grub
	cp -f build/bootloader/stage2/stage2 ../mnt/boot/grub
	cp -f build/bootloader/stage2/stage2_eltorito ../mnt/boot/grub
	cp -f build/bootloader/stage2/*stage1_5 ../mnt/boot/grub
	cp -f build/pal/x86-kernel ../mnt/boot
	cp -f build/kernel/build/libexec/l4/sigma0 ../mnt/boot
	cp -f build/kernel/build/lib/l4/* ../mnt
	cp -f build/freevms/vmskernel.sys ../mnt/boot

userland:
	@echo "Nothing to do, but you can write it ;-)"
