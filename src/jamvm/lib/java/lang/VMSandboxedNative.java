package java.lang;
import java.io.*;
import java.security.*;

class VMSandboxedNative
{
	static void checkSyscallPerm()
	{
		//System.out.println("VMSandboxedNative::checkSyscallPerm()");
		final RuntimePermission syscall = new RuntimePermission("syscall");
		SecurityManager sm = SecurityManager.current;
		if (sm != null)
		{
			//System.out.println("Security manager not null");
			sm.checkPermission(syscall);
		}
	}

	static void checkSyscallOpen(String path, String actions)
	{
		//System.out.print("VMSandboxedNative::checkSyscallOpen()... ");
		checkSyscallPerm();

		// Do access-control check
		//System.out.print("Doing access-control check... ");

		//System.out.print("Checking if (\"" + path + "\", \"" + actions + "\") is allowed... ");
		SecurityManager sm = SecurityManager.current;
		if (sm != null)
		{
			//System.out.println("Security manager not null");
			sm.checkPermission(new FilePermission(path, actions));
		}
	}
	
	static void checkSyscallReadFD()
	{
		final RuntimePermission readFD = new RuntimePermission("readFileDescriptor");
		SecurityManager sm = SecurityManager.current;
		if (sm != null)
		{
			sm.checkPermission(readFD);
		}
	}

	static void checkSyscallWriteFD()
	{
		final RuntimePermission writeFD = new RuntimePermission("writeFileDescriptor");
		SecurityManager sm = SecurityManager.current;
		if (sm != null)
		{
			sm.checkPermission(writeFD);
		}
	}

	static void checkSyscallReadWriteFD()
	{
		final RuntimePermission readFD = new RuntimePermission("readFileDescriptor");
		final RuntimePermission writeFD = new RuntimePermission("writeFileDescriptor");
		SecurityManager sm = SecurityManager.current;
		if (sm != null)
		{
			sm.checkPermission(readFD);
			sm.checkPermission(writeFD);
		}
	}
}
