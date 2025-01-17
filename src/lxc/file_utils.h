/* SPDX-License-Identifier: LGPL-2.1+ */

#ifndef __LXC_FILE_UTILS_H
#define __LXC_FILE_UTILS_H

#include "config.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/vfs.h>
#include <unistd.h>

#include "compiler.h"
#include "memory_utils.h"
#include "syscall_wrappers.h"

/* read and write whole files */
extern int lxc_write_to_file(const char *filename, const void *buf, size_t count,
				      bool add_newline, mode_t mode) __access_r(2, 3);

extern int lxc_readat(int dirfd, const char *filename, void *buf, size_t count)
    __access_w(3, 4);

extern int lxc_writeat(int dirfd, const char *filename, const void *buf, size_t count)
    __access_r(3, 4);

extern int lxc_write_openat(const char *dir, const char *filename, const void *buf,
				     size_t count) __access_r(3, 4);

extern int lxc_read_from_file(const char *filename, void *buf, size_t count)
    __access_w(2, 3);

/* send and receive buffers completely */
extern ssize_t lxc_write_nointr(int fd, const void *buf, size_t count) __access_r(2, 3);

extern ssize_t lxc_pwrite_nointr(int fd, const void *buf, size_t count, off_t offset)
    __access_r(2, 3);

extern ssize_t lxc_send_nointr(int sockfd, void *buf, size_t len, int flags)
    __access_r(2, 3);

extern ssize_t lxc_read_nointr(int fd, void *buf, size_t count) __access_w(2, 3);

__access_w(2, 3) static inline int lxc_read_string_nointr(int fd, char *buf,
							  size_t count)
{
	ssize_t ret;

	ret = lxc_read_nointr(fd, buf, STRARRAYLEN(buf));
	if (ret < 0)
		return -errno;
	if (ret == 0)
		return ret_errno(ENODATA);
	if ((size_t)ret >= sizeof(buf))
		return ret_errno(E2BIG);
	buf[ret] = '\0';
	return 0;
}

extern ssize_t lxc_read_nointr_expect(int fd, void *buf, size_t count,
					       const void *expected_buf) __access_w(2, 3);

extern ssize_t lxc_read_file_expect(const char *path, void *buf, size_t count,
					     const void *expected_buf) __access_w(2, 3);

extern ssize_t lxc_recv_nointr(int sockfd, void *buf, size_t len, int flags)
    __access_w(2, 3);

extern ssize_t lxc_recvmsg_nointr_iov(int sockfd, struct iovec *iov, size_t iovlen,
					       int flags);

extern bool file_exists(const char *f);
extern int print_to_file(const char *file, const char *content);
extern int lxc_is_dir(const char *path);
extern int lxc_count_file_lines(const char *fn);
extern int lxc_make_tmpfile(char *template, bool rm);

/* __typeof__ should be safe to use with all compilers. */
typedef __typeof__(((struct statfs *)NULL)->f_type) fs_type_magic;
extern bool has_fs_type(const char *path, fs_type_magic magic_val);
extern bool fhas_fs_type(int fd, fs_type_magic magic_val);
extern bool lxc_is_fs_type(const struct statfs *fs, fs_type_magic magic_val);
extern FILE *fopen_cloexec(const char *path, const char *mode);
extern ssize_t lxc_sendfile_nointr(int out_fd, int in_fd, off_t *offset, size_t count);
extern char *file_to_buf(const char *path, size_t *length);
extern int fd_to_buf(int fd, char **buf, size_t *length);
extern ssize_t __fd_to_fd(int from, int to);
static inline int fd_to_fd(int from, int to)
{
	return __fd_to_fd(from, to) >= 0;
}
extern int lxc_fd_cloexec(int fd, bool cloexec);
extern int lxc_open_dirfd(const char *dir);
extern FILE *fdopen_cached(int fd, const char *mode, void **caller_freed_buffer);
extern FILE *fdopen_at(int dfd, const char *path, const char *mode,
                                unsigned int o_flags,
                                unsigned int resolve_flags);
extern FILE *fopen_cached(const char *path, const char *mode, void **caller_freed_buffer);
extern int timens_offset_write(clockid_t clk_id, int64_t s_offset, int64_t ns_offset);
extern bool exists_dir_at(int dir_fd, const char *path);
extern bool exists_file_at(int dir_fd, const char *path);
extern int open_at(int dfd, const char *path, unsigned int o_flags,
			    unsigned int resolve_flags, mode_t mode);
extern int open_at_same(int fd_same, int dfd, const char *path,
				 unsigned int o_flags,
				 unsigned int resolve_flags, mode_t mode);
extern int open_beneath(int dfd, const char *path, unsigned int flags);
int fd_make_blocking(int fd);
int fd_make_nonblocking(int fd);
extern char *read_file_at(int dfd, const char *fnam,
                                   unsigned int o_flags,
                                   unsigned resolve_flags);
extern ssize_t lxc_read_try_buf_at(int dfd, const char *path,
                                            void *buf, size_t count);

/*
 * Check if two fds refer to the same file.
 * The function is "lax" in so far, as it doesn't care whether fda and fdb have
 * the same flags or whether they share the same device context when they refer
 * to devices.
 */
extern bool same_file_lax(int fda, int fdb);

static inline int dup_cloexec(int fd)
{
	__do_close int fd_dup = -EBADF;

	fd_dup = dup(fd);
	if (fd_dup < 0)
		return -errno;

	if (lxc_fd_cloexec(fd_dup, true))
		return -errno;

	return move_fd(fd_dup);
}

extern bool same_device(int fda, const char *patha, int fdb,
				 const char *pathb);

#endif /* __LXC_FILE_UTILS_H */
