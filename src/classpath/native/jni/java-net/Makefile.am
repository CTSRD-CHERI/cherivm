noinst_PROGRAMS = javanet

if ENABLE_LOCAL_SOCKETS
local_sources = gnu_java_net_local_LocalSocketImpl.c \
		local.c \
		local.h
else
local_sources = gnu_java_net_local_LocalSocketImpl.c
endif

javanet_SOURCES = javanet.c \
			javanet.h \
			java_net_VMInetAddress.c \
			java_net_VMNetworkInterface.c \
			java_net_VMURLConnection.c \
			gnu_java_net_VMPlainSocketImpl.c \
			cherijni_names.c \
			$(local_sources)

javanet_LDADD = $(top_builddir)/native/jni/classpath/jcl.lo \
		       $(top_builddir)/native/jni/native-lib/libclasspathnative.la \
		       $(LIBMAGIC) -lz \
		       $(top_srcdir)/../libcherijni/src/libcherijni.a \
		       $(top_srcdir)/../libcherijni/src/libc_cheri.a \
		       chsbrt.o

javanet_LDFLAGS = -all-static -nostdlib -Wl,--script=sandbox.ld

chsbrt.o:
	ln -s ../sandbox/chsbrt.o .

libjavanet.so: javanet
	${OBJCOPY} -S -O binary javanet libjavanet.so 

install-exec-local: libjavanet.so
	cp -v libjavanet.so ${libdir}/classpath/

AM_LDFLAGS = @CLASSPATH_MODULE@
AM_CPPFLAGS = @CLASSPATH_INCLUDES@
AM_CFLAGS = @WARNING_CFLAGS@ @STRICT_WARNING_CFLAGS@ @ERROR_CFLAGS@ \
            @EXTRA_CFLAGS@
