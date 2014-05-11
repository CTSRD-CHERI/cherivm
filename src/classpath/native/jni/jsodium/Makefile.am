nativeexeclib_LTLIBRARIES = libjsodium.la

libjsodium_la_SOURCES = jsodium.c

libjsodium_la_LIBADD = $(top_builddir)/native/jni/classpath/jcl.lo \
		       $(top_builddir)/native/jni/native-lib/libclasspathnative.la \
		       -lsodium 

AM_LDFLAGS = @CLASSPATH_MODULE@
AM_CPPFLAGS = @CLASSPATH_INCLUDES@
AM_CFLAGS = @WARNING_CFLAGS@ @STRICT_WARNING_CFLAGS@ @ERROR_CFLAGS@ \
            @EXTRA_CFLAGS@
