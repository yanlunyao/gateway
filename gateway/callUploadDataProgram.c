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
#include <unistd.h>
#include "cgic.h"
#include "cJSON.h"

static void api_response(int res, int datatype, const char *cmdstring);
/*
 * api:	http://192.168.1.227/cgi-bin/rest/network/CallUploadDataProgram.cgi?datatype=1
 * */
int cgiMain()
{
	cgiFormResultType cgi_re;
	int res=0;
	int datatype;
	char cmdstring[100] = {0};

	cgiHeaderContentType("application/json"); //MIME
	cgi_re = cgiFormInteger("datatype", &datatype, -1);

	if(datatype ==-1)
		snprintf(cmdstring, sizeof(cmdstring), "%s", "/gl/bin/BaseDataUpload");
	else
		snprintf(cmdstring, sizeof(cmdstring), "%s %d", "/gl/bin/BaseDataUpload", datatype);

    fflush(stdout);
    pid_t	pid;
	if ((pid = fork()) > 0) {						//parent process
	}
	else
	{												//child process
		//重定向输入输出流
		freopen("/dev/null", "r", stdin);
		freopen("/dev/null", "w", stdout);
		freopen("/dev/null", "w", stderr);
		system(cmdstring);
		exit(0);
	}
	res =0;
	api_response(res, datatype, cmdstring);
	return 0;
}

static void api_response(int res, int datatype, const char *cmdstring)
{
	cJSON *root;
	char *json_out;

	root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "status", res);
	cJSON_AddNumberToObject(root, "datatype", datatype);
	cJSON_AddStringToObject(root, "cmdstring", cmdstring);
	json_out=cJSON_Print(root);
	cJSON_Delete(root);

	fprintf(cgiOut,"%s\n", json_out);
	free(json_out);
}
