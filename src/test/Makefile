.PHONY: build
build: Prompt.so

.PHONY: install
install: build
	mkdir -p "${DIR_CHERIJNI_INSTALL}"
	cp -v *.class "${DIR_CHERIJNI_INSTALL}/"
	cp -v *.so "${DIR_CHERIJNI_INSTALL}/"

.PHONY: clean
clean:
	rm *.class *.so

Prompt.class: Prompt.java
	javac Prompt.java

Prompt.h: Prompt.class
	javah Prompt

Prompt.so: Prompt.h Prompt.c
	${CC} ${CFLAGS} ${LDFLAGS} -I"${DIR_JAMVM_INCLUDE}" -shared -o Prompt.so Prompt.c

