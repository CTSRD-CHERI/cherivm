noinst_PROGRAMS = java

if ENABLE_LOCAL_SOCKETS
local_sources = $(top_builddir)/native/jni/java-net/gnu_java_net_local_LocalSocketImpl.c \
                $(top_builddir)/native/jni/java-net/local.c \
                $(top_builddir)/native/jni/java-net/local.h
else
local_sources = $(top_builddir)/native/jni/java-net/gnu_java_net_local_LocalSocketImpl.c
endif

java_SOURCES = $(top_builddir)/native/jni/java-nio/gnu_java_nio_VMPipe.c \
               $(top_builddir)/native/jni/java-nio/gnu_java_nio_VMChannel.c \
               $(top_builddir)/native/jni/java-nio/gnu_java_nio_VMSelector.c \
               $(top_builddir)/native/jni/java-nio/gnu_java_nio_charset_iconv_IconvDecoder.c \
               $(top_builddir)/native/jni/java-nio/gnu_java_nio_charset_iconv_IconvEncoder.c \
               $(top_builddir)/native/jni/java-nio/java_nio_MappedByteBufferImpl.c \
               $(top_builddir)/native/jni/java-nio/java_nio_VMDirectByteBuffer.c \
               $(top_builddir)/native/jni/java-nio/gnu_java_nio_EpollSelectorImpl.c \
               $(top_builddir)/native/jni/java-nio/gnu_java_nio_KqueueSelectorImpl.c \
               $(top_builddir)/native/jni/java-nio/javanio.h \
               $(top_builddir)/native/jni/java-lang/java_lang_VMSystem.c \
               $(top_builddir)/native/jni/java-lang/java_lang_VMFloat.c \
               $(top_builddir)/native/jni/java-lang/java_lang_VMDouble.c \
               $(top_builddir)/native/jni/java-lang/java_lang_VMMath.c \
               $(top_builddir)/native/jni/java-lang/java_lang_VMProcess.c \
               $(top_builddir)/native/jni/java-io/java_io_VMConsole.c \
               $(top_builddir)/native/jni/java-io/java_io_VMFile.c \
               $(top_builddir)/native/jni/java-io/java_io_VMObjectInputStream.c \
               $(top_builddir)/native/jni/java-io/java_io_VMObjectStreamClass.c \
               $(top_builddir)/native/jni/java-net/javanet.c \
               $(top_builddir)/native/jni/java-net/javanet.h \
               $(top_builddir)/native/jni/java-net/java_net_VMInetAddress.c \
               $(top_builddir)/native/jni/java-net/java_net_VMNetworkInterface.c \
               $(top_builddir)/native/jni/java-net/java_net_VMURLConnection.c \
               $(top_builddir)/native/jni/java-net/gnu_java_net_VMPlainSocketImpl.c \
               $(local_sources) \
               $(top_builddir)/native/jni/java-util/java_util_VMTimeZone.c \
               cherijni_names.c

java_LDADD = $(top_builddir)/native/fdlibm/libfdlibm.la \
             $(top_builddir)/native/jni/classpath/jcl.lo \
             $(top_builddir)/native/jni/native-lib/libclasspathnative.la \
             $(LIBMAGIC) -lz \
             $(LTLIBICONV) \
             $(top_srcdir)/../libcherijni/src/libcherijni.a \
             $(top_srcdir)/../libcherijni/src/libc_cheri.a \
             chsbrt.o

java_LDFLAGS = -all-static -nostdlib -Wl,--script=sandbox.ld

chsbrt.o:
	ln -s ../sandbox/chsbrt.o .

libjava.so: java
	${OBJCOPY} -S -O binary java libjava.so 

install-exec-local: libjava.so
	cp -v libjava.so ${libdir}/classpath/

AM_LDFLAGS = @CLASSPATH_MODULE@
AM_CPPFLAGS = @CLASSPATH_INCLUDES@ -I$(top_srcdir)/native/fdlibm
AM_CFLAGS = @WARNING_CFLAGS@ @STRICT_WARNING_CFLAGS@ @ERROR_CFLAGS@ \
            @EXTRA_CFLAGS@
