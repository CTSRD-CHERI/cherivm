noinst_PROGRAMS = javanio

javanio_SOURCES = gnu_java_nio_VMPipe.c \
			gnu_java_nio_VMChannel.c \
			gnu_java_nio_VMSelector.c \
			gnu_java_nio_charset_iconv_IconvDecoder.c \
			gnu_java_nio_charset_iconv_IconvEncoder.c \
			java_nio_MappedByteBufferImpl.c \
			java_nio_VMDirectByteBuffer.c \
			gnu_java_nio_EpollSelectorImpl.c \
			gnu_java_nio_KqueueSelectorImpl.c \
			javanio.h \
			cherijni_names.c

javanio_LDADD = $(top_builddir)/native/jni/classpath/jcl.lo \
		       $(top_builddir)/native/jni/native-lib/libclasspathnative.la \
		       $(LTLIBICONV) \
		       $(top_srcdir)/../libcherijni/src/libcherijni.a \
		       $(top_srcdir)/../libcherijni/src/libc_cheri.a \
		       chsbrt.o

javanio_LDFLAGS = -all-static -nostdlib -Wl,--script=sandbox.ld

chsbrt.o:
	ln -s ../sandbox/chsbrt.o .

libjavanio.so: javanio
	${OBJCOPY} -S -O binary javanio libjavanio.so 

install-exec-local: libjavanio.so
	cp -v libjavanio.so ${libdir}/classpath/

# Directly included through javanio.h
EXTRA_DIST = javanio.c

AM_LDFLAGS = @CLASSPATH_MODULE@
AM_CPPFLAGS = @CLASSPATH_INCLUDES@
AM_CFLAGS = @WARNING_CFLAGS@ @STRICT_WARNING_CFLAGS@ @ERROR_CFLAGS@ \
            @EXTRA_CFLAGS@
