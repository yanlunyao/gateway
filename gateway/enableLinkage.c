/*
 *	File name   : enableLinkage.c
 *  Created on  : Apr 27, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

/*
 *	File name   : editLinkage.c
 *  Created on  : Apr 24, 2015
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

static void api_response(int res, int id_value, int enable);

int cgiMain()
{
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;
	char sql[SQL_STRING_MAX_LEN];

	int id_value;
	int enable_flag;

	cgiFormResultType cgi_re;
	int res=0;

	cgiHeaderContentType("application/json"); //MIME

	cgi_re = cgiFormInteger(FIELD_LID, &id_value, 0);
	cgi_re = cgiFormInteger(FIELD_ENABLE, &enable_flag, 0);

//	wrtie database
	res = db_init();
	if(res<0){
		goto all_over;
	}
	sprintf(sql, "UPDATE t_linkage SET enable=%d WHERE lid=%d", enable_flag, id_value);
	res = enable_linkage_db(sql);
	if(res<0){
		goto all_over;
	}
all_over:
	db_close();
    api_response(res, id_value, enable_flag);

	if((send_cb_len = cJsonEnableLinkage_callback(send_cb_string, SUBID_ENABLE_LINK, res, id_value, enable_flag)) >=0) {
		push_to_CBDaemon(send_cb_string, send_cb_len);
	}
	return 0;
}
static void api_response(int res, int id_value, int enable)
{
	cJSON *root;
	char *json_out;

	root=cJSON_CreateObject();

	cJSON_AddNumberToObject(root, FIELD_STATUS, res);
	if(res >=0){
		cJSON_AddNumberToObject(root, FIELD_LID, id_value);
		cJSON_AddNumberToObject(root, FIELD_ENABLE, enable);
	}

	json_out=cJSON_Print(root);
	cJSON_Delete(root);

	fprintf(cgiOut,"%s\n", json_out);
	free(json_out);
}

