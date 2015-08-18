/*
 *	File name   : invokeBaseDataUpldPrm.c
 *  Created on  : May 15, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */


#include "unpthread.h"
#include "smartgateway.h"
#include "invokeBaseDataUpldPrm.h"
#include "baseDataUpload.h"

#define EXEC_PROGRAM_PATH		"/gl/bin/BaseDataUpload"



static void *system_call(void *arg)
{
	int res;
	char cmd[100]=EXEC_PROGRAM_PATH;

	res = system(cmd);
	if(res <0) {
		GDGL_DEBUG("system call failed,res is: %d", res);
	}
	return(NULL);
}
//
//void invoke_by_datatype_pthread(int datatype, const char *para)
//{
//	char cmd[100] = {0};
//	pthread_t	tid;
//
//	if(datatype >DATATYPE_MAX) {
//		GDGL_DEBUG("datatype invalid\n");
//		return ;
//	}
//
//	if(para!=NULL)
//		snprintf(cmd, sizeof(cmd)+1, "%s", para);
//	else
//		snprintf(cmd, sizeof(cmd)+1, "%s %d", EXEC_PROGRAM_PATH, datatype);
//	printf("cmd:%s\n", cmd);
//
//	Pthread_create(&tid, NULL, &system_call, (void *)cmd);
//
//	return ;
//}

pid_t invoke_by_datatype_fork(int datatype, const char *para)
{
	char cmd[100] = {0};
	pid_t	pid;
	int res;

	if ((pid = fork()) > 0) {
		waitpid(-1,NULL,0);
		return(pid);		//parent process
	}
	if ((pid = fork()) > 0) {
		exit(0);			//child process
		return (pid);
	}
							//grandson process
	if(datatype >API_AMOUNT) {
		GDGL_DEBUG("datatype invalid\n");
		exit(1) ;
	}

	if(para!=NULL)
		snprintf(cmd, sizeof(cmd)+1, "%s %d %s", EXEC_PROGRAM_PATH, datatype, para);
	else
		snprintf(cmd, sizeof(cmd)+1, "%s %d", EXEC_PROGRAM_PATH, datatype);

	GDGL_DEBUG("systerm cmd:%s\n", cmd);

	sleep(3); //add by yanly150528
	res = system(cmd);
	if(res <0) {
		GDGL_DEBUG("system call failed,res is: %d\n", res);
	}
	exit(0);
	return 0;
}

void invoke_all_pthread()
{
	pthread_t	tid;
	Pthread_create(&tid, NULL, &system_call, (void *)NULL);
	return ;
}


