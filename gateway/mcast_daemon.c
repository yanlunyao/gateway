/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName: mcast_daemon.c
  Author: fengqiuchao      
  Version : V1.0         
  Date: 2014/01/20
  Description: Multicast module
  History:
      <author>       <time>     <version >   <desc>
      fengqiuchao    2014/01/20     1.0     build this moudle  
***************************************************************************/

#include "unp.h"
#include "smartgateway.h"

static void echo_client(int sockfd, SA *pcliaddr, socklen_t clilen);

int main(int argc, char ** argv)
{
    int sockfd;
    struct sockaddr_in servaddr, cliaddr, grpaddr;

    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(FEATURE_GDGL_MCAST_PORT);

    Bind(sockfd, (SA *)&servaddr, sizeof(servaddr));

	bzero(&grpaddr, sizeof(grpaddr));
	grpaddr.sin_family      = AF_INET;
	grpaddr.sin_addr.s_addr = inet_addr("224.0.0.1");

	mcast_join(sockfd, (const SA *)&grpaddr, sizeof(grpaddr), NULL, 0);

	echo_client(sockfd, (SA *) &cliaddr, sizeof(cliaddr));

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

	for ( ; ; ) {
		len = clilen;
		n = Recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);

		//mesg[n] = '\0';
		//printf("%s\n", mesg);
		//printf("receive %d chars\n", n);

		if ( n != strlen(request)) {
			Sendto(sockfd, reply_err, strlen(reply_err), 0, pcliaddr, len);
		}
		else if ( strncmp(mesg, request, strlen(request) ) == 0 ) {
			Sendto(sockfd, reply_ok, strlen(reply_ok), 0, pcliaddr, len);
		}
		else {
            Sendto(sockfd, reply_err, strlen(reply_err), 0, pcliaddr, len);
		}
	}
}


