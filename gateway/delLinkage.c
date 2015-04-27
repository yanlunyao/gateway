/*
 *	File name   : delLinkage.c
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


static void api_response(int res, int id_value ,const linkage_base_st *link_base);

int cgiMain()
{
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;

	int id_value;

	cgiFormResultType cgi_re;
	int res=0;

	cgiHeaderContentType("application/json"); //MIME

	cgi_re = cgiFormInteger(FIELD_LID, &id_value, 0);

//	wrtie database
	res = db_init();
	if(res<0){
		goto all_over;
	}
	res = del_linkage_db(id_value);
	if(res<0){
		goto all_over;
	}
all_over:
	db_close();
    api_response(res, id_value, NULL);

	if((send_cb_len = cJsonLinkage_callback(send_cb_string, SUBID_DEL_LINK, res, id_value, NULL)) >=0) {
		push_to_CBDaemon(send_cb_string, send_cb_len);
	}
	return 0;
}
static void api_response(int res, int id_value ,const linkage_base_st *link_base)
{
	cJSON *root;
	char *json_out;

	root=cJSON_CreateObject();

	cJSON_AddNumberToObject(root, FIELD_STATUS, res);
	if(res >=0){
		cJSON_AddNumberToObject(root, FIELD_LID, id_value);
		if(link_base !=NULL)
		{
			cJSON_AddStringToObject(root, FIELD_LNKNAME, link_base->lnkname);
			cJSON_AddStringToObject(root, FIELD_TRGIEEE, link_base->trgieee);
			cJSON_AddStringToObject(root, FIELD_TRGEP, link_base->trgep);
			cJSON_AddStringToObject(root, FIELD_TRGCND, link_base->trgcnd);
			cJSON_AddStringToObject(root, FIELD_LNKACT, link_base->lnkact);
			cJSON_AddNumberToObject(root, FIELD_ENABLE, link_base->enable);
		}
	}

	json_out=cJSON_Print(root);
	cJSON_Delete(root);

	fprintf(cgiOut,"%s\n", json_out);
	free(json_out);
}

