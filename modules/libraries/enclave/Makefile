TARGET_MODULE:=tyche_enclave

## ———————————————————————— Use Tyche's Linux build ————————————————————————— ##
BUILDSYSTEM_DIR ?= ../../../builds/linux-tyche-embedded/

LIB_PT=../../../programs/libs/pts


$(TARGET_MODULE)-objs := src/driver.o src/ioctl.o src/internal/enclave.o src/internal/process.o src/internal/mapper.o
obj-m := $(TARGET_MODULE).o
foo-obs += $(LIB_PT)/libpts.a
ccflags-y += -I$(src)/$(LIB_PT)/include

## ——————————————————— Where to generate the build files ———————————————————— ##
PWD:=$(shell pwd)


## —————————————————————— Where to install the module ——————————————————————— ##
INSTALL_PATH ?= ../../../builds/initramfs/x86-busybox/home/tyche/drivers/

all : 
# run kernel build system to make module
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules

.PHONY: install 

install: all
	mkdir -p $(INSTALL_PATH)
	cp $(TARGET_MODULE).ko $(INSTALL_PATH) 
	cp $(TARGET_MODULE).sh $(INSTALL_PATH)
	cp $(TARGET_MODULE).mod $(INSTALL_PATH)
	cp Module.symvers $(INSTALL_PATH)
	cp modules.order $(INSTALL_PATH)

clean:
# run kernel build system to cleanup in current directory
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean
