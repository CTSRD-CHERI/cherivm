.PHONY: build
build: libPrompt.cheri.so

.PHONY: install
install: build
	mkdir -p "${DIR_CHERIJNI_INSTALL}"
	cp -vf libPrompt.cheri.so "${DIR_CHERIJNI_INSTALL}/"
	cp -vf Prompt.class "${DIR_CHERIJNI_INSTALL}/"

.PHONY: clean
clean:
	rm -f *.class *.o *.bin

Prompt.class: Prompt.java
	javac Prompt.java

Prompt.h: Prompt.class
	javah Prompt

cherijni_guest.o: cherijni_guest.c 
	${CC} ${CFLAGS} -I"${DIR_JAMVM_INCLUDE}" -c cherijni_guest.c

cherijni_javavm.o: cherijni_javavm.c 
	${CC} ${CFLAGS} -I"${DIR_JAMVM_INCLUDE}" -c cherijni_javavm.c

cherijni_jnienv.o: cherijni_jnienv.c 
	${CC} ${CFLAGS} -I"${DIR_JAMVM_INCLUDE}" -c cherijni_jnienv.c

Prompt.o: Prompt.h Prompt.c
	${CC} ${CFLAGS} -I"${DIR_JAMVM_INCLUDE}" -c Prompt.c

chsbrt.o: chsbrt.S
	${CC} ${CCASFLAGS} -c chsbrt.S

Prompt: Prompt.o chsbrt.o sandbox.ld cherijni_guest.o cherijni_javavm.o cherijni_jnienv.o
	${CC} ${CFLAGS} ${LDFLAGS} -o Prompt Prompt.o chsbrt.o cherijni_guest.o cherijni_javavm.o cherijni_jnienv.o libc_cheri.a -Wl,--script=sandbox.ld -nostdlib -all-static

libPrompt.cheri.so: Prompt
	${OBJCOPY} -S -O binary Prompt libPrompt.cheri.so
	
