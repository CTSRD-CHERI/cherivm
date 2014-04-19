#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/event.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <iconv.h>
#include <stdio.h>
#include <unistd.h>
#include <utime.h>

#define STUB_ERRNO        { printf("[SANDBOX stub: %s\n", __func__); errno = ECAPMODE; return (-1); }
#define STUB_SIZET        { printf("[SANDBOX stub: %s\n", __func__); errno = ECAPMODE; return ((size_t) - 1); }
#define STUB_NULL         { printf("[SANDBOX stub: %s\n", __func__); return NULL; }
#define STUB_MAPFAILED    { printf("[SANDBOX stub: %s\n", __func__); errno = ECAPMODE; return MAP_FAILED; }

/* CONSTANTS */

int __isthreaded = 0;

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
