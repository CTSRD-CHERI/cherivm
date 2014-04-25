noinst_PROGRAMS = javaio 

javaio_SOURCES = 	java_io_VMConsole.c \
			java_io_VMFile.c \
			java_io_VMObjectInputStream.c \
			java_io_VMObjectStreamClass.c \
			cherijni_names.c

javaio_LDADD = $(top_builddir)/native/jni/classpath/jcl.lo \
		      $(top_builddir)/native/jni/native-lib/libclasspathnative.la \
		       $(top_srcdir)/../libcherijni/src/libcherijni.a \
		       $(top_srcdir)/../libcherijni/src/libc_cheri.a \
		       chsbrt.o

javaio_LDFLAGS = -all-static -nostdlib -Wl,--script=sandbox.ld

chsbrt.o:
	ln -s ../sandbox/chsbrt.o .

libjavaio.so: javaio
	${OBJCOPY} -S -O binary javaio libjavaio.so 

install-exec-local: libjavaio.so
	cp -v libjavaio.so ${libdir}/classpath/

AM_LDFLAGS = @CLASSPATH_MODULE@
AM_CPPFLAGS = @CLASSPATH_INCLUDES@
AM_CFLAGS = @WARNING_CFLAGS@ @STRICT_WARNING_CFLAGS@ @ERROR_CFLAGS@ \
            @EXTRA_CFLAGS@
