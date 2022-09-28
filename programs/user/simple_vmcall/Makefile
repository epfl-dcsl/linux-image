all: main hello

main: src/main.c
	gcc -nostdlib -no-pie $< -o $@

hello: src/hello.c
	gcc $< -o $@

.PHONY: clean

clean:
	rm main
	rm hello
