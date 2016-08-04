PREFIX=/exports/users/dc552/cheriroot/opt/target
INSTALLED_CLASSPATH=/exportl/users/dc552/cheriroot/opt/target/share/jamvm/classes.zip:${PREFIX}/share/classpath/glibj.zip:${PREFIX}/share/classpath/tools.zip:${PREFIX}/share/jamvm/:. 


SDK_ROOT=/home/dc552/sdk
SYSROOT=${SDK_ROOT}/sysroot

CFLAGS=-I ../opt/target/include/ -msoft-float -O2 -cheri-linker
SANDBOX_LDFLAGS=-Wl,--script=${SYSROOT}/usr/libdata/ldscripts/sandbox.ld -static -nostdlib -lc_cheri -lcheri 
SANDBOX_CFLAGS=${CFLAGS} -cheri-linker -mabi=sandbox
CC=${SDK_ROOT}/bin/clang


all: test.co bench.co libbench.so test.dump bench.dump SandboxTest.class BenchmarkMultiply.class BenchmarkZlib.class bench.S bench_unsafe.S

clean:
	rm -f test.co bench.co *.class libbench.so sandbox_*.h *.dump bench.S bench_unsafe.S

test.co: SandboxTest.c
	${CC} ${SANDBOX_CFLAGS} -o test.co SandboxTest.c -DJNI_SANDBOX_CLASS=test ${SANDBOX_LDFLAGS}

bench.co: bench.c bench_unsafe.c
	${CC}  ${SANDBOX_CFLAGS}  -o bench.co bench.c  ${SANDBOX_LDFLAGS} -DJNI_SANDBOX_CLASS=bench -lz

bench.S: bench.c bench_unsafe.c
	${CC}  ${SANDBOX_CFLAGS} bench.c -DJNI_SANDBOX_CLASS=bench -S -o bench.S

bench_unsafe.S: bench_unsafe.c
	${CC} ${CFLAGS} bench_unsafe.c -S -o bench_unsafe.S

bench.ll: bench.c bench_unsafe.c
	${CC}  ${SANDBOX_CFLAGS} bench.c -DJNI_SANDBOX_CLASS=bench -S -o bench.ll -emitllvm

bench_unsafe.ll: bench_unsafe.c
	${CC} ${CFLAGS} bench_unsafe.c -S -o bench_unsafe.ll -emitllvm


libbench.so: bench_unsafe.c
	${CC} -mabi=n64 -shared -o libbench.so bench_unsafe.c -I ../opt/target/include/ -msoft-float  -O2 -lz

test.dump: test.co
	${SDK_ROOT}/bin/llvm-objdump -triple cheri-unknown-freebsd -d test.co > test.dump

bench.dump: bench.co
	${SDK_ROOT}/bin/llvm-objdump -triple cheri-unknown-freebsd -d bench.co > bench.dump

SandboxTest.class: SandboxTest.java
	javac -bootclasspath ${INSTALLED_CLASSPATH} SandboxTest.java

BenchmarkMultiply.class: BenchmarkMultiply.java
	javac -bootclasspath ${INSTALLED_CLASSPATH} BenchmarkMultiply.java

BenchmarkZlib.class: BenchmarkZlib.java
	javac -bootclasspath ${INSTALLED_CLASSPATH} BenchmarkZlib.java


headers: SandboxTest.class BenchmarkMultiply.class BenchmarkZlib.class
	javah -bootclasspath ${INSTALLED_CLASSPATH} SandboxTest
	javah -bootclasspath ${INSTALLED_CLASSPATH} BenchmarkMultiply
	javah -bootclasspath ${INSTALLED_CLASSPATH} BenchmarkZlib
