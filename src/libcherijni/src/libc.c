#include "guest.h"

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/event.h>
#include <sys/wait.h>

#include <net/if.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <iconv.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#define hostInvoke_name(name) CHERIJNI_LIBC_ ## name

#define STUB_ERRNO        { printf("[SANDBOX error: %s\n", __func__); errno = ECAPMODE; return (-1); }
#define STUB_ZERO         { printf("[SANDBOX error: %s\n", __func__); errno = ECAPMODE; return 0; }
#define STUB_SIZET        { printf("[SANDBOX error: %s\n", __func__); errno = ECAPMODE; return ((size_t) - 1); }
#define STUB_NULL         { printf("[SANDBOX error: %s\n", __func__); errno = ECAPMODE; return NULL; }
#define STUB_EOF          { printf("[SANDBOX error: %s\n", __func__); errno = ECAPMODE; return EOF; }
#define STUB_MAPFAILED    { printf("[SANDBOX error: %s\n", __func__); errno = ECAPMODE; return MAP_FAILED; }
#define STUB_VOID         { printf("[SANDBOX error: %s\n", __func__); errno = ECAPMODE; }

#define OUTPUT_VAR(var)   (cap_buffer_wo(&var, sizeof(var)))

#define init_cap_fd(fd, err) \
	__capability void* cap_##fd = cherijni_fd_load(fd); \
	if (cap_##fd == CNULL) STUB_##err else { }

/* CONSTANTS */

int __isthreaded = 0;
int __local_h_errno = 0;
const char *__progname = "CheriJNI Sandbox";

int *__h_errno() {
	return &__local_h_errno;
}

/* STANDARD STREAMS */

pFILE __stdinp = NULL, __stdoutp = NULL, __stderrp = NULL;

/* PROCESS MANAGEMENT */

int pipe(int fildes[2])                                      STUB_ERRNO

/* FILE ACCESS */

int access(const char *path, int mode)                       STUB_ERRNO

int open(const char *path, int flags, ...) {
	int fileno = EIO;

	// TODO: support O_CREAT's extra argument

	__capability void *cap_fd = hostInvoke_1_2(cheri_invoke_cap, open, flags, cap_string(path), OUTPUT_VAR(fileno));
	if (cap_fd == CNULL) {
		errno = fileno;
		return -1;
	}

	if (cherijni_fd_store(cap_fd, fileno))
		return fileno;
	else
		return -1;
}

__weak_reference(open, _open);

int close(int fd) {
	init_cap_fd(fd, ERRNO);
	register_t res = hostInvoke_0_1(cheri_invoke_prim, close, cap_fd);
	if (res == CHERI_SUCCESS) {
		cherijni_fd_delete(fd);
		return 0;
	} else {
		errno = -res;
		return -1;
	}
}

__weak_reference(close, _close);

int chmod(const char *path, mode_t mode)                     STUB_ERRNO
int utime(const char *file, const struct utimbuf *timep)     STUB_ERRNO
int rename(const char *from, const char *to)                 STUB_ERRNO
int unlink(const char *path)                                 STUB_ERRNO

ssize_t read(int fd, void *buf, size_t nbytes) {
	init_cap_fd(fd, ERRNO)
	__capability void *cap_buf = cap_buffer_wo(buf, nbytes);
	register_t res = hostInvoke_0_2(cheri_invoke_prim, read, cap_fd, cap_buf);

	if (res >= 0)
		return (ssize_t) res;
	else {
		errno = -res;
		return -1;
	}
}

__weak_reference(read, _read);

ssize_t readv(int fd, const struct iovec *iov, int iovcnt)   STUB_ERRNO

ssize_t write(int fd, const void *buf, size_t nbytes) {
	init_cap_fd(fd, ERRNO)
	__capability void *cap_buf = cap_buffer_ro(buf, nbytes);
	register_t res = hostInvoke_0_2(cheri_invoke_prim, write, cap_fd, cap_buf);

	if (res >= 0)
		return (ssize_t) res;
	else {
		errno = -res;
		return -1;
	}
}

__weak_reference(write, _write);

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)  STUB_ERRNO

int dup(int oldd)                                            STUB_ERRNO
int dup2(int oldd, int newd)                                 STUB_ERRNO

int fsync(int fd)                                            STUB_ERRNO
int ftruncate(int fd, off_t length)                          STUB_ERRNO
int fprintf(FILE * restrict stream, \
            const char * restrict format, ...)               STUB_ERRNO

int ioctl(int fd, unsigned long request, ...) {
	init_cap_fd(fd, ERRNO)

	if (request == FIONREAD || request == FIONWRITE || request == FIONSPACE) {
		int arg, res;

		res = hostInvoke_1_2(cheri_invoke_prim, ioctl, request, cap_fd, OUTPUT_VAR(arg));
		if (res == CHERI_SUCCESS) {
			int *arg_res;
			va_list varargs;

			va_start(varargs, request);
			arg_res = va_arg(varargs, int*);
			va_end(varargs);

			*arg_res = arg;
			return 0;
		} else {
			errno = -res;
			return -1;
		}

	} else {
		printf("[SANDBOX ERROR: %s - unsupported operation %d]\n", __func__, request);
		errno = EINVAL;
		return -1;
	}
}

int	fcntl(int fd, int cmd, ...) {
	init_cap_fd(fd, ERRNO)

	if (cmd == F_SETFD) {
		int arg;
		va_list varargs;

		va_start(varargs, cmd);
		arg = va_arg(varargs, int);
		va_end(varargs);

		register_t res = hostInvoke_2_1(cheri_invoke_prim, fcntl, cmd, arg, cap_fd);
		if (res == CHERI_SUCCESS)
			return 0;
		else {
			errno = -res;
			return -1;
		}
	} else {
		printf("[SANDBOX ERROR: %s - unsupported operation %d]\n", __func__, cmd);
		errno = EINVAL;
		return -1;
	}
}

off_t lseek(int fildes, off_t offset, int whence) {
	init_cap_fd(fildes, ERRNO)
	register_t ret = hostInvoke_2_1(cheri_invoke_prim, lseek, offset, whence, cap_fildes);
	if (ret >= 0)
		return ret;
	else {
		errno = -ret;
		return -1;
	}
}

#define STAT_FUNCTION(NAME)                                                           \
	int NAME(const char *path, struct stat *sb) {                                     \
		__capability void *cap_path = cap_string(path);                               \
		__capability void *cap_data = cap_buffer_wo(sb, sizeof(struct stat));         \
	                                                                                  \
		register_t res = hostInvoke_0_2(cheri_invoke_prim, NAME, cap_path, cap_data); \
		if (res < 0) {                                                                \
			errno = -res;                                                             \
			return -1;                                                                \
		} else                                                                        \
			return 0;                                                                 \
	}

STAT_FUNCTION(stat)
STAT_FUNCTION(lstat)

int fstat(int fd, struct stat *sb) {
	init_cap_fd(fd, ERRNO)
	__capability void *cap_data = cap_buffer_wo(sb, sizeof(struct stat));

	register_t res = hostInvoke_0_2(cheri_invoke_prim, fstat, cap_fd, cap_data);
	if (res < 0) {
		errno = -res;
		return -1;
	} else
		return 0;
}

__weak_reference(fstat, _fstat);

int statvfs(const char * restrict path, \
            struct statvfs * restrict buf)                   STUB_ERRNO

int select(int nfds, fd_set *readfds, fd_set *writefds, \
           fd_set *exceptfds, struct timeval *timeout)       STUB_ERRNO
ssize_t readlink(const char *restrict path, \
                 char *restrict buf, size_t bufsiz)          STUB_ERRNO

int mkstemp(char *template)                                  STUB_ERRNO

/* FILE STREAMS */

FILE *fopen(const char * restrict path, \
            const char * restrict mode)                      STUB_NULL
int fclose(FILE *stream)                                     STUB_EOF
size_t fread(void * restrict ptr, size_t size, \
             size_t nmemb, FILE * restrict stream)           STUB_ZERO
size_t fwrite(const void * restrict ptr, size_t size, \
              size_t nmemb, FILE * restrict stream)          STUB_ZERO
int fprintf_l(FILE * restrict stream, locale_t loc, \
              const char * restrict format, ...)             STUB_ZERO
int vfprintf(FILE * restrict stream, \
             const char * restrict format, va_list ap)       STUB_ERRNO
int fputc(int c, FILE *stream)                               STUB_EOF
int fflush(FILE *stream)                                     STUB_EOF
void rewind(FILE *stream)                                    STUB_VOID

ssize_t getline(char ** restrict linep, \
                size_t * restrict linecapp, \
                FILE * restrict stream)                      STUB_ERRNO

/* DIRECTORY OPERATIONS */

DIR *opendir(const char *filename)                           STUB_NULL
int closedir(DIR *dirp)                                      STUB_ERRNO
struct dirent *readdir(DIR *dirp)                            STUB_NULL

int mkdir(const char *path, mode_t mode)                     STUB_ERRNO
int rmdir(const char *path)                                  STUB_ERRNO

/* SOCKETS */

int socket(int domain, int type, int protocol) {
	int fileno;
	__capability void *cap_fd = hostInvoke_3_1(cheri_invoke_cap, socket, domain, type, protocol, OUTPUT_VAR(fileno));
	if (cap_fd == CNULL) {
		errno = -fileno;
		return -1;
	}

	if (cherijni_fd_store(cap_fd, fileno))
		return fileno;
	else
		return -1;
}

int bind(int s, const struct sockaddr *addr, socklen_t addrlen) {
	init_cap_fd(s, ERRNO)
	register_t res = hostInvoke_0_2(cheri_invoke_prim, bind, cap_s, cap_buffer_ro(addr, addrlen));

	if (res == CHERI_SUCCESS)
		return 0;
	else {
		errno = -res;
		return -1;
	}
}

int listen(int s, int backlog) {
	init_cap_fd(s, ERRNO)
	register_t res = hostInvoke_1_1(cheri_invoke_prim, listen, backlog, cap_s);

	if (res == CHERI_SUCCESS)
		return 0;
	else {
		errno = -res;
		return -1;
	}
}

int shutdown(int s, int how)                                 STUB_ERRNO

int accept(int s, struct sockaddr * restrict addr, socklen_t * restrict addrlen) {
	int output[2];
	init_cap_fd(s, ERRNO)

	__capability void *res = hostInvoke_0_3(cheri_invoke_cap, accept, cap_s, cap_buffer_wo(addr, *addrlen), OUTPUT_VAR(output));

	if (res != CNULL && cherijni_fd_store(res, output[0])) {
		*addrlen = output[1];
		return output[0];
	} else {
		errno = output[0];
		return -1;
	}
}

int connect(int s, const struct sockaddr *name, \
            socklen_t namelen)                               STUB_ERRNO

ssize_t recv(int s, void *buf, size_t len, int flags)        STUB_ERRNO
ssize_t recvfrom(int s, void *buf, size_t len, int flags,
                 struct sockaddr * restrict from, \
                 socklen_t * restrict fromlen)               STUB_ERRNO
ssize_t send(int s, const void *msg, size_t len, int flags)  STUB_ERRNO
ssize_t sendto(int s, const void *msg, size_t len,
               int flags, const struct sockaddr *to,
               socklen_t tolen)                              STUB_ERRNO

int getsockopt(int s, int level, int optname, void * restrict optval, socklen_t * restrict optlen) {
	init_cap_fd(s, ERRNO)
	register_t res = hostInvoke_2_2(cheri_invoke_prim, getsockopt, level, optname, cap_s, cap_buffer_wo(optval, *optlen));

	if (res >= 0) {
		*optlen = res;
		return 0;
	} else {
		errno = -res;
		return -1;
	}
}

int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen) {
	init_cap_fd(s, ERRNO)
	register_t res = hostInvoke_2_2(cheri_invoke_prim, setsockopt, level, optname, cap_s, cap_buffer_ro(optval, optlen));

	if (res == CHERI_SUCCESS)
		return 0;
	else {
		errno = -res;
		return -1;
	}
}

int getsockname(int s, struct sockaddr * restrict name, socklen_t * restrict namelen) {
	init_cap_fd(s, ERRNO)
	register_t res = hostInvoke_0_2(cheri_invoke_prim, getsockname, cap_s, cap_buffer_wo(name, *namelen));

	if (res >= 0) {
		*namelen = res;
		return 0;
	} else {
		errno = -res;
		return -1;
	}
}

int getpeername(int s, struct sockaddr * restrict name, socklen_t * restrict namelen) {
	init_cap_fd(s, ERRNO)
	register_t res = hostInvoke_0_2(cheri_invoke_prim, getpeername, cap_s, cap_buffer_wo(name, *namelen));

	if (res >= 0) {
		*namelen = res;
		return 0;
	} else {
		errno = -res;
		return -1;
	}
}

int getifaddrs(struct ifaddrs **ifap)                        STUB_ERRNO
void freeifaddrs(struct ifaddrs *ifp)                        STUB_VOID

int gethostname(char *name, size_t namelen)                  STUB_ERRNO

struct hostent *gethostbyaddr(const void *addr, socklen_t len, int af) {
	register_t res = hostInvoke_1_1(cheri_invoke_prim, gethostbyaddr, af, cap_buffer_ro(addr, len));
	if (res < 0) {
		__local_h_errno = -res;
		printf("[SANDBOX ERROR: %s returned errno %d]\n", __func__, h_errno);
		return NULL;
	}
	return NULL;
}

int gethostbyname_r(const char *name, struct hostent *he, \
                    char *buffer, size_t buflen, \
                    struct hostent **result, int *h_errnop)  STUB_ERRNO

unsigned int if_nametoindex(const char *ifname)              STUB_ZERO

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
	printf("[SANDBOX: getpid should never fail!");
	STUB_ERRNO
}

int issetugid() {
	printf("[SANDBOX: issetugid should never fail!");
	STUB_ERRNO
}

pid_t waitpid(pid_t wpid, int *status, int options)          STUB_ERRNO
pid_t fork()                                                 STUB_ERRNO
int kill(pid_t pid, int sig)                                 STUB_ERRNO
int raise(int sig)                                           STUB_ERRNO

void exit(int status) {
	printf("[SANDBOX: exiting with code %d...]\n", status);
	abort();
}

/* TIME */

time_t time(time_t *tloc) {
	printf("[SANDBOX error: %s\n", __func__);
	return (time_t) - 1;
}

int utimes(const char *path, const struct timeval *times)    STUB_ERRNO

/* STRINGS */

int asprintf(char **ret, const char *format, ...) {
	int res;
	va_list args;

	va_start(args, format);
	res = vasprintf(ret, format, args);
	va_end(args);

	return res;
}

int vasprintf(char **ret, const char *format, va_list ap) {
	*ret = NULL;
	return (-1);
}


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
