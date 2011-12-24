/* future.c
 *
 * Patrick MacArthur <generalpenguin89@gmail.com>
 *
 * Functions that manipulate a FUTURE *, a value which may be obtained in the
 * future.  These may be returned when a task is submitted to a thread pool, and
 * their value is later obtained by calling future_get().  After the value has
 * been obtained, the future should be destroyed with future_destroy().
 */

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "tpool.h"
#include "tpool_private.h"

/* Initializes a future, assuming that it has already been allocated.  Only
 * meant to be called internally.  External applications should get a future
 * only through calls to tpool_submit(). */
int
future_init(FUTURE *future)
{
	int errcode;

	if ((errcode = pthread_mutex_init(&future->f_mutex, NULL)) != 0) {
		goto exit0;
	}

	if ((errcode = pthread_cond_init(&future->f_cond, NULL)) != 0) {
		goto fail1;
	}

	errcode = 0;
	goto exit0;

fail1:
	pthread_mutex_destroy(&future->f_mutex);

exit0:
	return errcode;
}

/* Sets the value of the future.  Should only be called from the worker threads.
 */
void
future_set(FUTURE *future, void *value)
{
	/* if this fails, setting of f_ready should be "atomic" anyways, so
	 * we're probably all right */
	pthread_mutex_lock(&future->f_mutex);

	future->f_value = value;
	future->f_ready = 1;
	pthread_mutex_unlock(&future->f_mutex);
	pthread_cond_signal(&future->f_cond);
}

/* Returns a future value.  Returns NULL if there was an error; otherwise
 * returns the value held within the future.  Note that this function will block
 * until the value is ready. */
void *
future_get(FUTURE *future)
{
	void *retval;
	int errcode;

	if ((errcode = pthread_mutex_lock(&future->f_mutex)) != 0) {
		errno = errcode;
		return NULL;
	}
	while (!future->f_ready) {
		pthread_cond_wait(&future->f_cond, &future->f_mutex);
	}

	retval = future->f_value;
	pthread_mutex_unlock(&future->f_mutex);

	return retval;
}

/* Destroys a future object.  Should only be called after the value is ready and
 * has been retrieved.  After destroying the future, no attempt should be made
 * to use it again.  On success, returns 0.  On failure, returns the error code.
 * Will return EBUSY if the future value is not yet ready. */
int
future_destroy(FUTURE *future)
{
	int errcode;
	int retval;
	pthread_mutex_lock(&future->f_mutex);
	if (!future->f_ready) {
		return EBUSY;
	}

	retval = 0;
	if ((errcode = pthread_cond_destroy(&future->f_cond)) != 0) {
		retval = errcode;
	}
	if ((errcode = pthread_mutex_destroy(&future->f_mutex)) != 0) {
		retval = errcode;
	}
	free(future);

	return retval;
}

/* vim: set shiftwidth=8 tabstop=8 noexpandtab : */
