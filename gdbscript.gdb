set disassembly-flavor intel
target remote | qemu-system-x86_64 -S -gdb stdio -m 32 -cdrom iso/Yoix.iso -machine q35
file kernel/bin/kernel