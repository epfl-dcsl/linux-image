all: linux-embedded

linux-embedded:
	./scripts/build.sh


run-linux-embedded:
	qemu-system-x86_64 \
    -kernel images/vmlinux \
    -nographic -append "earlyprintk=serial,ttyS0 console=ttyS0" \
		-s \
		-m 8G \
		-smp 8
