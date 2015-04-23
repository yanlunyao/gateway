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
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "smartgateway.h"

int http_get_method_by_socket(const char *urlstring)
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
