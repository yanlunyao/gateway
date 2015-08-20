/*
 *	File name   : apiComWithRFDaemon.c
 *  Created on  : Aug 3, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include "unp.h"
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"
#include "smartgateway.h"
#include "apiComWithRFDaemon.h"

#define	COM_RF_TIMEOUT					5
#define DEBUG_IP						"192.168.1.238"
#define COMM_WITH_RF_IP					LOCALHOST_TEST
//#define COMM_WITH_RF_IP					DEBUG_IP

/*
* Function: communicateWithRF
* Description: 不等待接收，发送完成即关闭连接。
* Input: send
* Output: none
* Return: 0>>push success;
*	  	  -1>>push error,socket send error
* Others:  none
*/
#if 1
int communicateWithRF(const char *send_text, int send_size, char *respond)
{
	int fd;
	struct sockaddr_in	servaddr;
//build socket
	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        RF_DEBUG("socket error\n");
		return SOCKET_BUILD_FAILED;
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(FEATURE_GDGL_API_RF_COM_PORT);
	if ( (inet_pton(AF_INET, COMM_WITH_RF_IP, &servaddr.sin_addr)) <= 0) {
		RF_DEBUG("inet_pton error\n");
		close(fd);
		return SOCKET_BUILD_FAILED;
	}
	if ( connect(fd, (SA *) &servaddr, sizeof(servaddr)) < 0 ) {
		RF_DEBUG("connect error\n");
		close(fd);
		return SOCKET_BUILD_FAILED;
	}
//send msg
    if ( writen(fd, send_text, send_size) != send_size ) {
    	RF_DEBUG("write error\n");
		close(fd);
		return WRITE_FAILED;
    }
	close(fd);
	return 0;
}
#else
int communicateWithRF(const char *send_text, int send_size, char *respond)
{
	int fd;
	int nfd;
	int nread;
	struct sockaddr_in	servaddr;
//build socket
	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        RF_DEBUG("socket error\n");
		return SOCKET_BUILD_FAILED;
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(FEATURE_GDGL_API_RF_COM_PORT);
	if ( (inet_pton(AF_INET, LOCALHOST_TEST, &servaddr.sin_addr)) <= 0) {
		RF_DEBUG("inet_pton error\n");
		close(fd);
		return SOCKET_BUILD_FAILED;
	}
	if ( connect(fd, (SA *) &servaddr, sizeof(servaddr)) < 0 ) {
		RF_DEBUG("connect error\n");
		close(fd);
		return SOCKET_BUILD_FAILED;
	}
//send msg
    if ( writen(fd, send_text, send_size) != send_size ) {
    	RF_DEBUG("write error\n");
		close(fd);
		return WRITE_FAILED;
    }
    if ( (nfd = readable_timeo(fd, COM_RF_TIMEOUT)) < 0) {
    	RF_DEBUG("readable_timeo error\n");
		close(fd);
		return SELECT_FAILED;
	}
	else if (nfd == 0) { //timeout, close connection
		RF_DEBUG("readable_timeo timeout\n");
		close(fd);
		return WAIT_RECV_TIMEOUT;
	}
//read msg
	while (1) {
	    if ( (nread = read(fd, respond, RF_RESPOND_TO_API_MAX_LEN-1)) < 0) {
			if (errno == EINTR)
				continue;
			else {
				RF_DEBUG("read error\n");
		        close(fd);
		        return READ_FAILED;
			}
	    }
		else if (nread == 0) {
			RF_DEBUG("closed by other end\n");
		    close(fd);
		    return READ_FAILED;
		}
		else
			break;
	}
	respond[nread] = 0; // add the terminated null
	RF_DEBUG("receive:\n%s\n", respond);
	close(fd);
//parse
	cJSON *root;
	char *out;
	root = cJSON_Parse(respond);
	if(root==NULL) {
		RF_DEBUG("read data parse error\n");
		return DATA_FORMAT_INVALID;
	}
	cJSON_AddNumberToObject(root, "respond_status", 0);
	out = cJSON_Print(root);
	snprintf(respond, RF_RESPOND_TO_API_MAX_LEN, "%s", out);
	free(out);
	cJSON_Delete(root);
	return 0;
}
#endif
