import uk.ac.cam.cheri.*;
import java.util.Arrays;

class BenchmarkNop
{
	@Sandbox(scope=Sandbox.Scope.Global,SandboxClass="bench")
	native void nopCheriGlobal();
	@Sandbox(scope=Sandbox.Scope.Method,SandboxClass="bench")
	native void nopCheriMethod();

	native void nopMips();

	void bench()
	{
		nopCheriGlobal();
		nopCheriMethod();
		nopCheriGlobal();
		nopCheriMethod();
		nopMips();
	}
	static void main(String[] args)
	{
		System.load("./libbench.so");
		(new BenchmarkNop()).bench();
	}
}
