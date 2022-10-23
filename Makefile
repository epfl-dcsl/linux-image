all: linux-embedded

refresh:
	make -B -C modules/libraries/enclave install
	make -B -C modules/libraries/tyche-capabilities install
	make -B -C programs/user/enclave install
	make linux-embedded

linux-embedded:
	./scripts/setup.sh repack
	./scripts/build.sh


run-linux-embedded: linux-embedded
	qemu-system-x86_64 \
    -kernel images/bzImage \
    -nographic -append "earlyprintk=serial,ttyS0 console=ttyS0" \
		-cpu host,+kvm \
		-machine q35 \
		-accel kvm,kernel-irqchip=split \
		-m 6G \
		-s
