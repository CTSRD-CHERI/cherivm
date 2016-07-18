package sandbox;

import java.nio.ByteBuffer;

class BenchmarkZlib
{
	@Sandbox(scope=Sandbox.Scope.Global,SandboxClass="bench")
	native static long compress(ByteBuffer input, ByteBuffer output);
	native static long compressUnsafe(ByteBuffer input, ByteBuffer output);
	native static long compressUnsafeCopy(ByteBuffer input, ByteBuffer output);
	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="bench")
	native static long decompress(ByteBuffer input, ByteBuffer output, int len);
	native static long decompressUnsafe(ByteBuffer input, ByteBuffer output, int len);
	native static long decompressUnsafeCopy(ByteBuffer input, ByteBuffer output, int len);

	static ByteBuffer init(int size)
	{
		ByteBuffer b = ByteBuffer.allocateDirect(size);
		for (int i = 0 ; i<size/4 ; i++)
		{
			b.putInt(i);
		}
		b.rewind();
		return b;
	}

	static void benchmarkCompress(int sz)
	{
		System.out.println("Allocating " + sz + " byte buffers");
		ByteBuffer s = init(sz);
		ByteBuffer u1 = ByteBuffer.allocateDirect(sz);
		ByteBuffer u2 = ByteBuffer.allocateDirect(sz);
		ByteBuffer u3 = ByteBuffer.allocateDirect(sz);
		long  mips_copy = 0;
		long mips = 0;
		long cheri = 0;
		System.out.println("cheri...");
		System.startSampling();
		for (int loops=1 ; loops<50 ; loops++)
		{
			compress(s, u1);
		}
		System.endSampling("zlib_cheri" + sz);
		System.out.println("MIPS...");
		System.startSampling();
		for (int loops=1 ; loops<50 ; loops++)
		{
			compressUnsafe(s, u2);
		}
		System.endSampling("zlib_mips" + sz);
		System.out.println("MIPS (copy)...");
		System.startSampling();
		for (int loops=1 ; loops<50 ; loops++)
		{
			compressUnsafeCopy(s, u3);
		}
		System.endSampling("zlib_mips_copy" + sz);
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
	}
	static void benchmarkDecompress(int sz)
	{
		System.out.println("Decompressing " + sz + " bytes of uncompressed data");
		ByteBuffer s = init(sz);
		ByteBuffer c = ByteBuffer.allocateDirect(sz);
		int limit = (int)compressUnsafeCopy(s, c);
		c.limit(limit);
		ByteBuffer u1 = ByteBuffer.allocateDirect(sz);
		ByteBuffer u2 = ByteBuffer.allocateDirect(sz);
		ByteBuffer u3 = ByteBuffer.allocateDirect(sz);
		long  mips_copy = 0;
		long mips = 0;
		long cheri = 0;
		System.out.println("cheri...");
		System.startSampling();
		for (int loops=1 ; loops<50 ; loops++)
		{
			decompress(c, u1, limit);
		}
		System.endSampling("zlib_decompress_cheri" + sz);
		System.out.println("MIPS...");
		System.startSampling();
		for (int loops=1 ; loops<50 ; loops++)
		{
			decompressUnsafe(c, u2, limit);
		}
		System.endSampling("zlib_decompress_mips" + sz);
		System.out.println("MIPS (copy)...");
		System.startSampling();
		for (int loops=1 ; loops<50 ; loops++)
		{
			decompressUnsafeCopy(c, u3, limit);
		}
		System.endSampling("zlib_decompress_mips_copy" + sz);
		if (!s.equals(u2))
		{
			System.out.println("MIPS decompression is buggy");
			System.exit(-1);
		}
		if (!s.equals(u1))
		{
			System.out.println("CHERI decompression is buggy");
			System.exit(-1);
		}
		if (!s.equals(u3))
		{
			System.out.println("MIPS (copy) decompression is buggy");
			System.exit(-1);
		}
	}
	static void main(String[] args)
	{
		System.load("./sandbox/libbench.so");
		ByteBuffer s = ByteBuffer.allocateDirect(1);
		ByteBuffer u = ByteBuffer.allocateDirect(50);
		compressUnsafeCopy(s, u);
		compressUnsafe(s, u);
		compress(s, u);
		decompressUnsafeCopy(u, s, 0);
		decompressUnsafe(u, s, 0);
		decompress(u, s, 0);
		for (int size=5 ; size<12 ; size++)
		{
			for (int iteration=0 ; iteration<10 ; iteration++)
			{
				benchmarkCompress(1<<size);
				System.gc();
			}
		}
		for (int size=5 ; size<24 ; size++)
		{
			for (int iteration=0 ; iteration<10 ; iteration++)
			{
				benchmarkDecompress(1<<size);
				System.gc();
			}
		}
	}
}
