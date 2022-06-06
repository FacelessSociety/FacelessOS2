all:
	cd kernel/; rm -rf limine 2> /dev/null; make clean; bash buildall.sh
	cd kernel/; bash build.sh; mv FacelessOS.iso ../
	@ rm $$(find ./ -type f -name "*.d")
	@ rm $$(find ./ -type f -name "*.o")

reset:
	cd kernel/; make clean; rm -rf limine
	@ rm $$(find ./ -type f -name "*.d")
	@ rm $$(find ./ -type f -name "*.o")



run:
	qemu-system-x86_64 -cdrom FacelessOS.iso -monitor stdio -d int -D logfile.txt

runfs:
	qemu-system-x86_64 -cdrom FacelessOS.iso -full-screen
