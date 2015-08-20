/*
 *	File name   : httpSocketRaw.c
 *  Created on  : Apr 22, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

/*
 * return: -1~failed, 0~success
 * */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define __USE_XOPEN
#include <time.h>

#include "sqliteOperator.h"

//通过原生socket发送http-get协议
static int http_get_method_by_socket(const char *urlstring)
{
    struct sockaddr_in remote_addr;
    int socket_fd;
    int res;

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    remote_addr.sin_port = htons(80);

    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        close(socket_fd);
        GDGL_DEBUG("socket creat failed\n");
        res = -1;
        return res;
    }
    if (connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) < 0)
    {
        close(socket_fd);
        GDGL_DEBUG("socket connect 80 failed\n");
        res = -1;
        return res;
    }
    send(socket_fd, urlstring, strlen(urlstring), 0);
    close(socket_fd);
    return 0;
}
//多进程执行
pid_t execute_url_action(int table_flag, int id_value)
{
	char sql[SQL_STRING_MAX_LEN];
	int res=0;
	char urlstring[URL_STRING_LEN+1];
	char url[URL_STRING_LEN+1];

	pid_t	pid;
	if ((pid = fork()) > 0) {
		waitpid(-1,NULL,0);
		return(pid);				//parent process
	}
	if ((pid = fork()) > 0) {
		exit(0);					//child process
		return (pid);
	}
									//grandson process
	switch (table_flag)
	{
		case TIME_ACTION_TABLE_FLAG:
			sprintf(sql, "SELECT urlstring FROM t_time_action WHERE tid=%d", id_value);
			break;

		case LINkAGE_TABLE_FLAG:
			sprintf(sql, "SELECT urlstring FROM t_linkage WHERE lid=%d", id_value);
			break;
		default :
			GDGL_DEBUG("table flag error\n");
			exit(1);
			break;
	}

//	res = db_init();
//	if(res <0) {
//		GDGL_DEBUG("db init failed\n");
//		exit(0);
//	}

	res = t_getact_per(sql, urlstring);
//	GDGL_DEBUG("res=%d, sql: %s\n", res, sql);
	if(res ==0) {
		snprintf(url, URL_STRING_LEN+1, "GET %s HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", urlstring);
		GDGL_DEBUG("url is: %s\n", url);
		res = http_get_method_by_socket(url);
	}
	exit(0);
	return 0;
}
//多进程执行截图
pid_t execute_ipccapture_url(int table_flag, int id_value, char *time_para)
{
	char sql[SQL_STRING_MAX_LEN];
	int res=0;
	char urlstring[URL_STRING_LEN+1];
	char url[URL_STRING_LEN+1];

	pid_t	pid;
	if ((pid = fork()) > 0) {
		waitpid(-1,NULL,0);
		return(pid);				//parent process
	}
	if ((pid = fork()) > 0) {
		exit(0);					//child process
		return (pid);
	}
									//grandson process
	switch (table_flag)
	{
		case TIME_ACTION_TABLE_FLAG:
			sprintf(sql, "SELECT urlstring FROM t_time_action WHERE tid=%d", id_value);
			break;

		case LINkAGE_TABLE_FLAG:
			sprintf(sql, "SELECT urlstring FROM t_linkage WHERE lid=%d", id_value);
			break;
		default :
			GDGL_DEBUG("table flag error\n");
			exit(1);
			break;
	}
	res = t_getact_per(sql, urlstring);
	if(res ==0) {
		int newtime=0;
		struct tm tmp_tm_time;
		if(time_para ==NULL) {
			//newtime = time(NULL);
			GDGL_DEBUG("warningtime=null\n");
		}
		else {
			strptime(time_para, "%Y-%m-%d %H:%M:%S", &tmp_tm_time);
			newtime = mktime(&tmp_tm_time);
		}
//		char newtime[50]={0};
//		int i,j=0;
//		for(i=0; i<strlen(time)+1; i++)
//		{
//			if(time[i] ==0x20) {
//				newtime[j] = '%';
//				j++;
//				newtime[j] = '2';
//				j++;
//				newtime[j] = '0';
//				j++;
//				continue;
//			}
//			if(time[i] =='\0')
//				break;
//			memcpy(newtime+j, time+i, 1);
//			j++;
//		}
		printf("newtime=%d\n",newtime);
		snprintf(url, URL_STRING_LEN+1, "GET %s&time=%d HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", urlstring, newtime);
		GDGL_DEBUG("url is: %s\n", url);
		res = http_get_method_by_socket(url);
	}
	exit(0);
	return 0;
}

