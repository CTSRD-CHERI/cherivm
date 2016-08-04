import uk.ac.cam.cheri.*;
import java.util.Arrays;

class BenchmarkMultiply
{
	@Sandbox(scope=Sandbox.Scope.Global,SandboxClass="bench")
	native static void multiplyNative(int [] out, int[] l, int[] r, int cc);
	native static void multiplyNativeUnsafeCopy(int [] out, int[] l, int[] r, int cc);
	native static void multiplyNativeUnsafe(int [] out, int[] l, int[] r, int cc);

	static int[] init(int sz)
	{
		int[] a = new int[sz];
		for (int i = 0 ; i<a.length ; i++)
		{
			a[i] = i;
		}
		return a;
	}
	static void benchmark(int sz)
	{
		System.out.println("Running benchmark " + sz);
		final long loop_count = 10;
		int[] out = new int[sz*sz];
		int[] in1 = init(sz*sz);
		int[] in2 = init(sz*sz);
		System.out.println("cheri...");
		System.startSampling();
		for (int loops=0 ; loops<loop_count ; loops++)
		{
			multiplyNative(out, in1, in2, sz);
		}
		System.endSampling("Multiply_cheri" + sz);
		System.out.println("MIPS (copy)...");
		System.startSampling();
		for (int loops=0 ; loops<loop_count ; loops++)
		{
			multiplyNativeUnsafeCopy(out, in1, in2, sz);
		}
		System.endSampling("Multiply_mips_copy" + sz);
		System.out.println("MIPS...");
		System.startSampling();
		for (int loops=0 ; loops<loop_count ; loops++)
		{
			multiplyNativeUnsafe(out, in1, in2, sz);
		}
		System.endSampling("Multiply_mips" + sz);
	}
	static void main(String[] args)
	{
		System.load("./libbench.so");
		// Make sure that everything is resolved first so that we don't skew
		// the first benchmarks.  CHERI libraries are loaded lazily, whereas
		// native ones are loaded explicitly.
		int[] array = new int[1];
		multiplyNativeUnsafe(array, array, array, 1);
		multiplyNativeUnsafeCopy(array, array, array, 1);
		multiplyNative(array, array, array, 1);
		// Sanity check that we're actually getting the same answers each time
		final boolean sanity = true;
		if (sanity)
		{
			int[] a1 = init(16);
			int[] a2 = init(16);
			int[] out1 = new int[16];
			int[] out2 = new int[16];
			multiplyNative(out1, a1, a2, 4);
			multiplyNativeUnsafe(out2, a1, a2, 4);
			if (!Arrays.equals(out1, out2))
			{
				System.out.println("Comparison failed");
				return;
			}
		}
		java.lang.Runtime R = java.lang.Runtime.getRuntime();
		for (int size=8 ; size<=50 ; size+=2)
		{
			for (int i=0 ; i<200 ; i++)
			{
				benchmark(size);
				System.gc();
			}
			R.resetGlobalSandbox("bench");
			System.gc();
		}
	}
}
