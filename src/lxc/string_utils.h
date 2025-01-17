/* SPDX-License-Identifier: LGPL-2.1+ */

#ifndef __LXC_STRING_UTILS_H
#define __LXC_STRING_UTILS_H

#include "config.h"

#include <stdarg.h>

#include "initutils.h"
#include "macro.h"

#if !HAVE_STRLCAT
#include "strlcat.h"
#endif

#if !HAVE_STRLCPY
#include "strlcpy.h"
#endif

#if !HAVE_STRCHRNUL
#include "strchrnul.h"
#endif

/* convert variadic argument lists to arrays (for execl type argument lists) */
extern char **lxc_va_arg_list_to_argv(va_list ap, size_t skip, int do_strdup);
extern const char **lxc_va_arg_list_to_argv_const(va_list ap, size_t skip);

/*
 * Some simple string functions; if they return pointers, they are allocated
 * buffers.
 */
extern char *lxc_string_replace(const char *needle, const char *replacement,
					 const char *haystack);
extern bool lxc_string_in_array(const char *needle, const char **haystack);
extern char *lxc_string_join(const char *sep, const char **parts, bool use_as_prefix);

extern char *lxc_append_paths(const char *first, const char *second);

/*
 * Note: the following two functions use strtok(), so they will never
 *       consider an empty element, even if two delimiters are next to
 *       each other.
 */
extern bool lxc_string_in_list(const char *needle, const char *haystack, char sep);
extern char **lxc_string_split(const char *string, char sep);
extern char **lxc_string_split_and_trim(const char *string, char sep);
extern char **lxc_string_split_quoted(char *string);

/* Append string to NULL-terminated string array. */
extern int lxc_append_string(char ***list, char *entry);

/* Some simple array manipulation utilities */
typedef void (*lxc_free_fn)(void *);
typedef void *(*lxc_dup_fn)(void *);
extern int lxc_grow_array(void ***array, size_t *capacity, size_t new_size,
				   size_t capacity_increment);
extern void lxc_free_array(void **array, lxc_free_fn element_free_fn);
extern size_t lxc_array_len(void **array);

extern void **lxc_append_null_to_array(void **array, size_t count);
extern void remove_trailing_newlines(char *l);

/* Helper functions to parse numbers. */
extern int lxc_safe_uint(const char *numstr, unsigned int *converted);
extern int lxc_safe_int(const char *numstr, int *converted);
extern int lxc_safe_long(const char *numstr, long int *converted);
extern int lxc_safe_long_long(const char *numstr, long long int *converted);
extern int lxc_safe_ulong(const char *numstr, unsigned long *converted);
extern int lxc_safe_uint64(const char *numstr, uint64_t *converted, int base);
extern int lxc_safe_int64_residual(const char *restrict numstr,
					    int64_t *restrict converted,
					    int base, char *restrict residual,
					    size_t residual_len);
/* Handles B, kb, MB, GB. Detects overflows and reports -ERANGE. */
extern int parse_byte_size_string(const char *s, long long int *converted);

/*
 * Concatenate all passed-in strings into one path. Do not fail. If any piece
 * is not prefixed with '/', add a '/'.
 */
__attribute__((sentinel)) extern char *must_concat(size_t *len, const char *first, ...);
__attribute__((sentinel)) extern char *must_make_path(const char *first, ...);
__attribute__((sentinel)) extern char *must_append_path(char *first, ...);

#define must_make_path_relative(__first__, ...)                                \
	({                                                                     \
		char *__ptr__;                                                 \
		if (*__first__ == '/')                                         \
			__ptr__ = must_make_path(".", __first__, __VA_ARGS__); \
		else                                                           \
			__ptr__ = must_make_path(__first__, __VA_ARGS__);      \
		__ptr__;                                                       \
	})

/* Return copy of string @entry. Do not fail. */
extern char *must_copy_string(const char *entry);

/* Re-allocate a pointer, do not fail */
extern void *must_realloc(void *orig, size_t sz);

extern int lxc_char_left_gc(const char *buffer, size_t len);

extern int lxc_char_right_gc(const char *buffer, size_t len);

extern char *lxc_trim_whitespace_in_place(char *buffer);

extern int lxc_is_line_empty(const char *line);
extern void remove_trailing_slashes(char *p);

static inline bool is_empty_string(const char *s)
{
	return !s || strcmp(s, "") == 0;
}

#define maybe_empty(s) ((!is_empty_string(s)) ? (s) : ("(null)"))

static inline ssize_t safe_strlcat(char *src, const char *append, size_t len)
{
	size_t new_len;

	new_len = strlcat(src, append, len);
	if (new_len >= len)
		return ret_errno(EINVAL);

	return (ssize_t)new_len;
}

static inline bool strnequal(const char *str, const char *eq, size_t len)
{
	return strncmp(str, eq, len) == 0;
}

static inline bool strequal(const char *str, const char *eq)
{
	return strcmp(str, eq) == 0;
}

static inline bool dotdot(const char *str)
{
	return !!strstr(str, "..");
}

static inline bool abspath(const char *str)
{
	return *str == '/';
}

static inline char *deabs(char *str)
{
	return str + strspn(str, "/");
}

#define strnprintf(buf, buf_size, ...)                                                    \
	({                                                                                \
		int __ret_strnprintf;                                                     \
		__ret_strnprintf = snprintf(buf, buf_size, ##__VA_ARGS__);                \
		if (__ret_strnprintf < 0 || (size_t)__ret_strnprintf >= (size_t)buf_size) \
			__ret_strnprintf = ret_errno(EIO);				  \
		__ret_strnprintf;                                                         \
	})

static inline const char *proc_self_fd(int fd)
{
	static const char *invalid_fd_path = "/proc/self/fd/-EBADF";
	static char buf[LXC_PROC_SELF_FD_LEN] = "/proc/self/fd/";

	if (strnprintf(buf + STRLITERALLEN("/proc/self/fd/"),
		       INTTYPE_TO_STRLEN(int), "%d", fd) < 0)
		return invalid_fd_path;

	return buf;
}

static inline const char *fdstr(__s64 fd)
{
	static const char *fdstr_invalid = "-EBADF";
	static char buf[INTTYPE_TO_STRLEN(__s64)];

	if (strnprintf(buf, sizeof(buf), "%lld", (long long signed int)fd) < 0)
		return fdstr_invalid;

	return buf;
}

#define lxc_iterate_parts(__iterator, __splitme, __separators)                  \
	for (char *__p = NULL, *__it = strtok_r(__splitme, __separators, &__p); \
	     (__iterator = __it);                                               \
	     __iterator = __it = strtok_r(NULL, __separators, &__p))

extern char *lxc_path_simplify(const char *path);

#endif /* __LXC_STRING_UTILS_H */
