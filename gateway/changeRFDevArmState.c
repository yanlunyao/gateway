/*
 *	File name   : changeRFDevArmState.c
 *  Created on  : Aug 3, 2015
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
	int state;

	char send_string[RF_RESPOND_TO_API_MAX_LEN]={0};

	char respond_string[RF_RESPOND_TO_API_MAX_LEN]={0};

	cgiFormResultType cgi_re;
	cgiHeaderContentType("application/json"); //MIME

	//获取rfid，state
	cgi_re = cgiFormString("rfid", rfid, IEEE_LEN + 1);
	cgi_re = cgiFormInteger("state", &state, -1);
	rfid[IEEE_LEN]= '\0';

	//communication with RF_Daemon
	snprintf(send_string, RF_RESPOND_TO_API_MAX_LEN, "{\n	\"api\": \"ChangeRFDevArmState\",\n	\"rfid\": \"%s\",\n	\"state\": \"%d\"\n}", rfid,state);
	RF_DEBUG("%s\n", send_string);
	res = communicateWithRF(send_string, strlen(send_string)+1, respond_string);
//	if(res !=0) {
		fprintf(cgiOut,"{\n	\"status\": %d\n}\n", res);
//	}
//	fprintf(cgiOut, "%s\n", respond_string);

	return 0;
}
