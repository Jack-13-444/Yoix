set disassembly-flavor intel
target remote | qemu-system-x86_64 -S -gdb stdio -m 32 -cdrom Yoix.iso
file kernel/bin/kernel