package sandbox;

class BenchmarkMultiply
{
	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="bench")
	native static void multiplyNative(int [] r, int cc);

	native static void multiplyNativeUnsafe(int [] r, int cc);

	static void multiplyJava(int[] r, int cc)
	{
		for (int i = 0 ; i<r.length ; i++)
		{
			r[i] *= cc;
		}
	}
	static void init(int[] a)
	{
		for (int i = 0 ; i<a.length ; i++)
		{
			a[i] = i;
		}
	}
	static void benchmark(int sz)
	{
		int[] array = new int[sz];
		init(array);
		int[] array2 = array.clone();
		double mips = 0;
		double cheri = 0;
		for (int loops=1 ; loops<50 ; loops++)
		{
			long time1 = System.nanoTime();
			multiplyNative(array2, 5);
			long time2 = System.nanoTime();
			multiplyNativeUnsafe(array, 5);
			long time3 = System.nanoTime();
			mips += time3 - time2;
			cheri += time2 - time1;
		}
		System.out.print(sz);
		System.out.println("\t" + mips + "\t" + cheri + "\t" + (cheri/mips*100));
	}
	static void main(String[] args)
	{
		System.load("./sandbox/libbench.so");
		for (int size=5 ; size<18 ; size++)
		{
			benchmark(1<<size);
			System.gc();
		}
	}
}
