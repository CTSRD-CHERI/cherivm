package java.lang;
import java.security.*;

class VMSandboxedNative
{
	static void checkSyscalls()
	{
		final RuntimePermission syscall = new RuntimePermission("syscall");
		SecurityManager sm = SecurityManager.current;
		if (sm != null)
		{
			sm.checkPermission(syscall);
		}
	}
}
