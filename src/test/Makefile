.PHONY: build
build: CHERI_Prompt.bin

.PHONY: install
install: build
	mkdir -p "${DIR_CHERIJNI_INSTALL}"
	cp -vf CHERI_Prompt.bin "${DIR_CHERIJNI_INSTALL}/CHERI_javanio.bin"

.PHONY: clean
clean:
	rm -f *.class *.o *.bin

Prompt.class: Prompt.java
	javac Prompt.java

Prompt.h: Prompt.class
	javah Prompt

cherijni_guest.o: cherijni_guest.c 
	${CC} ${CFLAGS} -I"${DIR_JAMVM_INCLUDE}" -c cherijni_guest.c

Prompt.o: Prompt.h Prompt.c
	${CC} ${CFLAGS} -I"${DIR_JAMVM_INCLUDE}" -c Prompt.c

chsbrt.o: chsbrt.S
	${CC} ${CCASFLAGS} -c chsbrt.S

Prompt: Prompt.o chsbrt.o sandbox.ld cherijni_guest.o
	${CC} ${CFLAGS} ${LDFLAGS} -o Prompt Prompt.o chsbrt.o cherijni_guest.o libc_cheri.a -Wl,--script=sandbox.ld -nostdlib -all-static

CHERI_Prompt.bin: Prompt
	${OBJCOPY} -S -O binary Prompt CHERI_Prompt.bin
	
