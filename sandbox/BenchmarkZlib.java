package sandbox;

import java.nio.ByteBuffer;

class BenchmarkZlib
{
	@Sandbox(scope=Sandbox.Scope.Global,SandboxClass="bench")
	native static void compress(ByteBuffer input, ByteBuffer output);
	native static void compressUnsafe(ByteBuffer input, ByteBuffer output);
	native static void compressUnsafeCopy(ByteBuffer input, ByteBuffer output);
	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="bench")
	native static void decompress(ByteBuffer input, ByteBuffer output);
	native static void decompressUnsafe(ByteBuffer input, ByteBuffer output);
	native static void decompressUnsafeCopy(ByteBuffer input, ByteBuffer output);

	static ByteBuffer init(int size)
	{
		ByteBuffer b = ByteBuffer.allocateDirect(size);
		for (int i = 0 ; i<size/4 ; i++)
		{
			b.putInt(i);
		}
		return b;
	}

	static void benchmarkCompress(int sz)
	{
		System.out.println("Allocating " + sz + " byte buffers");
		ByteBuffer s = init(sz);
		System.out.println("Capacity: " + s.capacity());
		// TODO: Have a version of this that verifies the output.
		ByteBuffer u1 = ByteBuffer.allocateDirect(sz);
		ByteBuffer u2 = ByteBuffer.allocateDirect(sz);
		ByteBuffer u3 = ByteBuffer.allocateDirect(sz);
		long  mips_copy = 0;
		long mips = 0;
		long cheri = 0;
		for (int loops=1 ; loops<50 ; loops++)
		{
			long time1 = System.nanoTime();
			compress(s, u1);
			long time2 = System.nanoTime();
			compressUnsafe(s, u2);
			long time3 = System.nanoTime();
			compressUnsafeCopy(s, u3);
			long time4 = System.nanoTime();
			mips_copy += time4 - time3;
			mips += time3 - time2;
			cheri += time2 - time1;
		}
		if (!u1.equals(u2))
		{
			System.out.println("compress and compressUnsafe produced different results");
			System.exit(-1);
		}
		if (!u1.equals(u3))
		{
			System.out.println("compress and compressUnsafeCopy produced different results");
			System.exit(-1);
		}
		System.out.print(sz);
		System.out.print("\t" + mips + "\t" + mips_copy + "\t" + cheri + "\t");
		System.out.println(((double)cheri/mips) + "\t" + ((double)mips_copy/mips));
	}
	static void main(String[] args)
	{
		System.load("./sandbox/libbench.so");
		for (int size=5 ; size<18 ; size++)
		{
			benchmarkCompress(1<<size);
			System.gc();
		}
	}
}
