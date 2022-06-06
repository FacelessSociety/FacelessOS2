for i in $(find ./ -type f -name "*.c")
do
    gcc $i -I \
	    -std=gnu11           \
	    -ffreestanding       \
	    -fno-stack-protector \
	    -fno-pic             \
	    -mabi=sysv           \
	    -mno-80387           \
	    -mno-mmx             \
	    -mno-3dnow           \
	    -mno-sse             \
	    -mno-sse2            \
	    -mno-red-zone        \
	    -mcmodel=kernel      \
	    -MMD                 \
        -Werror-implicit-function-declaration \
        -I src               \
        -c                   \
        -o $i.o
done

for i in $(find ./ -type f -name "*.asm")
do
    nasm $i -felf64 -o $i.o
done 


ld  $(find ./ -type f -name "*.o") -Tlinker.ld            \
	-nostdlib              \
	-zmax-page-size=0x1000 \
	-static                \
    -o kernel.elf          \


