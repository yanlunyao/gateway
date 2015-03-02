/*
 * siftCallback_daemon.c
 *
 *  Created on: Feb 12, 2015
 *      Author: yanly
 */

#include "timedaction.h"
#include "unpthread.h"
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include <sys/msg.h>
#include "smartgateway.h"



#define DAEMON_NAME			"SIFT_CALLBACK"
#define	DAEMON_VERSION		"V01-00"
#define SOCKET_INIT_RETRY_CNT	15
#define HEART_BEAT_TIME			30

#define THREAD_ID_CB		0
#define THERAD_ID_SIFT		1

//#define LOCALHOST_TEST		"192.168.1.196"//"192.168.1.196"//"127.0.0.1"  //debug

pthread_t tid[2];
timed_action_notifier* notifier;
int sift_fd_online;
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
/*
 * Function: send_heartbeat
 * description:
 * Input: socket fd
 * Output:
 * Return:
 * Other:
 * */
void send_heartbeat_cb(void* data)
{
	int status;
	int fd = *(int *)data;
	char hearbeat[] = "{\"msgtype\": 1,\"heartbeat\": \"0200070007\"}";
	status = mywriten(fd, hearbeat, strlen(hearbeat));
    if (status != strlen(hearbeat)) {
	    GDGL_DEBUG("thread write error\n");
	    close(fd);
		//break;
    }
}
/*
 * Function: send_heartbeat_sift
 * description:
 * Input: socket fd
 * Output:
 * Return:
 * Other:
 * */
void send_heartbeat_sift(void* data)
{
	int status;
	int fd = *(int *)data;
	char hearbeat[] = "{\"msgtype\": 1,\"heartbeat\": \"0200070007\"}";
	status = mywriten(fd, hearbeat, strlen(hearbeat));
    if (status != strlen(hearbeat)) {
	    GDGL_DEBUG("thread write error\n");
	    //close(fd);
	    sift_fd_online = 0;
		//break;
    }
}
/*
 * Function: init_socket_with_localhost
 * description: this init for CB_Daemon connect
 * Input:
 * Output:
 * Return: socket descriptor
 * Other:
 */
int init_socket_with_localhost(int port)
{
	int fd;
	struct sockaddr_in	servaddr;
	int init_retry_cnt = 0;

	while(1){

		if(init_retry_cnt > SOCKET_INIT_RETRY_CNT){
			GDGL_DEBUG("socket init retry exceed max conut to quit\n");
			exit(1);
		}

		if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
			GDGL_DEBUG("socket error\n");
			init_retry_cnt++;
			continue;
		}

		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(port);
		servaddr.sin_addr.s_addr = inet_addr(LOCALHOST_TEST);//htonl(INADDR_ANY);

		if ( connect(fd, (SA *) &servaddr, sizeof(servaddr)) < 0 ) {
			init_retry_cnt++;
			GDGL_DEBUG("connect error\n");
			close(fd);
			sleep(1);
			continue;
		}
		return fd;
	}
}
//int init_socketcb(int port)
//{
//	int fd;
//	struct sockaddr_in	servaddr;
//	int init_retry_cnt = 0;
//
//	while(1){
//
//		if(init_retry_cnt > SOCKET_INIT_RETRY_CNT){
//			GDGL_DEBUG("socket init retry exceed max conut to quit\n");
//			exit(1);
//		}
//
//		if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
//			GDGL_DEBUG("socket error\n");
//			init_retry_cnt++;
//			continue;
//		}
//
//		bzero(&servaddr, sizeof(servaddr));
//		servaddr.sin_family = AF_INET;
//		servaddr.sin_port = htons(port);
//		servaddr.sin_addr.s_addr = inet_addr("192.168.1.196");//htonl(INADDR_ANY);
//
//		if ( connect(fd, (SA *) &servaddr, sizeof(servaddr)) < 0 ) {
//			init_retry_cnt++;
//			GDGL_DEBUG("connect error\n");
//			close(fd);
//			sleep(1);
//			continue;
//		}
//		return fd;
//	}
//}

/*
 * Function: sift_callback_msg
 * description: parse and sift the cjson msg
 * Input:	original msg
 * Output:
 * Return:  true or false
 * Other:
 */
#define USEFUL_MSGTYPE_MAX	2
int useful_callback_msgtype[USEFUL_MSGTYPE_MAX] = {
		0x02,0x03
};
int sift_callback_msg(char *text, int text_len)
{
    int msgtype;
	cJSON *msg_json;
	int i;

	msg_json = cJSON_Parse(text);
	if (!msg_json) {
		GDGL_PRINTF("msg_json parse Error\n");
		return 0;
	}
	if(cJSON_GetObjectItem(msg_json, "msgtype") == NULL){
		GDGL_PRINTF("msgtype parse error!\n");
		cJSON_Delete(msg_json);
		return 0;
	}
	msgtype = cJSON_GetObjectItem(msg_json, "msgtype")->valueint;

	for(i=0; i<USEFUL_MSGTYPE_MAX; i++){
		if(msgtype == useful_callback_msgtype[i]){
			cJSON_Delete(msg_json);
			return 1;
		}
		i++;
	}
	cJSON_Delete(msg_json);
	return 0;
}
/*
 *thread_sift_push: connect sift push port, queue SIFTCALLBACK_MQ_KEY,
 *						&send queue msg to sift push port,
 *						&send heartbeat to sift push port,
 * */
static void * thread_sift_push(void *arg)
{
	int sockfd,s_status;
	int mqid,mlen;
	struct client_admin_msgbuf mesg;

	while(1){

//set up msg queue
		mqid = msgget(SIFTCALLBACK_MQ_KEY, SVMSG_MODE | IPC_CREAT);
		if (mqid == -1) {
			GDGL_PRINTF("msgget error %d:%s\n", mqid, strerror(errno));
			exit(1);
		}

//set up sift socket
		sockfd = init_socket_with_localhost(FEATURE_GDGL_CPROXY_SIFT_PUSH_PORT);
		sift_fd_online = 1;
//send heartbeat: timer30s
		timed_action_t *time_action_sift;
		time_action_sift = timed_action_schedule_periodic(notifier, HEART_BEAT_TIME, 0, &send_heartbeat_sift, &sockfd);
	    if(time_action_sift ==NULL){
	    	GDGL_DEBUG("timed_action_schedule_periodic error\n");
	    	exit(1);
	    }
//receive queue msg
		while(sift_fd_online ==1) {
			mlen = msgrcv(mqid, &mesg, CLIENTADMIN_MSG_LEN, 0, MSG_NOERROR|IPC_NOWAIT);
			if((mlen == -1)&&(errno!=ENOMSG)) {
				GDGL_PRINTF("msgrcv error %d:%s:%d\n", mqid, strerror(errno), errno);
				exit(1);
			}
//gnerate msg
			//GDGL_DEBUG("recive msg\n");
//send msg to sift push port
			if(mlen>0){
				s_status = mywriten(sockfd, &mesg.mtext[0], mlen);
			    if (s_status != mlen) {
				    GDGL_DEBUG("thread write error\n");
					break;
			    }
			}

//			s_status = send(sockfd, &mesg.mtext[0], mlen, 0);
//			if( s_status <0) {
//				GDGL_DEBUG("thread_sift_push send msg to sift push port error\n");
//				break;
//			}
//			else if(s_status ==0){
//				GDGL_DEBUG("send msg to sift push port error: other server close this socket\n");
//				break;
//			}
//			else{
//				//normal
//			}
		}
	    close(sockfd);
//stop period time task
	    timed_action_unschedule(notifier, time_action_sift);
	}
}
/*
 *thread_cb: connect to localaddr5018, sift callbck,
 * 				& send callback msg to thread_sift by ipc queue,
 *				&send heartbeat to 5018
 * */
static void * thread_cb(void *arg)
{
	int sockfd;
	int nread;
	char request[MAXLINE];
//	char response[MAXLINE] = {0};

	int mqid,mlen;
	int qmsg_res;
	struct client_admin_msgbuf mesg;

	while(1){

//set up msg queue
		mqid = msgget(SIFTCALLBACK_MQ_KEY, SVMSG_MODE | IPC_CREAT);
		if (mqid == -1) {
			GDGL_PRINTF("msgget error %d:%s\n", mqid, strerror(errno));
			exit(1);
		}

//set up cb socket
		sockfd = init_socket_with_localhost(FEATURE_GDGL_CPROXY_CALLBACK_PORT);
//send heartbeat: timer30s
		timed_action_t *time_action_cb;
		time_action_cb = timed_action_schedule_periodic(notifier, HEART_BEAT_TIME, 0, &send_heartbeat_cb, &sockfd);
	    if(time_action_cb ==NULL){
	    	GDGL_DEBUG("timed_action_schedule_periodic error\n");
	    	exit(1);
	    }

		while(sockfd>0){
//read
			if ( (nread = read(sockfd, request, MAXLINE-1)) < 0 ) {
				GDGL_DEBUG("thread_cb read error\n");
				break;
			}
			else if ( nread == 0) {
				GDGL_DEBUG("thread_cb connection closed by other end\n");
				break;		/* connection closed by other end */
			}

			else{
//read success
//sift callback msg
				mesg.mtype = 1;
				if(sift_callback_msg(request, nread) <=0){
					continue;
				}
				memcpy(&mesg.mtext[0], request, nread);
				mlen = nread;
//send ipc msg
				qmsg_res = msgsnd(mqid, &mesg, mlen, 0);
				if (qmsg_res == -1) {
					GDGL_DEBUG("msgsnd error %s\n", strerror(errno));
					exit(1);
				}
			}
		}
		close(sockfd);
//stop period time task
		timed_action_unschedule(notifier, time_action_cb);
	}
}

/*
 * Function: main
 * Input
 * Output
 * Return
 * Other
 * */
int main()
{
//multi time task main thread
	notifier = timed_action_mainloop_threaded();
    if(notifier == NULL)
    {
    	GDGL_DEBUG("timed_action_mainloop_threaded error \n");
    	exit(1);
    }
//ignore signal
	signal(SIGPIPE,SIG_IGN);
	Pthread_create(&tid[0], NULL, thread_sift_push, NULL);
	Pthread_create(&tid[1], NULL, thread_cb, NULL);

	Pthread_join(tid[0], NULL);
	Pthread_join(tid[1], NULL);
	return 0;
}

