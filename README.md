CheriVM
=======

This is a fork of JamVM and GNU Classpath containing support for sandboxed
native code using CHERI.  It is an ongoing research project and so is currently
missing a lot of features.

How to build
------------

Currently the CHERI version of JamVM can only build on FreeBSD.

You must have a JVM installed to be able to build GNU classpath.  The simplest
way to do this is to install the openjdk6 package:

	$ sudo pkg ins openjdk6

You will also need the `gmake` and `bash` packages installed.

You then need to build the CHERI SDK.  This repository includes a copy of the
script that will do this for you:

	$ ./build_sdk.sh

This will take a while, so now might be a good time to make a cup of tea.  Then
you're ready to configure and build classpath and jamvm:

	$ ./configure.sh
	$ ./make.sh

You should now have a `target` directory containing a JVM that will work on
CHERI.
