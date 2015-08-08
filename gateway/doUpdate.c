/*
 *	File name   : doUpdate.c
 *  Created on  : Jul 15, 2015
 *  Author      : yanly
 *  Description : 执行网关升级的cgi：实际上不直接执行，而是调用程序DoUpdate
 *
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include "cJSON.h"
#include "cgic.h"
#include "glCalkProtocol.h"
int cgiMain()
{
	int res=0;

    cgiHeaderContentType("application/json"); //MIME
    fflush(stdout);

    pid_t	pid;
	if ((pid = fork()) > 0) {						//parent process
	    cJSON *root;
	    char *json_out;
	    root=cJSON_CreateObject();
	    cJSON_AddNumberToObject(root, "status", res);
	    json_out=cJSON_Print(root);
	    fprintf(cgiOut,"%s\n", json_out);
	    free(json_out);
	    //==========================
		cJSON_AddNumberToObject(root, "msgtype", 0);
		cJSON_AddNumberToObject(root, "mainid", 6);
		cJSON_AddNumberToObject(root, "subid", 1);
		json_out=cJSON_Print(root);
		json_out[strlen(json_out)] = '\0';
	    push_to_CBDaemon(json_out, strlen(json_out)+1);
		free(json_out);
		cJSON_Delete(root);
	}
	else
	{												//child process
//		int res;
//		printf("/sbin/reboot\n");
//		res = system("/sbin/reboot");
//		printf("res=%d\n",res);
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		//重定向输入输出流
		freopen("/dev/null", "r", stdin);
		freopen("/dev/null", "w", stdout);
		freopen("/dev/null", "w", stderr);
		system("/gl/bin/DoUpdate &> /gl/log/DoUpdate@`date +%Y%m%d%H%M%S`.log");
		exit(0);
	}
    //read sw_version
	return 0;
}
