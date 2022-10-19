all: encl_loader.so encl.so

# The folders we depend on.
ELF_LIB=../elf64
ENCL_DRIVER=../../../modules/libraries/enclave
CAPA_LIB=../../../programs/libs/capabilities

# The includes we depend on.
INCLUDES=-I$(ELF_LIB)/include -I$(ENCL_DRIVER)/include -I$(CAPA_LIB)/include

# Configs for dyn libs.
ELF_CONFIG= -Wl,-R $(ELF_LIB) -L$(ELF_LIB)

encl_loader.so: loader/lib.c
	gcc $(INCLUDES) -Iinclude/ $(ELF_CONFIG) -shared -o $@ -fPIC $^ -lelf64

encl.so: runtime/lib.c runtime/entry.S
	gcc -Iinclude/ -shared -o $@ -fPIC $^

.PHONY: clean

clean:
	rm -rf encl_loader.so encl.so
