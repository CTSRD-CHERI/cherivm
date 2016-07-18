INSTALLED_CLASSPATH=/exports/users/dc552/cheriroot/opt/target/share/jamvm/classes.zip:/exports/users/dc552/cheriroot/opt/target/share/classpath/glibj.zip:/exports/users/dc552/cheriroot/opt/target/share/classpath/tools.zip:/exports/users/dc552/cheriroot/opt/target/share/jamvm/:. 

all: sandbox/test.co sandbox/bench.co sandbox/libbench.so sandbox/test.dump sandbox/bench.dump sandbox/Sandboxed.class sandbox/BenchmarkMultiply.class sandbox/BenchmarkZlib.class sandbox/bench.S sandbox/bench_unsafe.S

clean:
	rm -f sandbox/test.co sandbox/bench.co sandbox/*.class sandbox/libbench.so sandbox/sandbox_*.h sandbox/*.dump sandbox/bench.S sandbox/bench_unsafe.S

sandbox/test.co: sandbox/sandbox_Sandboxed.c
	~/sdk/bin/clang -cheri-linker -mabi=sandbox -o sandbox/test.co sandbox/sandbox_Sandboxed.c -I ../opt/target/include/ -Wl,--script=/home/dc552/sdk/sysroot/usr/libdata/ldscripts/sandbox.ld -nostdlib -lc_cheri -lcheri -msoft-float -DJNI_SANDBOX_CLASS=test

sandbox/bench.co: sandbox/bench.c sandbox/bench_unsafe.c
	~/sdk/bin/clang -cheri-linker -mabi=sandbox -o sandbox/bench.co sandbox/bench.c -I ../opt/target/include/ -Wl,--script=/home/dc552/sdk/sysroot/usr/libdata/ldscripts/sandbox.ld -nostdlib -lc_cheri -lcheri -msoft-float -DJNI_SANDBOX_CLASS=bench -O2 -lz

sandbox/bench.S: sandbox/bench.c sandbox/bench_unsafe.c
	~/sdk/bin/clang -cheri-linker -mabi=sandbox -S -o sandbox/bench.S sandbox/bench.c -I ../opt/target/include/ -msoft-float -DJNI_SANDBOX_CLASS=bench -O2 

sandbox/bench_unsafe.S: sandbox/bench_unsafe.c
	~/sdk/bin/clang -cheri-linker -S -o sandbox/bench_unsafe.S sandbox/bench_unsafe.c -I ../opt/target/include/ -msoft-float -DJNI_SANDBOX_CLASS=bench -O2 

sandbox/bench.ll: sandbox/bench.c sandbox/bench_unsafe.c
	~/sdk/bin/clang -cheri-linker -mabi=sandbox -S -o sandbox/bench.ll sandbox/bench.c -I ../opt/target/include/ -msoft-float -DJNI_SANDBOX_CLASS=bench -O2 -emit-llvm

sandbox/bench_unsafe.ll: sandbox/bench_unsafe.c
	~/sdk/bin/clang -cheri-linker -S -o sandbox/bench_unsafe.ll sandbox/bench_unsafe.c -I ../opt/target/include/ -msoft-float -DJNI_SANDBOX_CLASS=bench -O2 -emit-llvm


sandbox/libbench.so: sandbox/bench_unsafe.c
	~/sdk/bin/clang -target cheri-unknown-freebsd -mabi=n64 -shared -o sandbox/libbench.so sandbox/bench_unsafe.c -I ../opt/target/include/ -msoft-float  -O2 -lz

sandbox/test.dump: sandbox/test.co
	~/sdk/bin/llvm-objdump -triple cheri-unknown-freebsd -d sandbox/test.co > sandbox/test.dump

sandbox/bench.dump: sandbox/bench.co
	~/sdk/bin/llvm-objdump -triple cheri-unknown-freebsd -d sandbox/bench.co > sandbox/bench.dump

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
