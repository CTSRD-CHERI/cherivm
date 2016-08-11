package java.lang;
import java.io.*;
import java.security.*;

class VMSandboxedNative
{
	// open() flags
	static final int O_RDONLY = 0x0000; // open for reading only (read)
	static final int O_WRONLY = 0x0001; // open for writing only (write)
	static final int O_RDWR = 0x0002; // open for reading and writing (read,write)
	static final int O_EXEC = 0x00040000; // open for execute only (execute)
	static final int O_NONBLOCK = 0x0004; // do not block on open or for data to become available
	static final int O_APPEND = 0x0008; // append on each write (write)
	static final int O_CREAT = 0x0200; // create file if it does not exist (write)
	static final int O_TRUNC = 0x0400; // truncate size to 0 (write)
	static final int O_EXCL = 0x0800; // error if O_CREAT and the file exists 
	static final int O_SHLOCK = 0x0010; // atomically obtain a shared lock
	static final int O_EXLOCK = 0x0020; // atomically obtain an exclusive lock
	static final int O_DIRECT= 0x00010000; // eliminate or reduce cache effects
	static final int O_FSYNC = 0x0080; // synchronous writes
	static final int O_SYNC = 0x0080; // synchronous writes
	static final int O_NOFOLLOW = 0x0100; // do not follow symlinks
	static final int O_DIRECTORY = 0x00020000; // error if file is not a directory
	static final int O_CLOEXEC = 0x00100000; //mark as close-on-exec

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

	static void checkSyscallOpen(String path, int oflags)
	{
		//System.out.print("VMSandboxedNative::checkSyscallOpen()... ");
		checkSyscallPerm();

		// Do access-control check
		//System.out.print("Doing access-control check... ");

		// Java file permissions we care about
		boolean read = false, write = false, execute = false, readlink = false;
		read = (oflags & (O_WRONLY | O_EXEC)) == 0;
		write = (oflags & (O_WRONLY | O_RDWR | O_APPEND | O_CREAT | O_TRUNC)) != 0;
		execute = (oflags & O_EXEC) != 0;
		readlink = (oflags & O_NOFOLLOW) != 0;
		
		// convert to a comma-separated list
		String actions = "";
		boolean addComma = false;
		if (read)
		{
			actions = "read";
			addComma = true;
		}
		if (write)
		{
			actions += (addComma ? "," : "") + "write";
			addComma = true;
		}
		if (execute)
		{
			actions += (addComma ? "," : "") + "execute";
			addComma = true;
		}
		if (readlink)
		{
			actions += (addComma ? "," : "") + "readlink";
		}
		
		//System.out.print("Checking if (\"" + path + "\", \"" + actions + "\") is allowed... ");
		SecurityManager sm = SecurityManager.current;
		if (sm != null)
		{
			//System.out.println("Security manager not null");
			sm.checkPermission(new FilePermission(path, actions));
		}
	}
}
