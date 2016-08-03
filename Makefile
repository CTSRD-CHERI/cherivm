INSTALLED_CLASSPATH=/exports/users/dc552/cheriroot/opt/target/share/jamvm/classes.zip:/exports/users/dc552/cheriroot/opt/target/share/classpath/glibj.zip:/exports/users/dc552/cheriroot/opt/target/share/classpath/tools.zip:/exports/users/dc552/cheriroot/opt/target/share/jamvm/:. 


SDK_ROOT=/home/dc552/sdk
SYSROOT=${SDK_ROOT}/sysroot

CFLAGS=-I ../opt/target/include/ -msoft-float -O2 -cheri-linker
SANDBOX_LDFLAGS=-Wl,--script=${SYSROOT}/usr/libdata/ldscripts/sandbox.ld -static -nostdlib -lc_cheri -lcheri 
SANDBOX_CFLAGS=${CFLAGS} -cheri-linker -mabi=sandbox
CC=${SDK_ROOT}/bin/clang


all: sandbox/test.co sandbox/bench.co sandbox/libbench.so sandbox/test.dump sandbox/bench.dump sandbox/Sandboxed.class sandbox/BenchmarkMultiply.class sandbox/BenchmarkZlib.class sandbox/bench.S sandbox/bench_unsafe.S

clean:
	rm -f sandbox/test.co sandbox/bench.co sandbox/*.class sandbox/libbench.so sandbox/sandbox_*.h sandbox/*.dump sandbox/bench.S sandbox/bench_unsafe.S

sandbox/test.co: sandbox/sandbox_Sandboxed.c
	${CC} ${SANDBOX_CFLAGS} -o sandbox/test.co sandbox/sandbox_Sandboxed.c -DJNI_SANDBOX_CLASS=test ${SANDBOX_LDFLAGS}

sandbox/bench.co: sandbox/bench.c sandbox/bench_unsafe.c
	${CC}  ${SANDBOX_CFLAGS}  -o sandbox/bench.co sandbox/bench.c  ${SANDBOX_LDFLAGS} -DJNI_SANDBOX_CLASS=bench -lz

sandbox/bench.S: sandbox/bench.c sandbox/bench_unsafe.c
	${CC}  ${SANDBOX_CFLAGS} sandbox/bench.c -DJNI_SANDBOX_CLASS=bench -S -o sandbox/bench.S

sandbox/bench_unsafe.S: sandbox/bench_unsafe.c
	${CC} ${CFLAGS} sandbox/bench_unsafe.c -S -o sandbox/bench_unsafe.S

sandbox/bench.ll: sandbox/bench.c sandbox/bench_unsafe.c
	${CC}  ${SANDBOX_CFLAGS} sandbox/bench.c -DJNI_SANDBOX_CLASS=bench -S -o sandbox/bench.ll -emitllvm

sandbox/bench_unsafe.ll: sandbox/bench_unsafe.c
	${CC} ${CFLAGS} sandbox/bench_unsafe.c -S -o sandbox/bench_unsafe.ll -emitllvm


sandbox/libbench.so: sandbox/bench_unsafe.c
	${CC} -mabi=n64 -shared -o sandbox/libbench.so sandbox/bench_unsafe.c -I ../opt/target/include/ -msoft-float  -O2 -lz

sandbox/test.dump: sandbox/test.co
	${SDK_ROOT}/bin/llvm-objdump -triple cheri-unknown-freebsd -d sandbox/test.co > sandbox/test.dump

sandbox/bench.dump: sandbox/bench.co
	${SDK_ROOT}/bin/llvm-objdump -triple cheri-unknown-freebsd -d sandbox/bench.co > sandbox/bench.dump

sandbox/Sandboxed.class: sandbox/Sandboxed.java
	javac -bootclasspath ${INSTALLED_CLASSPATH} sandbox/Sandboxed.java

sandbox/BenchmarkMultiply.class: sandbox/BenchmarkMultiply.java
	javac -bootclasspath ${INSTALLED_CLASSPATH} sandbox/BenchmarkMultiply.java

sandbox/BenchmarkZlib.class: sandbox/BenchmarkZlib.java
	javac -bootclasspath ${INSTALLED_CLASSPATH} sandbox/BenchmarkZlib.java


headers: sandbox/Sandboxed.class sandbox/BenchmarkMultiply.class sandbox/BenchmarkZlib.class
	javah sandbox.Sandboxed
	javah sandbox.BenchmarkMultiply
	javah sandbox.BenchmarkZlib
	mv sandbox_*.h sandbox/
