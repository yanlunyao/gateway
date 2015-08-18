/*
 *	File name   : auth_callback.c
 *  Created on  : Aug 11, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <stdlib.h>
#include "timedaction.h"
#include "auth_daemon.h"
#include "auth_callback.h"
#include "smartgateway.h"
#include "auth_push2cb.h"

timed_action_notifier* auth_notifier;
timed_action_t* expire_task;
timed_action_t* remiding_task;

//定时器初始化
void auth_multi_timer_init()
{
	auth_notifier = timed_action_mainloop_threaded(); //初始化多定时任务
	if(auth_notifier == NULL)
	{
		GDGL_DEBUG("multi timeaction task init error \n");
		exit(1);
	}
}

/*
 * 授权时间到达的callback函数：
 * 		如果时间未到，重新计时；
 * 		如果时间确实到达，发送续费提醒callback，再设置续费提醒定时任务；
 *
 * */
void expire_time_arrival_callback(void *arg)
{
	int now_time = time(NULL);
	printf("expire_time_arrival_callback, now timestamp = %d\n", now_time);

	if(expire_task!=NULL) {    //延时时间到达后要不要把任务关闭？
		timed_action_unschedule(auth_notifier, expire_task);
		free(expire_task);
		expire_task = NULL;
		GDGL_DEBUG("expire_time_arrival_callback, timed_action_unschedule expire_task\n");
	}

	if(auth_limit.expire_timestamp <= now_time) {

//		if(remiding_task !=NULL) {
//			GDGL_DEBUG("remiding_task not null\n");
//			timed_action_unschedule(auth_notifier, remiding_task);
//		}
		setting_auth_limit_state(ARREARAGE_STATE);
		update_auth_json_file_by_auth_st(&auth_limit);
		send_reminding_callback(auth_limit.expire_time);

		if(auth_limit.nolonger_remind_time <=now_time) {
			GDGL_DEBUG("nolonger_remind_time == expire_timestamp, service over\n");
			setting_auth_limit_available(AVAILABLE_INVALID);
			update_auth_json_file_by_auth_st(&auth_limit);
			send_service_stop_callback();
		}
		else {
			GDGL_DEBUG("timed_action_schedule_periodic, delaytime=%d\n",REMIND_PERIOD_TIME);
			remiding_task = timed_action_schedule_periodic(auth_notifier, REMIND_PERIOD_TIME, 0, &remiding_time_arrival_callback, NULL);
		}
	}
	else {//如果时间未到，重新计时
		GDGL_DEBUG("timed_action_schedule, delaytime=%ld\n",auth_limit.expire_timestamp-now_time);
		expire_task = timed_action_schedule(auth_notifier, auth_limit.expire_timestamp-now_time, 0, &expire_time_arrival_callback, NULL);
	}

}

//定时发送续费提醒的callback函数
void remiding_time_arrival_callback(void *arg)
{
	int interval;

	int now_time = time(NULL);
	printf("remiding_time_arrival_callback, now timestamp = %d\n", now_time);

	send_reminding_callback(auth_limit.expire_time);

	interval = auth_limit.nolonger_remind_time - now_time;
	if(interval >= REMIND_PERIOD_TIME) {

	}
	else if((interval > 0)&&(interval < REMIND_PERIOD_TIME)) {
		timed_action_unschedule(auth_notifier, remiding_task);
		free(remiding_task);
		remiding_task = NULL;
		remiding_task = timed_action_schedule_periodic(auth_notifier, interval, 0, &remiding_time_arrival_callback, NULL);
	}
	else {
		GDGL_DEBUG("remiding_task over, service over\n");
		setting_auth_limit_available(AVAILABLE_INVALID);
		update_auth_json_file_by_auth_st(&auth_limit);
		send_service_stop_callback();
		timed_action_unschedule(auth_notifier, remiding_task);
		free(remiding_task);
		remiding_task = NULL;
	}
}
