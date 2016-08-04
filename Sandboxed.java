import uk.ac.cam.cheri.*;
import java.lang.reflect.Method;
import java.util.*;


class Sandboxed
{
	private void printTest(int num, int count, String test)
	{
		System.out.print('[');
		System.out.print(num);
		System.out.print('/');
		System.out.print(count);
		System.out.print("] ");
		System.out.print(test);
		System.out.print("...");
		System.out.flush();
	}
	public void runTests()
	{
		Class cls = this.getClass();
		Method[] allMethods = cls.getDeclaredMethods();
		List<Method> testMethods = new ArrayList<Method>();
		for (Method m : allMethods)
		{
			if (m.getName().startsWith("test"))
			{
				testMethods.add(m);
			}
		}
		System.out.println("Running tests...");
		int count = testMethods.size();
		int passcount = 0;
		int current = 0;
		int exceptioncount = 0;
		for (Method m : testMethods)
		{
			printTest(++current, count, m.getName());
			//System.out.printf("[%d/%d] %s...", ++current, count, m.getName());
			//System.out.flush();
			boolean passed = false;
			boolean exception = false;
			try
			{
				passed = ((Boolean)m.invoke(this)).booleanValue();
				System.out.println(passed ? "passed" : "failed");
			}
			catch (Exception e)
			{
				exception = true;
				System.out.println("threw unexpected exception:");
				e.printStackTrace(System.out);
			}
			passcount += passed ? 1 : 0;
		}
		System.out.print(passcount);
		System.out.print(" of ");
		System.out.print(count);
		System.out.println(" tests passed.");
		//System.out.printf("%d of %d tests passed\n", passcount, count);
	}

	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native void nativeThingWithArgs(Object foo, int bar, long baz);
	boolean testArguments()
	{
		try
		{
			nativeThingWithArgs("a string", 1, 2);
		}
		catch (java.lang.NullPointerException e)
		{
			return false;
		}
		return true;
	}

	@Sandbox(scope=Sandbox.Scope.Global,SandboxClass="test")
	native int nativeThing();
	boolean testPersistentSandbox()
	{
		for (int i=0 ; i<5 ; i++)
		{
			if (nativeThing() != 42 + i)
			{
				return false;
			}
		}
		return true;
	}

	boolean testSandboxReset()
	{
		java.lang.Runtime R = java.lang.Runtime.getRuntime();
		R.resetGlobalSandbox("test");
		int val = nativeThing();
		R.resetGlobalSandbox("test");
		return val == 42;
	}

	@Sandbox(scope=Sandbox.Scope.Global,SandboxClass="test")
	native void leakArray(int ints[]);
	boolean testArrayLeak()
	{
		int[] array = new int[10];
		for (int i=0 ; i<10 ; i++)
		{
			array[i] = 10-i;
		}
		leakArray(array);
		java.lang.Runtime R = java.lang.Runtime.getRuntime();
		R.resetGlobalSandbox("test");
		leakArray(array);
		return true;
	}

	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native void arrayaccess(int ints[]);
	boolean testArrayAccess()
	{
		int[] array = new int[10];
		for (int i=0 ; i<10 ; i++)
		{
			array[i] = 10-i;
		}
		arrayaccess(array);
		return true;
	}
	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native void invalidarrayaccess(int ints[]);
	boolean testInvalidArrayAccess()
	{
		int[] array = new int[10];
		for (int i=0 ; i<10 ; i++)
		{
			array[i] = 10-i;
		}
		try
		{
			invalidarrayaccess(array);
		}
		catch (java.lang.NullPointerException e)
		{
			return true;
		}
		return true;
	}

	class NativeState
	{
		@Sandbox(scope=Sandbox.Scope.Object,SandboxClass="test")
		native int count();
	}
	boolean testObjectState()
	{
		NativeState n1 = new NativeState();
		NativeState n2 = new NativeState();
		if (n1.count() != 1) { return false; }
		if (n2.count() != 1) { return false; }
		if (n1.count() != 2) { return false; }
		if (n2.count() != 2) { return false; }
		if (n1.count() != 3) { return false; }
		if (n2.count() != 3) { return false; }
		if (n1.count() != 4) { return false; }
		if (n2.count() != 4) { return false; }
		return true;
	}

	class ReflectionTest
	{
		public int x;
		public long y;
	};
	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native void reflectfields(ReflectionTest r);
	boolean testfieldreflection()
	{
		ReflectionTest r = new ReflectionTest();
		r.x = 12;
		r.y = 42;
		reflectfields(r);
		if (r.x != -1)
		{
			return false;
		}
		if (r.y != 47)
		{
			return false;
		}
		return true;
	}
	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native void reflectabuse(ReflectionTest r);
	boolean testReflectionTypeSafety()
	{
		try
		{
			ReflectionTest r = new ReflectionTest();
			reflectabuse(r);
		}
		catch (Exception e)
		{
			return true;
		}
		return false;
	}

	class MethodTest
	{
		int x;
		int accumulate(int a, int b)
		{
			x += a*b;
			return x;
		}
	};
	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native void callbackintA(MethodTest r);
	boolean testCallback()
	{
		MethodTest m = new MethodTest();
		callbackintA(m);
		return m.x == 124;
	}

	public static void main(String[] args)
	{
		System.out.println("Starting...");
		Sandboxed s = new Sandboxed();
		s.runTests();
	}
};
