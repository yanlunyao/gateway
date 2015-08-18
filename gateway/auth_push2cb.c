/*
 *	File name   : auth_push2cb.c
 *  Created on  : Aug 11, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <string.h>
#include <stdio.h>
#include "glCalkProtocol.h"
#include "auth_push2cb.h"

const char reminding[] = "{\n	\"msgtype\": 0,\n	\"mainid\": 8,\n	\"subid\": 1,\n	\"expire_time\": \"%s\"\n}";
const char stopServiceString[] = "{\n	\"msgtype\": 0,\n	\"mainid\": 8,\n	\"subid\": 2\n}";

//发送续费提醒的callback
void send_reminding_callback(const char *expire_time)
{
	char temp[1024] = {0};
	snprintf(temp, sizeof(temp), reminding, expire_time);
	push_to_CBDaemon(temp, strlen(temp)+1);
}

//发送服务终止的callback
void send_service_stop_callback()
{
	push_to_CBDaemon(stopServiceString, strlen(stopServiceString)+1);
}

