/*
 *	File name   : getLinkageList.c
 *  Created on  : Apr 27, 2015
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
#include "callbackProtocolField.h"


static void api_response(int res, int total, const linkage_base_st *link_base);

int cgiMain()
{
	linkage_base_st *base_all_list;
	int total_num;
	int res=0;

	cgiHeaderContentType("application/json"); //MIME

	res = db_init();
	if(res<0){
		goto all_over;
	}
	res = get_linkage_base_all_list_db(&base_all_list, &total_num);
	if(res<0){
		goto all_over;
	}
all_over:
	db_close();
    api_response(res, total_num, base_all_list);
	return 0;
}
static void api_response(int res, int total, const linkage_base_st *link_base)
{
	cJSON *root;
    cJSON *list_array;
    cJSON *list_item;
	char *json_out;
	int i;

	root=cJSON_CreateObject();

	cJSON_AddNumberToObject(root, FIELD_STATUS, res);

	if(res >=0){
		cJSON_AddItemToObject(root, "list", list_array =cJSON_CreateArray());
		for(i=0; i<total; i++){
			cJSON_AddItemToArray(list_array, list_item =cJSON_CreateObject());
			cJSON_AddNumberToObject(list_item, FIELD_LID, link_base[i].lid);
			cJSON_AddStringToObject(list_item, FIELD_LNKNAME, link_base[i].lnkname);
			cJSON_AddStringToObject(list_item, FIELD_TRGIEEE, link_base[i].trgieee);
			cJSON_AddStringToObject(list_item, FIELD_TRGEP, link_base[i].trgep);
			cJSON_AddStringToObject(list_item, FIELD_TRGCND, link_base[i].trgcnd);
			cJSON_AddStringToObject(list_item, FIELD_LNKACT, link_base[i].lnkact);
			cJSON_AddNumberToObject(list_item, FIELD_ENABLE, link_base[i].enable);
		}
	}

	json_out=cJSON_Print(root);
	cJSON_Delete(root);

	fprintf(cgiOut,"%s\n", json_out);
	free(json_out);
}

