/* tpool.h
 *
 * Patrick MacArthur <generalpenguin89@gmail.com>
 */

#ifndef TPOOL_H
#define TPOOL_H

#include <unistd.h>

#if !defined(_POSIX_VERSION) || _POSIX_VERSION < 200112L
#error tpool.h requires a system that conforms to IEEE POSIX.1-2001 or a \
later version.
#endif

#define TASK_WANT_FUTURE (1 << 0)

/* Represents a value that will be known at some point in the future. */
typedef struct future FUTURE;

/* Represents a thread pool. */
typedef struct tpool TPOOL;

void *
future_get(FUTURE *future);

int
future_destroy(FUTURE *future);

#endif
/* vim: set shiftwidth=8 tabstop=8 noexpandtab : */
