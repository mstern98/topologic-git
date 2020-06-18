/**
 * SRC: https://nachtimwald.com/2019/04/05/cross-platform-thread-wrapper/
**/

#ifndef __TOPILOGIC_WINDOWS__
#define __TOPILOGIC_WINDOWS__

#include <windows.h>
#include <process.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef void pthread_mutexattr_t;
typedef void pthread_condattr_t;
typedef void pthread_attr_t;
typedef CRITICAL_SECTION pthread_mutex_t;
typedef HANDLE pthread_t;
typedef CONDITION_VARIABLE pthread_cond_t;

int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
int pthread_detach(pthread_t);
int pthread_exit(void *retval);

int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_signal(pthread_cond_t *cond);

#ifdef __cplusplus
}
#endif

#endif
