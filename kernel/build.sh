git clone https://github.com/limine-bootloader/limine.git --branch=v2.0-branch-binary --depth=1 limine

cp -v kernel.elf limine.cfg limine/limine.sys \
      limine/limine-cd.bin limine/limine-eltorito-efi.bin FacelessOS


xorriso -as mkisofs -b limine-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-eltorito-efi.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        FacelessOS -o FacelessOS.iso
