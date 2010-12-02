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
	@echo "make convert            : convert tabs"
	@echo "make start-vb           : start VirtualBox"
	@echo
	@echo "To build FreeVMS, run kernelconfig, kernel, bootstrap, freevms "
	@echo "and userland."

clean:
	rm -rf $(BUILDDIR)

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
			--with-kickstart-linkbase=0x00100000 \
			--with-s0-linkbase=0x00080000 \
			--with-roottask-linkbase=0x00200000 \
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

bootstrap-install:
	@make -C $(BUILDDIR)/kernel install

freevms-clean:
	@make -C $(SRCDIR)/freevms clean

freevms: bootstrap-install
	@make -C $(SRCDIR)/freevms

build/freevms/vmskernel.sys build/freevms/pager.sys build/freevms/init.exe: \
		freevms

image: build/freevms/vmskernel.sys \
		build/freevms/pager.sys \
		build/freevms/init.exe
	mkdir -p ../mnt/boot/grub
	cp -f build/bootloader/stage1/stage1 ../mnt/boot/grub
	cp -f build/bootloader/stage2/stage2 ../mnt/boot/grub
	cp -f build/bootloader/stage2/stage2_eltorito ../mnt/boot/grub
	cp -f build/bootloader/stage2/*stage1_5 ../mnt/boot/grub
	cp -f build/pal/x86-kernel ../mnt/boot/
	cp -f build/kernel/util/kickstart/kickstart ../mnt/boot/
	cp -f build/kernel/build/libexec/l4/sigma0 ../mnt/boot/
	cp -f build/kernel/build/lib/l4/* ../mnt
	cp -f build/freevms/vmskernel.sys ../mnt/boot
	cp -f build/freevms/pager.sys ../mnt/boot
	cp -f build/freevms/init.exe ../mnt/boot

userland:
	@echo "Nothing to do, but you can write it ;-)"

convert:
	find sources/freevms -name "*.[chS]" -exec ./converttab {} \;

../frevms.vdi: ../freevms.img
	sync
	qemu-img convert -O vdi ../freevms.img ../freevms.vdi

start-vb: ../freevms.vdi
	@VBoxManage storageattach FreeVMS --storagectl 'Contrôleur IDE' \
			--port 0 --device 0 --medium none
	@VBoxManage closemedium disk /home/bertrand/openvms/freevms.vdi
	@VBoxManage storageattach FreeVMS --storagectl 'Contrôleur IDE' \
			--port 0 --device 0 --medium /home/bertrand/openvms/freevms.vdi \
			--type hdd
	@VBoxManage startvm FreeVMS
