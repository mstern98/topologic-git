/**
 * SRC: https://nachtimwald.com/2019/04/05/cross-platform-thread-wrapper/
**/

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include "../include/windows_wrap.h"

int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
    (void)attr;
    if (!thread || !start_routine)
        return 1;

    *thread = CreateThread(NULL, 0, start_routine, arg, 0, NULL);
    if (!(*thread))
        return 1;
    return 0;
}

int pthread_detach(phtread_t thread)
{
    CloseHandle(thread);
}

int pthread_exit(void *retval)
{
    ExitThread(retval);
}

int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t *attr)
{
    (void)attr;

    if (mutex == NULL)
        return 1;

    InitializeCriticalSection(mutex);
    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    if (!mutex)
        return 1;
    DeleteCriticalSection(mutex);
    return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    if (!mutex)
        return 1;
    EnterCriticalSection(mutex);
    return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    if (!mutex)
        return 1;
    LeaveCriticalSection(mutex);
    return 0;
}

int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *attr)
{
    (void)attr;
    if (!cond)
        return 1;
    InitializeConditionVariable(cond);
    return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
    (void)cond;
    return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    if (!cond || !mutex)
        return 1;
    if (!SleepConditionVariableCS(cond, mutex, NULL))
        return 1;
    return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    if (!cond)
        return 1;
    WakeConditionVariable(cond);
    return 0;
}

#endif
