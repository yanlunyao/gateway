/*
 *	File name   : getTimeActionList.c
 *  Created on  : Apr 14, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

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

static void api_response(int res, time_action_alllist_st *ta_list);

int cgiMain()
{
	//cgiFormResultType cgi_re;
	time_action_alllist_st ta_list;
	ta_list.time_action_base = NULL;
	int res=0;

	cgiHeaderContentType("application/json"); //MIME

	res = db_init();
	if(res<0){
		goto all_over;
	}
	res = get_timeaction_list_db(&ta_list.time_action_base, &ta_list.list_total);							//malloc
	if(res<0){
		goto all_over;
	}
all_over:
	db_close();
    api_response(res, &ta_list);

	//free
	if(ta_list.time_action_base){
		free(ta_list.time_action_base);
	}
	return 0;
}
static void api_response(int res, time_action_alllist_st *ta_list)
{
	cJSON *root;
    cJSON *list_array;
    cJSON *list_item;
	char *json_out;
	int i;

	root=cJSON_CreateObject();

	cJSON_AddNumberToObject(root, "status", res);
	if(res >=0){
		cJSON_AddItemToObject(root, "list", list_array =cJSON_CreateArray());
		for(i=0; i<ta_list->list_total; i++){
			cJSON_AddItemToArray(list_array, list_item =cJSON_CreateObject());
			cJSON_AddNumberToObject(list_item, "tid", ta_list->time_action_base[i].tid);
			cJSON_AddStringToObject(list_item, "actname", ta_list->time_action_base[i].actname);
			cJSON_AddStringToObject(list_item, "actpara", ta_list->time_action_base[i].actpara);
			cJSON_AddNumberToObject(list_item, "actmode", ta_list->time_action_base[i].actmode);
			cJSON_AddStringToObject(list_item, "para1", ta_list->time_action_base[i].para1);
			cJSON_AddNumberToObject(list_item, "para2", ta_list->time_action_base[i].para2);
			cJSON_AddStringToObject(list_item, "para3", ta_list->time_action_base[i].para3);
			cJSON_AddNumberToObject(list_item, "enable", ta_list->time_action_base[i].enable);
			//cJSON_AddItemToArray(list_array, list_item =cJSON_CreateObject());
		}
	}

	json_out=cJSON_Print(root);
	cJSON_Delete(root);

	fprintf(cgiOut,"%s\n", json_out);
	free(json_out);
}
