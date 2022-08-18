all: linux-embedded

linux-embedded:
	./scripts/build.sh


run-linux-embedded:
	qemu-system-x86_64 \
    -kernel images/bzImage \
    -nographic -append "earlyprintk=serial,ttyS0 console=ttyS0" \
		-cpu host,+kvm \
		-enable-kvm \
		-s \
		-m 8G \
		-smp 8 \
		-S 
