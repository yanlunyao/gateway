/*
 * siftCallback_daemon.c
 *
 *  Created on: Feb 12, 2015
 *      Author: yanly
 */

#include "unpthread.h"
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include <sys/msg.h>
#include "smartgateway.h"



#define DAEMON_NAME			"SIFT_CALLBACK"
#define	DAEMON_VERSION		"V01-00"
#define SOCKET_INIT_RETRY_CNT	15
#define LOCALHOST_TEST		"192.168.1.196"//"192.168.1.196"//"127.0.0.1"  //debug

pthread_t tid[2];


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

		//send heartbeat: timer30s

		//receive queue msg
		for ( ; ; ) {
			mlen = msgrcv(mqid, &mesg, CLIENTADMIN_MSG_LEN, 0, MSG_NOERROR);
			if (mlen == -1) {
				GDGL_PRINTF("msgrcv error %d:%s:%d\n", mqid, strerror(errno), errno);
				exit(1);
			}
			//gnerate msg
//			GDGL_DEBUG("recive msg\n");
			//send msg to sift push port
			s_status = mywriten(sockfd, &mesg.mtext[0], mlen);
		    if (s_status != mlen) {
			    GDGL_DEBUG("thread write error\n");
				break;
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
	}
	close(sockfd);
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
//sift callback
				mesg.mtype = 1;
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
	signal(SIGPIPE,SIG_IGN);
	Pthread_create(&tid[0], NULL, thread_sift_push, NULL);
	Pthread_create(&tid[1], NULL, thread_cb, NULL);

	Pthread_join(tid[0], NULL);
	Pthread_join(tid[1], NULL);
	return 0;
}

