all: main hello

main: src/main.c
	gcc -nostdlib -no-pie $< -o $@

hello: src/hello.c
	gcc $< -o $@

install: main hello
	cp main ../../../builds/initramfs/x86-busybox/home/tyche/
	cp hello ../../../builds/initramfs/x86-busybox/home/tyche/ 

.PHONY: clean

clean:
	rm main
	rm hello
