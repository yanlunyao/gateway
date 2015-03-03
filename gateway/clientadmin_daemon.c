/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName: clientadmin_daemon.c
  Author: fengqiuchao      
  Version : V1.0         
  Date: 2014/04/17
  Description: client admin daemon
  History:
      <author>       <time>     <version >   <desc>
      fengqiuchao    2014/04/17     1.0     build this moudle  
***************************************************************************/

#include "unpthread.h"
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "lockFile.h"
#include <sys/msg.h>
#include "smartgateway.h"

typedef enum {
    hb_first = 1,
    hb_pw = hb_first,
	hb_pw_alias,
	hb_last = hb_pw_alias
} heartbeatType;

static int ca_daemon_read_alias(char * alias_str);
static int ca_daemon_read_password(char * passwd_str);
static int process_msg(const struct client_admin_msgbuf *pmsg);
static void * thread_main(void *arg);
static void thread_doit(int sockfd, int id);
static int thread_response(const int id, const int hb_num, const int status_num, const char * status_msg_str, char * response_str);
static ssize_t	mywriten(int fd, const void *vptr, size_t n);

#define THREAD_NUM 20
#define	MAX_RESPONSE	65536
#define THREAD_TIMEOUT 90 //second
#define CA_ERROR_HEARTBEAT_VALUE 0
#define CA_HEARTBEAT_RESPONSE_SAME 0
#define CA_HEARTBEAT_RESPONSE_PASSWD_CHANGED 1
#define CA_HEARTBEAT_RESPONSE_ALIAS_CHANGED 2
#define CA_HEARTBEAT_RESPONSE_BOTH_CHANGED 3
#define CA_HEARTBEAT_RESPONSE_JSON_PARSE_FAILED 4
#define CA_HEARTBEAT_RESPONSE_HEARTBEAT_NOT_EXIST 5
#define CA_HEARTBEAT_RESPONSE_HEARTBEAT_NOT_NUMBER 6
#define CA_HEARTBEAT_RESPONSE_HEARTBEAT_VALUE_WRONG 7
#define CA_HEARTBEAT_RESPONSE_PASSWD_NOT_EXIST 8
#define CA_HEARTBEAT_RESPONSE_PASSWD_NOT_STRING 9
#define CA_HEARTBEAT_RESPONSE_ALIAS_NOT_EXIST 10
#define CA_HEARTBEAT_RESPONSE_ALIAS_NOT_STRING 11
#define CA_HEARTBEAT_RESPONSE_REQ_HANDLER_INVALID 12

static int listenfd;
static socklen_t addrlen;
static pthread_mutex_t	mlock = PTHREAD_MUTEX_INITIALIZER;

static char password_latest[FEATURE_GDGL_PASSWD_MAX_LEN+1] = {0};
static pthread_mutex_t	pwlock = PTHREAD_MUTEX_INITIALIZER;

static char alias_latest[FEATURE_GDGL_ACCOUNT_MAX_LEN+1] = {0};
static pthread_mutex_t	alock = PTHREAD_MUTEX_INITIALIZER;


/***************************************************************************
  Function: main
  Description: 
  Input:  
  Output: 
  Return: 
  Others:  none
***************************************************************************/
int main(int argc, char **argv)
{
	int		i, res;
	int mqid;
	struct client_admin_msgbuf mesg;
	ssize_t msg_len;
	pthread_t tid[THREAD_NUM];

	// Read init password
	res = ca_daemon_read_password(password_latest);
	if (res != 0) {
		GDGL_PRINTF("read init password error %d\n", res);
		return res;
	}
	// Read init alias
	res = ca_daemon_read_alias(alias_latest);
	if (res != 0) {
		GDGL_PRINTF("read init alias error %d\n", res);
		return res;
	}

	mqid = msgget(CLIENTADMIN_MQ_KEY, SVMSG_MODE | IPC_CREAT);
	if (mqid == -1) {
		GDGL_PRINTF("msgget error %d:%s\n", mqid, strerror(errno));
		return -1;
	}

	listenfd = Tcp_listen(NULL, FEATURE_GDGL_CPROXY_CA_LISTEN_PORT_STR, &addrlen);
	
	for (i = 0; i < THREAD_NUM; i++)
		Pthread_create(&tid[i], NULL, thread_main, (void *) i);

	for ( ; ; ) {
		// receive msg
		msg_len = msgrcv(mqid, &mesg, CLIENTADMIN_MSG_LEN, 0, 0);
		if (msg_len == -1) {
			GDGL_PRINTF("msgrcv error %d:%s\n", mqid, strerror(errno));
			return -2;
		}
		// process msg
		process_msg(&mesg);
	}
}

/***************************************************************************
  Function: ca_daemon_read_alias
  Description: read smartgateway alias from alias.conf
  Input:  
  Output: alias_str, null terminated
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
static int ca_daemon_read_alias(char * alias_str)
{
    int fd_alias;
	int res;
	
    fd_alias = open(FEATURE_GDGL_ALIAS_PATH, O_RDONLY, 0777);
	if (fd_alias < 0) {
		return -1;
	}
	res = read_lock(fd_alias, 0, SEEK_SET, 0);
	if (res == -1) {
		close(fd_alias);
		return -2;
	}
	res = read(fd_alias, alias_str, FEATURE_GDGL_ACCOUNT_MAX_LEN);
	if (res < 0) {
		close(fd_alias);
		return -3;
	}
	close(fd_alias); //also unlock
	alias_str[res] = '\0';

	return 0;
}

/***************************************************************************
  Function: ca_daemon_read_password
  Description: read smartgateway password from password.conf
  Input:  
  Output: passwd_str, null terminated
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
static int ca_daemon_read_password(char * passwd_str)
{
    int fd_passwd;
	int res;
	
    fd_passwd = open(FEATURE_GDGL_PASSWD_PATH, O_RDONLY, 0777);
	if (fd_passwd < 0) {
		return -1;
	}
	res = read_lock(fd_passwd, 0, SEEK_SET, 0);
	if (res == -1) {
		close(fd_passwd);
		return -2;
	}
	res = read(fd_passwd, passwd_str, FEATURE_GDGL_PASSWD_MAX_LEN);
	if (res < 0) {
		close(fd_passwd);
		return -3;
	}
	close(fd_passwd); //also unlock
	passwd_str[res] = '\0';

	return 0;
}

/***************************************************************************
  Function: process_msg
  Description: process message
  Input:  msg
  Output: none
  Return: 0:OK
              other:error
  Others:  none
***************************************************************************/
static int process_msg(const struct client_admin_msgbuf *pmsg)
{
    long msg_type;
	cJSON *msg_json, *pw_json, *alias_json;

	msg_json = cJSON_Parse(pmsg->mtext);
	if (!msg_json) {
		GDGL_PRINTF("msg_json parse Error\n");
		return -1;
	}
	
	msg_type = pmsg->mtype;
	switch (msg_type) {
		case clientAdmintMsgPassword: //password changed
		    pw_json = cJSON_GetObjectItem(msg_json, "password");
		    if (!pw_json) {
			    GDGL_DEBUG("password key not exist\n");
			    cJSON_Delete(msg_json);
			    return -2;
		    }
		    if (pw_json->type != cJSON_String) {
			    GDGL_DEBUG("password is not a string\n");
			    cJSON_Delete(msg_json);
			    return -2;
		    }
			Pthread_mutex_lock(&pwlock);
		    strncpy(password_latest, pw_json->valuestring, FEATURE_GDGL_PASSWD_MAX_LEN + 1);
			password_latest[FEATURE_GDGL_PASSWD_MAX_LEN] = 0;
		    Pthread_mutex_unlock(&pwlock);
			break;
		case clientAdmintMsgAlias: //alias changed
		    alias_json = cJSON_GetObjectItem(msg_json, "alias");
		    if (!alias_json) {
			    GDGL_DEBUG("alias key not exist\n");
			    cJSON_Delete(msg_json);
			    return -3;
		    }
		    if (alias_json->type != cJSON_String) {
			    GDGL_DEBUG("alias is not a string\n");
			    cJSON_Delete(msg_json);
			    return -3;
		    }
			Pthread_mutex_lock(&alock);
		    strncpy(alias_latest, alias_json->valuestring, FEATURE_GDGL_ACCOUNT_MAX_LEN + 1);
			alias_latest[FEATURE_GDGL_ACCOUNT_MAX_LEN] = 0;
		    Pthread_mutex_unlock(&alock);
			break;
		default:
			GDGL_PRINTF("msg_type Error: %ld\n", msg_type);
			cJSON_Delete(msg_json);
		    return -4;
	}

    cJSON_Delete(msg_json);
	return 0;		
}

/***************************************************************************
  Function: thread_main
  Description: thread main function, accept a client connection, if closed then reaccept
  Input:  arg, the index of thread
  Output: none
  Return: never return
  Others:  none
***************************************************************************/
static void * thread_main(void *arg)
{
	int				connfd;
	socklen_t		clilen;
	struct sockaddr	*cliaddr;

	cliaddr = Malloc(addrlen);

	GDGL_PRINTF("thread %d starting\n", (int) arg);
	for ( ; ; ) {
		clilen = addrlen;
    	Pthread_mutex_lock(&mlock);
		connfd = Accept(listenfd, cliaddr, &clilen);
		Pthread_mutex_unlock(&mlock);

		thread_doit(connfd, (int)arg);		/* process request */
		Close(connfd);
	}

	return NULL; //nerver run to here
}

/***************************************************************************
  Function: thread_doit
  Description: thread process the heartbeat
  Input:  sockfd
            id,  the index of thread
  Output: none
  Return: none
  Others:  none
***************************************************************************/
static void thread_doit(int sockfd, int id)
{
	int nfd;
	ssize_t		nread, nwrite;
	char		request[MAXLINE], response[MAX_RESPONSE] = {0};
	cJSON *request_json, *hb_json, *pw_json, *alias_json;
	int hb_num = 0;
	int rt, pw_changed, alias_changed;

    while (1) {
		if ( (nfd = readable_timeo(sockfd, THREAD_TIMEOUT)) < 0) {
			GDGL_DEBUG("thread %d readable_timeo error\n", id);
		    return;
		}
		else if (nfd == 0) { //timeout , return & close connection
			GDGL_DEBUG("thread %d readable_timeo timeout\n", id);
		    return;
		}
		
	    if ( (nread = read(sockfd, request, MAXLINE-1)) < 0 ) {
		    GDGL_DEBUG("thread %d read error\n", id);
		    return;
        }
	    else if ( nread == 0) {
	    	GDGL_DEBUG("thread %d connection closed by other end\n", id);
		    return;		/* connection closed by other end */
	    }
	    request[nread] = 0; // add null

        pw_changed = 0;
		alias_changed = 0;
		
		// parse request string
		request_json = cJSON_Parse(request);
		if (!request_json) {
			GDGL_DEBUG("thread %d request_json parse Error\n", id);
			nwrite = thread_response(id, CA_ERROR_HEARTBEAT_VALUE, CA_HEARTBEAT_RESPONSE_JSON_PARSE_FAILED, "json parse failed", response);
			goto end;
		}
	
		hb_json = cJSON_GetObjectItem(request_json, "heartbeat");
		if (!hb_json) {
			GDGL_DEBUG("thread %d heartbeat key not exist\n", id);
			cJSON_Delete(request_json);
			nwrite = thread_response(id, CA_ERROR_HEARTBEAT_VALUE, CA_HEARTBEAT_RESPONSE_HEARTBEAT_NOT_EXIST, "heartbeat key not exist", response);
			goto end;
		}
		if (hb_json->type != cJSON_Number) {
			GDGL_DEBUG("thread %d heartbeat is not a number\n", id);
			cJSON_Delete(request_json);
			nwrite = thread_response(id, CA_ERROR_HEARTBEAT_VALUE, CA_HEARTBEAT_RESPONSE_HEARTBEAT_NOT_NUMBER, "heartbeat is not a number", response);
			goto end;
		}
		hb_num = hb_json->valueint;
		if ( (hb_num < hb_first) || (hb_num > hb_last) ) {
			GDGL_DEBUG("thread %d heartbeat value wrong\n", id);
			cJSON_Delete(request_json);
			nwrite = thread_response(id, CA_ERROR_HEARTBEAT_VALUE, CA_HEARTBEAT_RESPONSE_HEARTBEAT_VALUE_WRONG, "heartbeat value wrong", response);
			goto end;
		}

		pw_json = cJSON_GetObjectItem(request_json, "password");
		if (!pw_json) {
			GDGL_DEBUG("thread %d password key not exist\n", id);
			cJSON_Delete(request_json);
			nwrite = thread_response(id, CA_ERROR_HEARTBEAT_VALUE, CA_HEARTBEAT_RESPONSE_PASSWD_NOT_EXIST, "password key not exist", response);
			goto end;
		}
		if (pw_json->type != cJSON_String) {
			GDGL_DEBUG("thread %d password is not a string\n", id);
			cJSON_Delete(request_json);
			nwrite = thread_response(id, CA_ERROR_HEARTBEAT_VALUE, CA_HEARTBEAT_RESPONSE_PASSWD_NOT_STRING, "password is not a string", response);
			goto end;
		}

		if (hb_num > hb_pw) { // also should pass alias
		    alias_json = cJSON_GetObjectItem(request_json, "alias");
		    if (!alias_json) {
			    GDGL_DEBUG("thread %d alias key not exist\n", id);
			    cJSON_Delete(request_json);
			    nwrite = thread_response(id, CA_ERROR_HEARTBEAT_VALUE, CA_HEARTBEAT_RESPONSE_ALIAS_NOT_EXIST, "alias key not exist", response);
			    goto end;
		    }
		    if (alias_json->type != cJSON_String) {
			    GDGL_DEBUG("thread %d alias is not a string\n", id);
			    cJSON_Delete(request_json);
			    nwrite = thread_response(id, CA_ERROR_HEARTBEAT_VALUE, CA_HEARTBEAT_RESPONSE_ALIAS_NOT_STRING, "alias is not a string", response);
			    goto end;
		    }
		}
			
		Pthread_mutex_lock(&pwlock);
		rt = strcmp(pw_json->valuestring, password_latest);
		Pthread_mutex_unlock(&pwlock);
		if ( rt != 0 ) {
			pw_changed = 1;
		}
		
		if (hb_num > hb_pw) {
		    Pthread_mutex_lock(&alock);
		    rt = strcmp(alias_json->valuestring, alias_latest);
		    Pthread_mutex_unlock(&alock);
		    if ( rt != 0 ) {
			    alias_changed = 1;
		    }
		}
		
		if ( (pw_changed == 1) && (alias_changed == 1) ) {
		    GDGL_DEBUG("thread %d both changed\n", id);
		    cJSON_Delete(request_json);
		    nwrite = thread_response(id, hb_num, CA_HEARTBEAT_RESPONSE_BOTH_CHANGED, "both changed", response);
			goto end;
		}
		else if (pw_changed == 1) {
			GDGL_DEBUG("thread %d password changed\n", id);
		    cJSON_Delete(request_json);
		    nwrite = thread_response(id, hb_num, CA_HEARTBEAT_RESPONSE_PASSWD_CHANGED, "password changed", response);
			goto end;
		}
		else if (alias_changed == 1) {
		    GDGL_DEBUG("thread %d alias changed\n", id);
		    cJSON_Delete(request_json);
			nwrite = thread_response(id, hb_num, CA_HEARTBEAT_RESPONSE_ALIAS_CHANGED, "alias changed", response);
			goto end;
		}
		else {
			GDGL_DEBUG("thread %d same\n", id);
		    cJSON_Delete(request_json);
			nwrite = thread_response(id, hb_num, CA_HEARTBEAT_RESPONSE_SAME, "same", response);
		}

        // response to client 
end:
		if (nwrite > 0) {
	        rt = mywriten(sockfd, response, nwrite);
		    if (rt != nwrite) {
			    GDGL_DEBUG("thread %d write error, %d/%d\n", id, rt, nwrite);
				return;
		    }
        }
		else { // some thread_response error ,return & close connection
			return;
		}
    }
}

/***************************************************************************
  Function: thread_response
  Description: thread construct response JSON string
  Input:  id
             hb_num
             status_num
             status_msg_str
  Output: response_str
  Return: negative:error
              positive:the length of response_str, including null
  Others:  none
***************************************************************************/
static int thread_response(const int id, const int hb_num, const int status_num, const char * status_msg_str, char * response_str)
{
    char *out;
	cJSON *response_json;
	int re;
	
	response_json = cJSON_CreateObject();
	if (!response_json) {
		GDGL_DEBUG("thread %d Create response_json failed\n", id);
		return -1;
	}
	
	cJSON_AddNumberToObject(response_json, "heartbeat", hb_num);
	cJSON_AddNumberToObject(response_json, "status", status_num);
	cJSON_AddStringToObject(response_json, "status_msg", status_msg_str);
	
	out = cJSON_PrintUnformatted(response_json);
	cJSON_Delete(response_json);
	if (!out) {
		GDGL_DEBUG("thread %d cJSON_PrintUnformatted response_json failed\n", id);
		return -2;
	}
	re = snprintf(response_str, MAX_RESPONSE, "%s", out);	
	free(out);

	return (re+1); //including null
}

/***************************************************************************
  Function: mywriten
  Description: writer n bytes wrap function
  Input:  fd
             vptr
             n
  Output: none
  Return: -1:error
              n
  Others:  none
***************************************************************************/
static ssize_t	mywriten(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}


