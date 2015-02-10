/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName: bcast_daemon.c
  Author: fengqiuchao      
  Version : V1.0         
  Date: 2014/01/20
  Description: Broadcast module
  History:
      <author>       <time>     <version >   <desc>
      fengqiuchao    2014/01/20     1.0     build this moudle 
      fengqiuchao    2014/11/18     1.1     add response info
***************************************************************************/

#include "unp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "lockFile.h"
#include "smartgateway.h"

static void echo_client(int sockfd, SA *pcliaddr, socklen_t clilen);

int main(int argc, char ** argv)
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(FEATURE_GDGL_BCAST_PORT);

    Bind(sockfd, (SA *)&servaddr, sizeof(servaddr));

	echo_client(sockfd, (SA *) &cliaddr, sizeof(cliaddr));

    return 0;
}

/***************************************************************************
  Function: bcast_read_id
  Description: read smartgateway ID from mac.conf
  Input:  
  Output: id_str, null terminated
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
static int bcast_read_id(char * id_str)
{
    int fd_id;
	int res;
	
    fd_id = open(FEATURE_GDGL_MAC_PATH, O_RDONLY, 0777);
	if (fd_id < 0) {
		printf("id file open err\n");
		return -1;
	}
	res = read_lock(fd_id, 0, SEEK_SET, 0);
	if (res == -1) {
		close(fd_id);
		printf("id file lock err\n");
		return -2;
	}
	res = read(fd_id, id_str, FEATURE_GDGL_ID_LEN);
	if (res != FEATURE_GDGL_ID_LEN) {
		close(fd_id);
		printf("id file read err\n");
		return -3;
	}
	close(fd_id); //also unlock
	id_str[FEATURE_GDGL_ID_LEN] = '\0';

	return 0;
}

/***************************************************************************
  Function: bcast_read_alias
  Description: read smartgateway alias from alias.conf
  Input:  
  Output: alias_str, null terminated
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
static int bcast_read_alias(char * alias_str)
{
    int fd_alias;
	int res;
	
    fd_alias = open(FEATURE_GDGL_ALIAS_PATH, O_RDONLY, 0777);
	if (fd_alias < 0) {
		printf("alias file open err\n");
		return -1;
	}
	res = read_lock(fd_alias, 0, SEEK_SET, 0);
	if (res == -1) {
		close(fd_alias);
		printf("alias file lock err\n");
		return -2;
	}
	res = read(fd_alias, alias_str, FEATURE_GDGL_ACCOUNT_MAX_LEN);
	if (res < 0) {
		close(fd_alias);
		printf("alias file read err\n");
		return -3;
	}
	close(fd_alias); //also unlock
	alias_str[res] = '\0';

	return 0;
}

static void echo_client(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	int			n;
	socklen_t	len;
	char		mesg[MAXLINE];
	const char * request = "Who is smart gateway?";
	const char * reply_ok = "I am smart gateway.";
	const char * reply_err = "Wrong request.";
	char gateway_id[FEATURE_GDGL_ID_LEN + 1]; //12 + terminating null
	char gateway_alias[FEATURE_GDGL_ACCOUNT_MAX_LEN + 1];
	int res;
	char *out;
	cJSON *response_ok_json;
	cJSON *response_err_json;
	char response_ok[128];
	char response_err[128];
/*
	// Read ID
    res = bcast_read_id(gateway_id);
	if (res != 0) {
		strncpy(gateway_id, "no id", sizeof(gateway_id));
	}
	
	// Read alias
	res = bcast_read_alias(gateway_alias);
	if (res != 0) {
		strncpy(gateway_alias, "no alias", sizeof(gateway_alias));
	}

	response_ok_json = cJSON_CreateObject();
	if (!response_ok_json) {
		printf("Create response_ok_json failed\n");
		return;
	}
	cJSON_AddStringToObject(response_ok_json, "reply", reply_ok);
	cJSON_AddStringToObject(response_ok_json, "id", gateway_id);
	cJSON_AddStringToObject(response_ok_json, "alias", gateway_alias);
	out = cJSON_PrintUnformatted(response_ok_json);
	cJSON_Delete(response_ok_json);
	if (!out) {
		printf("cJSON_PrintUnformatted response_ok_json failed\n");
		return;
	}
	snprintf(response_ok, sizeof(response_ok), "%s", out);	
	free(out);

	response_err_json = cJSON_CreateObject();
	if (!response_err_json) {
		printf("Create response_err_json failed\n");
		return;
	}
	cJSON_AddStringToObject(response_err_json, "reply", reply_err);
	out = cJSON_PrintUnformatted(response_err_json);
	cJSON_Delete(response_err_json);
	if (!out) {
		printf("cJSON_PrintUnformatted response_err_json failed\n");
		return;
	}
	snprintf(response_err, sizeof(response_err), "%s", out);	
	free(out);
*/
	for ( ; ; ) {
		len = clilen;
		n = Recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);

		// Read ID
        res = bcast_read_id(gateway_id);
	    if (res != 0) {
		    strncpy(gateway_id, "no id", sizeof(gateway_id));
	    }
	
	    // Read alias
	    res = bcast_read_alias(gateway_alias);
	    if (res != 0) {
		    strncpy(gateway_alias, "no alias", sizeof(gateway_alias));
	    }

	    response_ok_json = cJSON_CreateObject();
	    if (!response_ok_json) {
		    printf("Create response_ok_json failed\n");
		    return;
	    }
	    cJSON_AddStringToObject(response_ok_json, "reply", reply_ok);
	    cJSON_AddStringToObject(response_ok_json, "id", gateway_id);
	    cJSON_AddStringToObject(response_ok_json, "alias", gateway_alias);
	    out = cJSON_PrintUnformatted(response_ok_json);
	    cJSON_Delete(response_ok_json);
	    if (!out) {
		    printf("cJSON_PrintUnformatted response_ok_json failed\n");
		    return;
	    }
	    snprintf(response_ok, sizeof(response_ok), "%s", out);	
	    free(out);

	    response_err_json = cJSON_CreateObject();
	    if (!response_err_json) {
		    printf("Create response_err_json failed\n");
		    return;
	    }
	    cJSON_AddStringToObject(response_err_json, "reply", reply_err);
	    out = cJSON_PrintUnformatted(response_err_json);
	    cJSON_Delete(response_err_json);
	    if (!out) {
		    printf("cJSON_PrintUnformatted response_err_json failed\n");
		    return;
	    }
	    snprintf(response_err, sizeof(response_err), "%s", out);	
	    free(out);

		if ( n != strlen(request)) {
			Sendto(sockfd, response_err, strlen(response_err), 0, pcliaddr, len);
		}
		else if ( strncmp(mesg, request, strlen(request) ) == 0 ) {
			Sendto(sockfd, response_ok, strlen(response_ok), 0, pcliaddr, len);
		}
		else {
            Sendto(sockfd, response_err, strlen(response_err), 0, pcliaddr, len);
		}
	}
}

