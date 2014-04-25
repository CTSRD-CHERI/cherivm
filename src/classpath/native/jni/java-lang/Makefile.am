nativeexeclib_LTLIBRARIES = libjavalangreflect.la libjavalangmanagement.la
noinst_PROGRAMS = javalang

javalang_SOURCES = java_lang_VMSystem.c \
			 java_lang_VMFloat.c \
			 java_lang_VMDouble.c \
			 java_lang_VMMath.c \
			 java_lang_VMProcess.c \
			 cherijni_names.c

javalang_LDADD = $(top_builddir)/native/fdlibm/libfdlibm.la \
			$(top_builddir)/native/jni/classpath/jcl.lo \
			$(top_builddir)/native/jni/native-lib/libclasspathnative.la \
		       $(top_srcdir)/../libcherijni/src/libcherijni.a \
		       $(top_srcdir)/../libcherijni/src/libc_cheri.a \
		       chsbrt.o

javalang_LDFLAGS = -all-static -nostdlib -Wl,--script=sandbox.ld

chsbrt.o:
	ln -s ../sandbox/chsbrt.o .

libjavalang.so: javalang
	${OBJCOPY} -S -O binary javalang libjavalang.so 

install-exec-local: libjavalang.so
	cp -v libjavalang.so ${libdir}/classpath/

libjavalangreflect_la_SOURCES = java_lang_reflect_VMArray.c

libjavalangmanagement_la_SOURCES = gnu_java_lang_management_VMOperatingSystemMXBeanImpl.c

AM_LDFLAGS = @CLASSPATH_MODULE@
AM_CPPFLAGS = @CLASSPATH_INCLUDES@ -I$(top_srcdir)/native/fdlibm
AM_CFLAGS = @WARNING_CFLAGS@ @STRICT_WARNING_CFLAGS@ @ERROR_CFLAGS@ \
            @EXTRA_CFLAGS@
