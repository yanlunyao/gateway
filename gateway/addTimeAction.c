/*
 *	File name   : addTimeAction.c
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
extern int gnerate_per_urlobj(url_act_st *object, const char *text);

int cgiMain()
{
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;
	time_action_st time_act;
	cgiFormResultType cgi_re;
	int res=0;

	cgiHeaderContentType("application/json"); //MIME

	cgi_re = cgiFormString("actname", time_act.ta_base.actname, NAME_STRING_LEN + 1);
	cgi_re = cgiFormString("actpara", time_act.ta_base.actpara, ACTPARA_LEN + 1);
	cgi_re = cgiFormInteger("actmode", &time_act.ta_base.actmode, 0);
//	cgi_re = cgiFormInteger("acttype", (int *)&time_act.acttype, 0);
	cgi_re = cgiFormString("para1", time_act.ta_base.para1, OUT_TIME_FORMAT_LEN + 1);
	cgi_re = cgiFormInteger("para2", &time_act.ta_base.para2, 0);
	cgi_re = cgiFormString("para3", time_act.ta_base.para3, PARA3_LEN + 1);
	cgi_re = cgiFormInteger("enable", &time_act.ta_base.enable, 0);
//	printf("%s, %s, %d, %s, %d, %s, %d\n", time_act.actname, time_act.actpara, time_act.actmode, time_act.para1, time_act.para2, time_act.para3,
//			time_act.enable);
	//generate scene_action_st
//	url_act_st object;
	res = gnerate_per_urlobj(&time_act.urlobject, time_act.ta_base.actpara);
//	res = gnerate_per_urlobj(&object, time_act.actpara);
	if(res <0){
		goto all_over;
	}
//	printf("%s, %s\n", object.urlstring, object.actobj);
//	printf("%s, %s, %d, %s, %d, %s, %d\n", time_act.actname, time_act.actpara, time_act.actmode, time_act.para1, time_act.para2, time_act.para3,
//				time_act.enable);
//	snprintf(time_act.urlobject.urlstring, URL_STRING_LEN, object.urlstring);
//	snprintf(time_act.urlobject.actobj, IEEE_LEN+1, object.actobj);
//	printf("actobj=%s,url=%s\n",  time_act.urlobject.actobj,time_act.urlobject.urlstring);
//	wrtie database
	int id_value;
	res = add_timeaction_db(&id_value, &time_act);
	if(res<0){
		goto all_over;
	}
	time_act.ta_base.tid =  id_value;
all_over:

    //all right
    api_response(res, time_act.ta_base.tid, &time_act.ta_base);
    //push to cb_daemon
	if((send_cb_len = cJsonTimeAction_callback(send_cb_string, ADD_TA_SUBID, res, time_act.ta_base.tid, &time_act.ta_base)) >=0) {
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
