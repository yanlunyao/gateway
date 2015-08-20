/*
 *	File name   : changeAllRFDevArmState.c
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
	int state;

	char send_string[RF_RESPOND_TO_API_MAX_LEN]={0};
	char respond_string[RF_RESPOND_TO_API_MAX_LEN]={0};

	cgiFormResultType cgi_re;
	cgiHeaderContentType("application/json"); //MIME

	//获取state
	cgi_re = cgiFormInteger("state", &state, -1);

	//communication with RF_Daemon
	snprintf(send_string, RF_RESPOND_TO_API_MAX_LEN, "{\n	\"api\": \"ChangeAllRFDevArmState\",\n	\"state\": %d\n}", state);
	RF_DEBUG("%s\n", send_string);
	res = communicateWithRF(send_string, strlen(send_string)+1, respond_string);
//	if(res !=0) {
		fprintf(cgiOut,"{\n	\"status\": %d\n}\n", res);
//	}
//	fprintf(cgiOut, "%s\n", respond_string);

	return 0;
}
