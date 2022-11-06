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

ubuntu_mount:
	sudo ./scripts/mount_ubuntu.sh mount ../ubuntu.qcow2 /tmp/mount

ubuntu_umount:
	sudo ./scripts/mount_ubuntu.sh umount ../ubuntu.qcow2 /tmp/mount

# Make sure you mount the disk first
refresh_disk:
	make -B -C modules/libraries/enclave install_disk
	make -B -C modules/libraries/tyche-capabilities install_disk
	make -B -C programs/user/enclave install_disk
	cp modules/scripts/install_drivers.sh /tmp/mount/tyche/
	make linux-embedded
