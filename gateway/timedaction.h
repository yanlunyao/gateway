/*
 *基于epoll timefd实现多定时器任务
 * */
#ifndef __TIMED_ACTION_H__
#define __TIMED_ACTION_H__

#include <pthread.h>

/**
 * Opaque type. Timed action handle.
 */
typedef struct {
    int tfd;
    void (*timed_action_handler)(void*);
    void* arg;
} timed_action_t;

/**
 * Opaque type. Notifies user by callback when timer expires.
 */
typedef struct {
    int epfd;
    pthread_t th;
} timed_action_notifier;

/**
 * Create a threaded mainloop for even handling. 
 */
timed_action_notifier* timed_action_mainloop_threaded();

/**
 * description: Schedule a delayed action (callback) at a given time
 */
timed_action_t* timed_action_schedule(timed_action_notifier* notifier, time_t sec, long nsec, void (*timed_action_handler)(void*), void* arg);

/**
 * Schedule a periodic action (callback) at a given time
 */
timed_action_t* timed_action_schedule_periodic(timed_action_notifier* notifier, time_t sec, long nsec, void (*timed_action_handler)(void*), void* arg);

/**
 * Unschedule an action
 */
int timed_action_unschedule(timed_action_notifier* notifier, timed_action_t* action);

#endif
