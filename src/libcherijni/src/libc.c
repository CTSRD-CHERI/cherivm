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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <iconv.h>
#include <unistd.h>
#include <utime.h>

#define hostInvoke_7(name, a1, a2, a3, a4, a5, a6, a7) \
	(cheri_invoke(cherijni_obj_system, \
	    CHERIJNI_LIBC_ ## name, \
	    a1, a2, a3, a4, a5, a6, a7, \
	    cheri_getdefault(), \
	    CNULL, \
		cap_output, \
	    cheri_zerocap(), cheri_zerocap(), cheri_zerocap(), cheri_zerocap(), cheri_zerocap()))

#define STUB_ERRNO        { printf("[SANDBOX stub: %s\n", __func__); errno = ECAPMODE; return (-1); }
#define STUB_SIZET        { printf("[SANDBOX stub: %s\n", __func__); errno = ECAPMODE; return ((size_t) - 1); }
#define STUB_NULL         { printf("[SANDBOX stub: %s\n", __func__); return NULL; }
#define STUB_MAPFAILED    { printf("[SANDBOX stub: %s\n", __func__); errno = ECAPMODE; return MAP_FAILED; }

/* CONSTANTS */

int __isthreaded = 0;

/* STANDARD STREAMS */

FILE *__stdinp = NULL, *__stdoutp = NULL, *__stderrp = NULL;

struct __sFILE_compat {
	void   *_capability;
	int     _r;
	int     _w;
	short   _flags;
	short   _file;          /* (*) fileno, if Unix descriptor, else -1 */
};

FILE *create_file_ptr(void *cap_ptr, short file_num) {
	struct __sFILE_compat *file_ptr = (struct __sFILE_compat*) malloc(sizeof(struct __sFILE_compat));
	file_ptr->_capability = cap_ptr;
	file_ptr->_file = file_num;
	return (FILE*) file_ptr;
}

/* PROCESS MANAGEMENT */

int pipe(int fildes[2])                                      STUB_ERRNO

/* FILE ACCESS */

int access(const char *path, int mode)                       STUB_ERRNO
int open(const char *path, int flags, ...)                   STUB_ERRNO
int close(int fd)                                            STUB_ERRNO

int chmod(const char *path, mode_t mode)                     STUB_ERRNO
int utime(const char *file, const struct utimbuf *timep)     STUB_ERRNO
int rename(const char *from, const char *to)                 STUB_ERRNO
int unlink(const char *path)                                 STUB_ERRNO

ssize_t read(int d, void *buf, size_t nbytes)                STUB_ERRNO
ssize_t readv(int fd, const struct iovec *iov, int iovcnt)   STUB_ERRNO
ssize_t write(int fd, const void *buf, size_t nbytes)        STUB_ERRNO
ssize_t writev(int fd, const struct iovec *iov, int iovcnt)  STUB_ERRNO

int	fcntl(int fd, int cmd, ...)                              STUB_ERRNO
int fstat(int fd, struct stat *sb)                           STUB_ERRNO
int fsync(int fd)                                            STUB_ERRNO
int ftruncate(int fd, off_t length)                          STUB_ERRNO
int fprintf(FILE * restrict stream, \
            const char * restrict format, ...)               STUB_ERRNO

int ioctl(int fd, unsigned long request, ...)                STUB_ERRNO
off_t lseek(int fildes, off_t offset, int whence)            STUB_ERRNO

int stat(const char *path, struct stat *sb)                  STUB_ERRNO
int statvfs(const char * restrict path, \
            struct statvfs * restrict buf)                   STUB_ERRNO

int select(int nfds, fd_set *readfds, fd_set *writefds, \
           fd_set *exceptfds, struct timeval *timeout)       STUB_ERRNO

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

/* GETPID */

pid_t getpid() {
	printf("WARNING: getpid should never fail!");
	STUB_ERRNO
}

static FILE *get_stdin() {
	register_t result = hostInvoke_0(GetStdin);
	check_cheri_fail(result, NULL);
	return create_file_ptr(get_output, result);
}

static FILE *get_stdout() {
	register_t result = hostInvoke_0(GetStdout);
	check_cheri_fail(result, NULL);
	return create_file_ptr(get_output, result);
}

static FILE *get_stderr() {
	register_t result = hostInvoke_0(GetStderr);
	check_cheri_fail(result, NULL);
	return create_file_ptr(get_output, result);
}

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
