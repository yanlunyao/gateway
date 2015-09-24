/*
 *	File name   : modifyRFDevRoomId.c
 *  Created on  : Sep 8, 2015
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
	char rfid[IEEE_LEN+1]={0};
	int new_roomid;

	char send_string[RF_RESPOND_TO_API_MAX_LEN]={0};

	char respond_string[RF_RESPOND_TO_API_MAX_LEN]={0};

	cgiFormResultType cgi_re;
	cgiHeaderContentType("application/json"); //MIME

	cgi_re = cgiFormString("rfid", rfid, IEEE_LEN + 1);
	cgi_re = cgiFormInteger("new_roomid", &new_roomid, -1);
	rfid[IEEE_LEN]= '\0';

	//communication with RF_Daemon
	snprintf(send_string, RF_RESPOND_TO_API_MAX_LEN, "{\n	\"api\": \"ModifyRFDevRoomId\",\n	\"rfid\": \"%s\",\n	\"new_roomid\": %d\n}", rfid, new_roomid);
	RF_DEBUG("%s\n", send_string);
	res = communicateWithRFRes(send_string, strlen(send_string)+1, respond_string);
	if(res ==0)
		fprintf(cgiOut,"%s\n", respond_string);
	else
		fprintf(cgiOut,"{\n	\"rf_daemon_comm_fail\": %d\n}\n", res);

	return 0;
}
