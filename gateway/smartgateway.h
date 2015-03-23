/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  File name: smartgateway.h
  Description: All common define in smart gateway project
  Author: fengqiuchao
  Version: 1.0      
  Date: 2014/01/20
  History:   
    1. Date:
       Author:
       Modification:
    2. ...
***************************************************************************/

#ifndef SMARTGATEWAY_H__
#define SMARTGATEWAY_H__

#define SMARTGATEWAY_DEBUG
#include <stdio.h>	
#ifdef SMARTGATEWAY_DEBUG
#define GDGL_DEBUG(fmt, args...)	printf("%s(%d)[%s]: " fmt, __FILE__, __LINE__, __func__, ## args)
#else
#define GDGL_DEBUG(fmt, args...)
#endif
#define GDGL_PRINTF(fmt, args...)	printf("%s(%d)[%s]: " fmt, __FILE__, __LINE__, __func__, ## args)

// UDP PORT
#define FEATURE_GDGL_BCAST_PORT 5002
#define FEATURE_GDGL_MCAST_PORT 5001

// TCP PORT
#define FEATURE_GDGL_CPROXY_CA_PUSH_PORT 5013
#define FEATURE_GDGL_CPROXY_CA_PUSH_PORT_STR "5013"
#define FEATURE_GDGL_CPROXY_HTTPMODULE_PORT 5014
#define FEATURE_GDGL_CPROXY_HTTPMODULE_PORT_STR "5014"
#define FEATURE_GDGL_CPROXY_CA_LISTEN_PORT 5016
#define FEATURE_GDGL_CPROXY_CA_LISTEN_PORT_STR "5016"
#define FEATURE_GDGL_CPROXY_CB_PUSH_PORT 5088   //debug yan
#define FEATURE_GDGL_CPROXY_CB_PUSH_PORT_STR "5088"
#define FEATURE_GDGL_CPROXY_SIFT_PUSH_PORT 5021 //test debug
#define FEATURE_GDGL_CPROXY_SIFT_PUSH_PORT_STR "5021" //test debug
#define FEATURE_GDGL_CPROXY_CALLBACK_PORT 5019  //test debug
#define FEATURE_GDGL_CPROXY_CALLBACK_PORT_STR "5019" //test debug

//yan test
#define LOCALHOST_TEST		"127.0.0.1"//"127.0.0.1"//"192.168.1.196"

// Init
#define FEATURE_GDGL_MAC_LEN 6
#define FEATURE_GDGL_IFNAME "eth0"
#define FEATURE_GDGL_MAC_PATH "/gl/etc/mac.conf"
#define FEATURE_GDGL_ALIAS_PATH "/gl/etc/alias.conf"
#define FEATURE_GDGL_PASSWD_PATH "/gl/etc/password.conf"

// Client admin
#define FEATURE_GDGL_ACCOUNT_MAX_LEN 16
#define FEATURE_GDGL_ACCOUNT_MIN_LEN 6
#define FEATURE_GDGL_PASSWD_MAX_LEN 16
#define FEATURE_GDGL_PASSWD_MIN_LEN 6
#define FEATURE_GDGL_ID_LEN 12

#define CLIENTADMIN_MQ_KEY 1234L
#define MSG_R 0400
#define MSG_W 0200
#define	SVMSG_MODE	(MSG_R | MSG_W | MSG_R>>3 | MSG_R>>6)
#define CLIENTADMIN_MSG_LEN 256

//siftCallback_daemon
#define SIFTCALLBACK_MQ_KEY	1235L


typedef enum {
    clientAdmintMsgPassword = 1,
	clientAdmintMsgAlias
} clientAdminMsgType;

struct client_admin_msgbuf {
    long int mtype;		/* type of received/sent message */
    char mtext[CLIENTADMIN_MSG_LEN];		/* text of the message */
};

// error_id public
#define ERROR_ID_PUBLIC_JSON_PARSE_FAILED 1
#define ERROR_ID_PUBLIC_REQUEST_HANDLER_INVALID 2

#endif //SMARTGATEWAY_H__
