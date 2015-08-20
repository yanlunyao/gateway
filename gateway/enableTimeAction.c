/*
 *	File name   : enableTimeAction.c
 *  Created on  : Apr 14, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cgic.h"
#include "cJSON.h"
#include "sqliteOperator.h"
#include "glCalkProtocol.h"
#include <time.h>

static void api_response(int res, int id_value, int do_status);

int cgiMain()
{
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;

	cgiFormResultType cgi_re;
	int res;
	int id_value, do_status;

	cgiHeaderContentType("application/json"); //MIME

	char *ipaddr = getenv("REMOTE_ADDR");
	URLAPI_DEBUG("remote ip=%s\n",ipaddr);

	cgi_re = cgiFormInteger("tid", &id_value, 0);
	cgi_re = cgiFormInteger("enable", &do_status, 0);

	res = db_init();
	if(res <0) {
		goto all_over;
	}
	res = do_time_action_db(id_value, do_status);
	if(res <0) {
		goto all_over;
	}

all_over:
	db_close();
	api_response(res, id_value, do_status);

	if((send_cb_len = cJsonEnableTimeAction_callback(send_cb_string, ENABLE_TA_SUBID, res, id_value, do_status)) >=0) {
		push_to_CBDaemon(send_cb_string, send_cb_len);
		URLAPI_DEBUG("[time]=%ld,callback from [enableTimeAction]=%s\n", time(NULL), send_cb_string);
	}
	return 0;
}
static void api_response(int res, int id_value, int do_status)
{
	cJSON *root;
	char *json_out;

	root=cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "status", res);
	if(res >=0){
		cJSON_AddNumberToObject(root, "tid", id_value);
		cJSON_AddNumberToObject(root, "enable", do_status);
	}

	json_out=cJSON_Print(root);
	cJSON_Delete(root);

	fprintf(cgiOut,"%s\n", json_out);
	free(json_out);
}

