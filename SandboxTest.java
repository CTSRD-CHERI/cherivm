import uk.ac.cam.cheri.*;
import java.lang.reflect.Method;
import java.util.*;
import java.security.*;
import java.io.*;
import java.nio.*;


class SandboxTest
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
	native void checkBuffer(ByteBuffer b, boolean readOnly);
	boolean testBuffer()
	{
		ByteBuffer b = ByteBuffer.allocateDirect(64);
		ByteBuffer ro = b.asReadOnlyBuffer();
		checkBuffer(b, false);
		for (int i=0 ; i<64 ; i++)
		{
			if (b.get(i) != i)
			{
				return false;
			}
		}
		ro.rewind();
		if (!ro.isReadOnly())
		{
			return false;
		}
		checkBuffer(ro, true);
		return true;
	}

	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native void callbackintA(MethodTest r);
	boolean testCallback()
	{
		MethodTest m = new MethodTest();
		callbackintA(m);
		return m.x == 124;
	}

	class SingleDenySecurityManager extends SecurityManager
	{
		private Permission deny;
		SingleDenySecurityManager(Permission p)
		{
			deny = p;
		}

		public void checkPermission(Permission perm)
		{
			if (perm.equals(deny))
				throw new SecurityException();
		}
	}
	boolean testRevokeResetSecurityManager()
	{
		try
		{
			Permission revoke = new RuntimePermission("revokeSandbox");
			Permission reset = new RuntimePermission("resetSandbox");
			java.lang.Runtime R = java.lang.Runtime.getRuntime();
			try
			{
				System.setSecurityManager(new SingleDenySecurityManager(reset));
				R.resetGlobalSandbox("test");
				return false;
			}
			catch (SecurityException e) {}
			try
			{
				System.setSecurityManager(new SingleDenySecurityManager(revoke));
				R.revokeGlobalSandbox("test");
				return false;
			}
			catch (SecurityException e) {}
		}
		finally
		{
			System.setSecurityManager(null);
		}
		return true;
	}

	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native int syscallGetPid();
	boolean testSyscallGetPid()
	{
		try
		{
			Permission reset = new RuntimePermission("syscall");
			System.setSecurityManager(new SingleDenySecurityManager(reset));
			syscallGetPid();
			return false;
		}
		catch (SecurityException e)
		{
			//System.out.println(e);
			return true;
		}
		finally
		{
			System.setSecurityManager(null);
			// Check that we *can* do system calls if we have no security manager
			// installed
			syscallGetPid();
			return true;
		}
	}

	static final int O_CREAT = 0x0200;
	static final int O_RDWR = 0x0002;
	static final int O_EXEC = 0x00040000;
	static final String TEST_FILE = "file";

	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native int syscallOpen(String path, int oflags);
	boolean testSyscallOpen()
	{
		try
		{
			FilePermission filePerm = new FilePermission(TEST_FILE, "execute");
			SecurityManager sm = new SingleDenySecurityManager(filePerm);
			System.setSecurityManager(sm);
			
			//System.out.println("about to call syscallOpen");
			// Check that we *can* do open() when the policy allows it
			syscallOpen(TEST_FILE, O_RDWR | O_CREAT);

			// Check that we *can't* do something the policy disallows
			syscallOpen(TEST_FILE, O_EXEC);
			return false;
		}
		catch (SecurityException e)
		{
			//System.out.println(e);
			return true;
		}
		finally
		{
			System.setSecurityManager(null);
			// Check that we *can* do a previously disallowed open() when there
			// is no security manager
			syscallOpen(TEST_FILE, O_EXEC);
			return true;
		}
	}

	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native int syscallRead(int fd);
	boolean testSyscallRead()
	{
		int fd = syscallOpen(TEST_FILE, O_RDWR | O_CREAT);
		if (fd == -1)
		{
			//System.out.println("open failed for read");
			return false;
		}
		try
		{
			Permission readfd = new RuntimePermission("readFileDescriptor");
			System.setSecurityManager(new SingleDenySecurityManager(readfd));
			syscallRead(fd);
			return false;
		}
		catch (SecurityException e)
		{
			//System.out.println("read");
			//System.out.println(e);
			return true;
		}
		finally
		{
			System.setSecurityManager(null);
			// Check that we *can* do read if we have no security manager
			// installed
			syscallRead(fd);
			//System.out.println("read succeeded");
			return true;
		}
	}

	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native int syscallWrite(int fd);
	boolean testSyscallWrite()
	{
		int fd = syscallOpen(TEST_FILE, O_RDWR | O_CREAT);
		if (fd == -1)
		{
			//System.out.println("open failed for write");
			return false;
		}
		try
		{
			Permission writefd = new RuntimePermission("writeFileDescriptor");
			System.setSecurityManager(new SingleDenySecurityManager(writefd));
			syscallWrite(fd);
			return false;
		}
		catch (SecurityException e)
		{
			//System.out.println("write");
			//System.out.println(e);
			return true;
		}
		finally
		{
			System.setSecurityManager(null);
			// Check that we *can* do read if we have no security manager
			// installed
			syscallWrite(fd);
			//System.out.println("write succeeded");
			return true;
		}
	}

	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="test")
	native int syscallKqueue();
	boolean testSyscallKqueue()
	{
		try
		{
			Permission readfd = new RuntimePermission("readFileDescriptor");
			System.setSecurityManager(new SingleDenySecurityManager(readfd));
			syscallKqueue();
			return false;
		}
		catch (SecurityException e)
		{
			//System.out.println("kqueue");
			//System.out.println(e);
			return true;
		}
		finally
		{
			System.setSecurityManager(null);
			// Check that we *can* do read if we have no security manager
			// installed
			syscallKqueue();
			//System.out.println("kqueue succeeded");
			return true;
		}
	}

	public static void main(String[] args)
	{
		System.out.println("Starting...");
		SandboxTest s = new SandboxTest();
		s.runTests();
	}
};
