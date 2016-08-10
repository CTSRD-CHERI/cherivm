package java.lang;
import java.security.*;

class VMSandboxedNative
{
	static void checkSyscalls()
	{
		//System.out.println("VMSandboxedNative::checkSyscalls()");
		final RuntimePermission syscall = new RuntimePermission("syscall");
		SecurityManager sm = SecurityManager.current;
		if (sm != null)
		{
			//System.out.println("Security manager not null");
			sm.checkPermission(syscall);
		}
	}
}
