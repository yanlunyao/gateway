/*
 *	File name   : auth_callback.h
 *  Created on  : Aug 11, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef GATEWAY_AUTH_CALLBACK_H_
#define GATEWAY_AUTH_CALLBACK_H_

extern timed_action_notifier* auth_notifier;
extern timed_action_t* expire_task;
extern timed_action_t* remiding_task;

void auth_multi_timer_init();
void expire_time_arrival_callback(void *arg);
void remiding_time_arrival_callback(void *arg);

#endif /* GATEWAY_AUTH_CALLBACK_H_ */
