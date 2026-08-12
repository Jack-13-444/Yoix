hdd()
{
	make -p iso/
	dd if=/dev/zero bs=1M count=0 seek=64 of=iso/Yoix.hdd
	PATH=$PATH:/usr/sbin:/sbin sgdisk iso/Yoix.hdd -n 1:2048 -t 1:ef00 -m 1
	./limine-binary/limine bios-install iso/Yoix.hdd
	mformat -i iso/Yoix.hdd@@1M

	# Make relevant subdirectories.
	mmd -i iso/Yoix.hdd@@1M ::/EFI ::/EFI/BOOT ::/boot ::/boot/limine

	# Copy over the relevant files.
	mcopy -i iso/Yoix.hdd@@1M kernel/bin/kernel ::/boot
	mcopy -i iso/Yoix.hdd@@1M limine.conf limine-binary/limine-bios.sys ::/boot/limine
	mcopy -i iso/Yoix.hdd@@1M limine-binary/BOOTX64.EFI ::/EFI/BOOT
	mcopy -i iso/Yoix.hdd@@1M limine-binary/BOOTIA32.EFI ::/EFI/BOOT
}
target_make_iso() 
{

    # Create a directory which will be our ISO root.
	
    mkdir -p target/
	
    # Copy the relevant files over.
    mkdir -p target/boot
	cp -v kernel/bin/kernel target/boot/
	mkdir -p target/boot/limine
	cp -v limine.conf limine-binary/limine-bios.sys limine-binary/limine-bios-cd.bin \
		limine-binary/limine-uefi-cd.bin target/boot/limine/

	mkdir -p target/EFI/BOOT
	cp -v limine-binary/BOOTX64.EFI target/EFI/BOOT/
	cp -v limine-binary/BOOTIA32.EFI target/EFI/BOOT/

    # Create the bootable ISO.
    xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
            -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
            -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
            -efi-boot-part --efi-boot-image --protective-msdos-label \
            target -o iso/Yoix.iso

    # Install Limine stage 1 and 2 for legacy BIOS boot.
    ./limine-binary/limine bios-install iso/Yoix.iso   
}
hdd
target_make_iso