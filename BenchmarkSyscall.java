import uk.ac.cam.cheri.*;
import java.util.Arrays;
import java.security.*;

class BenchmarkSyscall
{
	@Sandbox(scope=Sandbox.Scope.Global,SandboxClass="bench")
	native static int getPid();
	native static int getPidUnsafe();

	class TrivialSecurityManager extends SecurityManager
	{
		public void checkPermission(Permission perm) {}
	}
	
	void benchmark()
	{
		System.out.println("Running benchmark ");
		final long loop_count = 50;
		System.out.println("cheri...");
		System.startSampling();
		for (int loops=0 ; loops<loop_count ; loops++)
		{
			getPid();
		}
		System.endSampling("Syscall_cheri");
		System.out.println("CHERI (security manager)...");
		System.setSecurityManager(new TrivialSecurityManager());
		System.startSampling();
		for (int loops=0 ; loops<loop_count ; loops++)
		{
			getPid();
		}
		System.endSampling("Syscall_cheri_security_manager");
		System.setSecurityManager(null);
		System.out.println("MIPS...");
		System.startSampling();
		for (int loops=0 ; loops<loop_count ; loops++)
		{
			getPidUnsafe();
		}
		System.endSampling("Syscall_mips");
	}
	static void main(String[] args)
	{
		System.load("./libbench.so");
		// Make sure that everything is resolved first so that we don't skew
		// the first benchmarks.  CHERI libraries are loaded lazily, whereas
		// native ones are loaded explicitly.
		getPid();
		getPidUnsafe();
		BenchmarkSyscall b = new BenchmarkSyscall();
		for (int i=0 ; i<10 ; i++)
		{
			b.benchmark();
			System.gc();
		}
	}
}
