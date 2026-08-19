
build:
	make -C kernel
	./iso.sh

run:
	qemu-system-x86_64 -cdrom iso/Yoix.iso -machine q35
	
debug:
	gdb -x gdbscript.gdb

clean:
	rm -rf iso/*
	rm -rf kernel/obj/*
	rm -rf kernel/bin/*
