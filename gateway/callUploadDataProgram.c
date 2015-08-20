/*
 *	File name   : callUploadDataProgram.c
 *  Created on  : May 14, 2015
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

static void api_response(int res, const char *datatype, const char *cmdstring);
/*
 * api:	http://192.168.1.227/cgi-bin/rest/network/CallUploadDataProgram.cgi?datatype=1
 * */
int cgiMain()
{
	cgiFormResultType cgi_re;
	int res=0;
	char datatype[4]={0};
	char cmdstring[100] = "/gl/bin/BaseDataUpload";

	cgiHeaderContentType("application/json"); //MIME
	cgi_re = cgiFormString("datatype", datatype, sizeof(datatype)+1);

	if(cgi_re == cgiFormSuccess) {
		sprintf(cmdstring,"%s %s", "/gl/bin/BaseDataUpload", datatype);
	}
	res = system(cmdstring);
	//调用程序

	api_response(res, datatype, cmdstring);
	return 0;
}

static void api_response(int res, const char *datatype, const char *cmdstring)
{
	cJSON *root;
	char *json_out;

	root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "status", res);
	cJSON_AddStringToObject(root, "datatype", datatype);
	cJSON_AddStringToObject(root, "cmdstring", cmdstring);
	json_out=cJSON_Print(root);
	cJSON_Delete(root);

	fprintf(cgiOut,"%s\n", json_out);
	free(json_out);
}
