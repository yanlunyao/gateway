/*
 *	File name   : delTimeAction.c
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

static void api_response(int res, int id_value, const time_action_base_st *time_act);

int cgiMain()
{
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;
	cgiFormResultType cgi_re;
	int res=0;
	int tid;

	cgiHeaderContentType("application/json"); //MIME

	cgi_re = cgiFormInteger("tid", &tid, 0);

//	delete database
	res = del_timeaction_db(tid);
	if(res<0){
		goto all_over;
	}
all_over:

    //all right
    api_response(res, tid, NULL);
    //push to cb_daemon
	if((send_cb_len = cJsonTimeAction_callback(send_cb_string, DEL_TA_SUBID, res, tid, NULL)) >=0) {
		//if push failed, not handle temporary
		push_to_CBDaemon(send_cb_string, send_cb_len);
	}
	return 0;
}
static void api_response(int res, int id_value, const time_action_base_st *time_act)
{
	cJSON *root;
	char *json_out;

	root=cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "status", res);
	if(res >=0){
		cJSON_AddNumberToObject(root, "tid", id_value);
		if(time_act !=NULL)
		{
			cJSON_AddStringToObject(root, "actname", time_act->actname);
	    	cJSON_AddStringToObject(root, "actpara", time_act->actpara);
	    	cJSON_AddNumberToObject(root, "actmode", time_act->actmode);
	    	cJSON_AddStringToObject(root, "para1", time_act->para1);
	    	cJSON_AddNumberToObject(root, "para2", time_act->para2);
	    	cJSON_AddStringToObject(root, "para3", time_act->para3);
	    	cJSON_AddNumberToObject(root, "enable", time_act->enable);
		}
	}

	json_out=cJSON_Print(root);
	cJSON_Delete(root);

	fprintf(cgiOut,"%s\n", json_out);
	free(json_out);
}
