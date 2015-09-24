/*
 *	File name   : notifyRoomIdHasBeenDeleted.c
 *  Created on  : Sep 14, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cgic.h"
#include "smartgateway.h"
#include "apiComWithRFDaemon.h"

int cgiMain()
{
	int res=0;
	char send_string[RF_RESPOND_TO_API_MAX_LEN]={0};
	char respond_string[RF_RESPOND_TO_API_MAX_LEN]={0};
	int roomid;

	cgiHeaderContentType("application/json"); //MIME
	cgiFormInteger("roomid", &roomid, -1);

	//communication with RF_Daemon
	snprintf(send_string, RF_RESPOND_TO_API_MAX_LEN, "{\n	\"api\": \"NotifyRoomIdHasBeenDeleted\",\n	\"roomid\": %d\n}", roomid);
	RF_DEBUG("%s\n", send_string);
	res = communicateWithRFRes(send_string, strlen(send_string)+1, respond_string);
	if(res ==0)
		fprintf(cgiOut,"%s\n", respond_string);
	else
		fprintf(cgiOut,"{\n	\"rf_daemon_comm_fail\": %d\n}\n", res);
	return 0;
}
