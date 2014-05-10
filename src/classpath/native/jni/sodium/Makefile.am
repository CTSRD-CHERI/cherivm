noinst_PROGRAMS = jsodium

jsodium_SOURCES = 	jsodium.c \
			cherijni_names.c

jsodium_LDADD = $(LIBSODIUM) \
		       $(top_srcdir)/../libcherijni/src/libcherijni.a \
		       $(top_srcdir)/../libcherijni/src/libc_cheri.a \
		       chsbrt.o

jsodium_LDFLAGS = -all-static -nostdlib -Wl,--script=sandbox.ld

chsbrt.o:
	ln -s ../sandbox/chsbrt.o .

libjsodium.so: jsodium
	${OBJCOPY} -S -O binary jsodium libjsodium.so 

install-exec-local: libjsodium.so
	cp -v libjsodium.so ${libdir}/classpath/

AM_LDFLAGS = @CLASSPATH_MODULE@
AM_CPPFLAGS = @CLASSPATH_INCLUDES@
AM_CFLAGS = @WARNING_CFLAGS@ @STRICT_WARNING_CFLAGS@ @ERROR_CFLAGS@ \
            @EXTRA_CFLAGS@
