CHERI JNI Evaluation
====================


This branch includes the evaluation material for the CHERI JNI prototype.
This includes a set of benchmarks and a feature test program.

Building
--------

The `Makefile` expects two variables to be defined on the command line:

- `PREFIX` should point to where you have built CHERI JamVM (from the master branch of this repository).
  By default, this will be the `target` subdirectory of your build directory.
- `SDK_ROOT` should point to where you have built the CHERI SDK.

The `make` command should produce several `.class`, `.co` and `.so` files.
All of the Java classes are outside of any package, so there is no requirement to create a complex directory structure to run them: simply copy the built files to a single directory on your CHERI system.

Running
-------

To run these tests, you must have installed the JamVM from the master branch of the repository onto a CheriBSD system.
I recommend creating a `java` shell script along the lines of the one below.

```sh
#!/bin/sh
# Change this to where you've installed CHERI JamVM:
INSTALL_PATH=/opt/target

# Don't drop core files if things go wrong (it's very slow on CHERI!)
ulimit -c 0
# Look for libraries used to implement the system classes in their installed location
export LD_LIBRARY_PATH=${INSTALL_PATH}/lib/classpath
# Look for Java system classes in their installed location
export BOOTCLASSPATH=${INSTALL_PATH}/share/jamvm/classes.zip:${INSTALL_PATH}/share/classpath/glibj.zip:${INSTALL_PATH}/share/classpath/tools.zip:${INSTALL_PATH}/share/jamvm/
# The tests put the CHERI classes in the current directory, so tell the VM to look for them there
export LIBCHERI_LIBRARY_PATH=.
# Run JamVM with a larger heap size than the default (larger than this crashes
# on the DE4 with out-of-memory- problems.
${INSTALL_PATH}/bin/jamvm -Xmx493m $@
```

With this command, you can now run the tests.
You should begin by running the functionality tests.
From the tests' install directory on your CheriBSD system, run the `SandboxTest` class:

```sh
$ java SandboxTest
Starting...
Running tests...
[1/17] testArguments...passed
[2/17] testPersistentSandbox...passed
[3/17] testSandboxReset...passed
[4/17] testArrayLeak...passed
[5/17] testArrayAccess...passed
[6/17] testInvalidArrayAccess...passed
[7/17] testObjectState...passed
[8/17] testfieldreflection...passed
[9/17] testReflectionTypeSafety...passed
[10/17] testBuffer...passed
[11/17] testCallback...passed
[12/17] testRevokeResetSecurityManager...passed
[13/17] testSyscallGetPid...passed
[14/17] testSyscallOpen...passed
[15/17] testSyscallRead...passed
[16/17] testSyscallWrite...passed
[17/17] testSyscallKqueue...passed
17 of 17 tests passed.
```

If all of the tests pass, then you can try reproducing the benchmarks.
`BenchmarkMultiply` and `BenchmarkZlib` each run benchmarks multiple times with different data sizes.
Each run will generate a single line of a CSV file containing all of the CHERI performance counters.
The `cat_results.sh` script in this repository will combine these into a single CSV file.
Run it with the prefix (`Multiply` or `zlib`) as the argument.


