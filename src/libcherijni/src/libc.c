#include "guest.h"

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/event.h>
#include <sys/wait.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <iconv.h>
#include <unistd.h>
#include <utime.h>
#include <signal.h>

#define hostInvoke_name(name) CHERIJNI_LIBC_ ## name

#define STUB_ERRNO        { printf("[SANDBOX error: %s\n", __func__); errno = ECAPMODE; return (-1); }
#define STUB_SIZET        { printf("[SANDBOX error: %s\n", __func__); errno = ECAPMODE; return ((size_t) - 1); }
#define STUB_NULL         { printf("[SANDBOX error: %s\n", __func__); return NULL; }
#define STUB_MAPFAILED    { printf("[SANDBOX error: %s\n", __func__); errno = ECAPMODE; return MAP_FAILED; }

#define init_cap_fd(fd, err) \
	__capability void* cap_##fd = cherijni_fd_load(fd); \
	if (cap_##fd == CNULL) STUB_##err else { }

/* CONSTANTS */

int __isthreaded = 0;

/* STANDARD STREAMS */

pFILE __stdinp = NULL, __stdoutp = NULL, __stderrp = NULL;

/* PROCESS MANAGEMENT */

int pipe(int fildes[2])                                      STUB_ERRNO

/* FILE ACCESS */

int access(const char *path, int mode)                       STUB_ERRNO

int open(const char *path, int flags, ...) {
	int fileno;
	__capability void *cap_fileno = cap_buffer_wo(&fileno, sizeof(fileno));
	__capability void *cap_path = cap_string(path);

	// TODO: support O_CREAT's extra argument

	__capability void *cap_fd = hostInvoke_1_2(cheri_invoke_cap, open, flags, cap_path, cap_fileno);
	if (cap_fd == CNULL) {
		STUB_ERRNO
	}

	return cherijni_fd_store(cap_fd, fileno);
}

int close(int fd) {
	init_cap_fd(fd, ERRNO);
	register_t res = hostInvoke_0_1(cheri_invoke_prim, close, cap_fd);
	if (res == CHERI_FAIL)
		return -1;
	else {
		cherijni_fd_delete(fd);
		return 0;
	}
}

int chmod(const char *path, mode_t mode)                     STUB_ERRNO
int utime(const char *file, const struct utimbuf *timep)     STUB_ERRNO
int rename(const char *from, const char *to)                 STUB_ERRNO
int unlink(const char *path)                                 STUB_ERRNO

ssize_t read(int fd, void *buf, size_t nbytes) {
	init_cap_fd(fd, ERRNO)
	__capability void *cap_buf = cap_buffer_wo(buf, nbytes);
	return (ssize_t) hostInvoke_0_2(cheri_invoke_prim, read, cap_fd, cap_buf);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)   STUB_ERRNO

ssize_t write(int fd, const void *buf, size_t nbytes) {
	init_cap_fd(fd, ERRNO)
	__capability void *cap_buf = cap_buffer_ro(buf, nbytes);
	return (ssize_t) hostInvoke_0_2(cheri_invoke_prim, write, cap_fd, cap_buf);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)  STUB_ERRNO

int dup(int oldd)                                            STUB_ERRNO
int dup2(int oldd, int newd)                                 STUB_ERRNO

int	fcntl(int fd, int cmd, ...)                              STUB_ERRNO
int fsync(int fd)                                            STUB_ERRNO
int ftruncate(int fd, off_t length)                          STUB_ERRNO
int fprintf(FILE * restrict stream, \
            const char * restrict format, ...)               STUB_ERRNO

int ioctl(int fd, unsigned long request, ...)                STUB_ERRNO
off_t lseek(int fildes, off_t offset, int whence)            STUB_ERRNO

#define STAT_FUNCTION(NAME)                                                           \
	int NAME(const char *path, struct stat *sb) {                                     \
		__capability void *cap_path = cap_string(path);                               \
		__capability void *cap_data = cap_buffer_wo(sb, sizeof(struct stat));         \
	                                                                                  \
		register_t res = hostInvoke_0_2(cheri_invoke_prim, NAME, cap_path, cap_data); \
		if (res == CHERI_FAIL)                                                        \
			STUB_ERRNO                                                                \
		else                                                                          \
			return 0;                                                                 \
	}

STAT_FUNCTION(stat)
STAT_FUNCTION(lstat)

int fstat(int fd, struct stat *sb) {
	init_cap_fd(fd, ERRNO)
	__capability void *cap_data = cap_buffer_wo(sb, sizeof(struct stat));

	register_t res = hostInvoke_0_2(cheri_invoke_prim, fstat, cap_fd, cap_data);
	if (res == CHERI_FAIL)
		STUB_ERRNO
	else
		return 0;
}

int statvfs(const char * restrict path, \
            struct statvfs * restrict buf)                   STUB_ERRNO

int select(int nfds, fd_set *readfds, fd_set *writefds, \
           fd_set *exceptfds, struct timeval *timeout)       STUB_ERRNO
ssize_t readlink(const char *restrict path, \
                 char *restrict buf, size_t bufsiz)          STUB_ERRNO

/* DIRECTORY OPERATIONS */

DIR *opendir(const char *filename)                           STUB_NULL
int closedir(DIR *dirp)                                      STUB_ERRNO
struct dirent *readdir(DIR *dirp)                            STUB_NULL

int mkdir(const char *path, mode_t mode)                     STUB_ERRNO
int rmdir(const char *path)                                  STUB_ERRNO

/* SOCKETS */

int socket(int domain, int type, int protocol)               STUB_ERRNO
int accept(int s, struct sockaddr * restrict addr, \
           socklen_t * restrict addrlen)                     STUB_ERRNO
int connect(int s, const struct sockaddr *name, \
            socklen_t namelen)                               STUB_ERRNO
ssize_t recvfrom(int s, void *buf, size_t len, int flags,
                 struct sockaddr * restrict from, \
                 socklen_t * restrict fromlen)               STUB_ERRNO
ssize_t sendto(int s, const void *msg, size_t len,
               int flags, const struct sockaddr *to,
               socklen_t tolen)                              STUB_ERRNO
int getsockopt(int s, int level, int optname, \
               void * restrict optval, \
               socklen_t * restrict optlen)                  STUB_ERRNO
int getsockname(int s, struct sockaddr * restrict name, \
                socklen_t * restrict namelen)                STUB_ERRNO
int getpeername(int s, struct sockaddr * restrict name, \
                socklen_t * restrict namelen)                STUB_ERRNO

/* MMAP */

void *mmap(void *addr, size_t len, int prot, int flags, \
           int fd, off_t offset)                             STUB_MAPFAILED
int munmap(void *addr, size_t len)                           STUB_ERRNO
int msync(void *addr, size_t len, int flags)                 STUB_ERRNO
int madvise(void *addr, size_t len, int behav)               STUB_ERRNO
int mincore(const void *addr, size_t len, char *vec)         STUB_ERRNO

int getpagesize() {
	return PAGE_SIZE;
}

/* KQUEUE */

int kqueue()                                                 STUB_ERRNO
int kevent(int kq, const struct kevent *changelist, \
           int nchanges, struct kevent *eventlist, \
           int nevents, const struct timespec *timeout)      STUB_ERRNO

/* ICONV */

iconv_t iconv_open(const char *dstname, const char *srcname) STUB_ERRNO
int iconv_close(iconv_t cd)                                  STUB_ERRNO
size_t iconv(iconv_t cd, const char ** restrict src, \
             size_t * restrict srcleft, \
             char ** restrict dst, \
             size_t * restrict dstleft)                      STUB_SIZET

/* TERMINAL */

int tcgetattr(int fd, struct termios *t)                     STUB_ERRNO
int tcsetattr(int fd, int action, const struct termios *t)   STUB_ERRNO

/* PROCESSES */

char *environ[] = { NULL };

int execvp(const char *file, char *const argv[])             STUB_ERRNO
int execve(const char *path, char *const argv[], \
           char *const envp[])                               STUB_ERRNO
int chdir(const char *path)                                  STUB_ERRNO
char *getenv(const char *name)                               STUB_NULL
pid_t getpid() {
	printf("WARNING: getpid should never fail!");
	STUB_ERRNO
}
pid_t waitpid(pid_t wpid, int *status, int options)          STUB_ERRNO
pid_t fork()                                                 STUB_ERRNO
int kill(pid_t pid, int sig)                                 STUB_ERRNO
int raise(int sig)                                           STUB_ERRNO

/* INITIALIZATION */

#define GET_STD_STREAM(lowercase, uppercase)                                                 \
	static pFILE get_std##lowercase() {                                                      \
		__capability void *cap_fd = hostInvoke_0_0(cheri_invoke_cap, GetStd##lowercase##FD); \
		if (cap_fd == CNULL)                                                                 \
			return NULL;                                                                     \
		cherijni_fd_store(cap_fd, STD##uppercase##_FILENO);                                  \
		                                                                                     \
		__capability void *cap_file = hostInvoke_0_1(cheri_invoke_cap, GetStream, cap_fd);   \
		if (cap_file == CNULL)                                                               \
			return NULL;                                                                     \
		else                                                                                 \
			return cherijni_pFILE_store(cap_file, STD##uppercase##_FILENO);                  \
	}

GET_STD_STREAM(in, IN)
GET_STD_STREAM(out, OUT)
GET_STD_STREAM(err, ERR)

void cherijni_libc_init() {
	if (__stdinp == NULL) __stdinp = get_stdin();
	if (__stdoutp == NULL) __stdoutp = get_stdout();
	if (__stderrp == NULL) __stderrp = get_stderr();
}

/*
 * Must provide a definition of fileno, but it will never be called.
 * The system defines it (provided __isThreaded == 0) as: (stream->file)
 */
#undef fileno
int fileno(FILE *stream) {
	return 0;
}
