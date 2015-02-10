/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName: init_smartgateway.c
  Author: fengqiuchao      
  Version : V1.0         
  Date: 2014/02/12
  Description: Init module
                    get MAC
                    init mac.conf, alias.conf & password.conf if these files are not present
  History:
      <author>       <time>     <version >   <desc>
      fengqiuchao    2014/02/12     1.0     build this moudle  
***************************************************************************/

#include "unp.h"
#include <net/if.h>
#include "lockFile.h"
#include "smartgateway.h"

/***************************************************************************
  Function: getmac
  Description: get mac address
  Input: ifname 
  Output: mac
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int getmac(const char *ifname, unsigned char *mac)
{
    int sockfd;
	struct ifreq ifr;

	if (ifname == NULL || mac == NULL)
		return -1;
		
    sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, ifname);
	Ioctl(sockfd, SIOCGIFHWADDR, &ifr);
	memcpy(mac, ifr.ifr_hwaddr.sa_data, FEATURE_GDGL_MAC_LEN);

	close(sockfd);

	return 0;
}

/***************************************************************************
  Function: main
  Description: init smartgateway, 
                    get MAC,
                    init mac.conf, alias.conf & password.conf if these files are not present
  Input:  
  Output: 
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int main(int argc, char **argv)
{
    int ret, fd;
	char ifname[IF_NAMESIZE];
	unsigned char mac[FEATURE_GDGL_MAC_LEN] = {0};
	unsigned char macstr[2*FEATURE_GDGL_MAC_LEN] = {0};
	mode_t mask;
	int i;

	if (argc == 1) {
		strcpy(ifname, FEATURE_GDGL_IFNAME);
	}
	else {
		strcpy(ifname, argv[1]);
	}

	//get MAC
    ret = getmac(ifname, mac);
	if (ret < 0) {
		printf("init_smartgateway getmac error\n");
		return -1;
	}
	//printf("%s mac addr is [%02X:%02X:%02X:%02X:%02X:%02X]\n",
			//ifname, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	for (i = 0; i < FEATURE_GDGL_MAC_LEN; i++) {
		macstr[2*i] = (mac[i] & 0xF0) >> 4;
		macstr[2*i + 1] = mac[i] & 0x0F;
	}
	//ASCII to Char
	for (i = 0; i < 2*FEATURE_GDGL_MAC_LEN; i++) {
		if (macstr[i] <= 9)
			macstr[i] += 0x30; //0-9
		else
			macstr[i] += 0x37; //A-F
	}

	mask = umask(0);
	//printf("mask %o\n", mask);

	//init mac.conf, alias.conf & password.conf if these files are not present
	ret = access(FEATURE_GDGL_MAC_PATH, F_OK);
	if (ret != 0) {
		printf("init_smartgateway mac.conf not exit\n");
		//creat mac.conf
		fd = open(FEATURE_GDGL_MAC_PATH, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd < 0) {
			printf("init_smartgateway creat mac.conf error\n");
			return -2;
		}
		//lock file
		ret = write_lock(fd, 0, SEEK_SET, 0);
	    if (ret == -1) {
			printf("init_smartgateway lock mac.conf error\n");
		    return -2;
	    }
		//write MAC in mac.conf
		ret = pwrite(fd, macstr, 2*FEATURE_GDGL_MAC_LEN, 0);
		if (ret != 2*FEATURE_GDGL_MAC_LEN) {
			printf("init_smartgateway write mac.conf error\n");
			return -2;
		}
		//close mac.conf, also unlock
		if (close(fd) < 0) {
			printf("init_smartgateway close mac.conf error\n");
			return -2;
		}
	}
	
	ret = access(FEATURE_GDGL_ALIAS_PATH, F_OK);
	if (ret != 0) {
		printf("init_smartgateway alias.conf not exit\n");
		//creat alias.conf
		fd = open(FEATURE_GDGL_ALIAS_PATH, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd < 0) {
			printf("init_smartgateway creat alias.conf error\n");
			return -3;
		}
		//lock file
		ret = write_lock(fd, 0, SEEK_SET, 0);
	    if (ret == -1) {
			printf("init_smartgateway lock alias.conf error\n");
		    return -3;
	    }
		//write MAC in alias.conf
		ret = pwrite(fd, macstr, 2*FEATURE_GDGL_MAC_LEN, 0);
		if (ret != 2*FEATURE_GDGL_MAC_LEN) {
			printf("init_smartgateway write alias.conf error\n");
			return -3;
		}
		//close alias.conf, also unlock
		if (close(fd) < 0) {
			printf("init_smartgateway close alias.conf error\n");
			return -3;
		}
	}
	
	ret = access(FEATURE_GDGL_PASSWD_PATH, F_OK);
	if (ret != 0) {
		printf("init_smartgateway password.conf not exit\n");
		//creat password.conf
		fd = open(FEATURE_GDGL_PASSWD_PATH, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd < 0) {
			printf("init_smartgateway creat password.conf error\n");
			return -4;
		}
		//lock file
		ret = write_lock(fd, 0, SEEK_SET, 0);
	    if (ret == -1) {
			printf("init_smartgateway lock password.conf error\n");
		    return -4;
	    }
		//write the last 6 chars of MAC in password.conf
		ret = pwrite(fd, macstr + FEATURE_GDGL_MAC_LEN, FEATURE_GDGL_MAC_LEN, 0);
		if (ret != FEATURE_GDGL_MAC_LEN) {
			printf("init_smartgateway write password.conf error\n");
			return -4;
		}
		//close password.conf, also unlock
		if (close(fd) < 0) {
			printf("init_smartgateway close password.conf error\n");
			return -4;
		}
	}

	umask(mask);

	return 0;
}

