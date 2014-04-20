.PHONY: build
build: libjavanio.so

.PHONY: install
install: build
	mkdir -p "${DIR_CHERIJNI_INSTALL}"
	cp -vf libjavanio.so "${DIR_CHERIJNI_INSTALL}/"

.PHONY: clean
clean:
	rm -f *.class *.o *.bin

cherijni_names.o: cherijni_names.c 
	${CC} ${CFLAGS} -I../libcherijni/src -c cherijni_names.c

chsbrt.o: chsbrt.S
	${CC} ${CCASFLAGS} -c chsbrt.S

javanio: cherijni_names.o chsbrt.o ../libcherijni/src/libcherijni.a ../libcherijni/src/libc_cheri.a
	${CC} ${CFLAGS} ${LDFLAGS} -o javanio cherijni_names.o chsbrt.o ../libcherijni/src/libcherijni.a ../libcherijni/src/libc_cheri.a -Wl,--script=sandbox.ld -nostdlib -all-static

libjavanio.so: javanio
	${OBJCOPY} -S -O binary javanio libjavanio.so
