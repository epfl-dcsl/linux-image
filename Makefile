all: linux-embedded

linux-embedded:
	./scripts/build.sh


run-linux-embedded:
	qemu-system-x86_64 \
    -kernel images/bzImage \
    -nographic -append "earlyprintk=serial,ttyS0 console=ttyS0" \
		-cpu host,+kvm \
		-machine q35 \
		-accel kvm,kernel-irqchip=split \
		-m 6G \
		-s
