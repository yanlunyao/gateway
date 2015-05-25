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


#include "list.h"
#define SMARTGATEWAY_DEBUG
#include <stdio.h>	
#ifdef SMARTGATEWAY_DEBUG
#define GDGL_DEBUG(fmt, args...)	printf("%s(%d)[%s]: " fmt, __FILE__, __LINE__, __func__, ## args)
#else
#define GDGL_DEBUG(fmt, args...)
#endif
#define GDGL_PRINTF(fmt, args...)	printf("%s(%d)[%s]: " fmt, __FILE__, __LINE__, __func__, ## args)

//#define URL_API_DEBUG
#ifdef URL_API_DEBUG
#define URLAPI_DEBUG(fmt, args...)	printf("%s(%d)[%s]: " fmt, __FILE__, __LINE__, __func__, ## args)
#else
#define URLAPI_DEBUG(fmt, args...)	fprintf( fopen("/gl/etc/database/api.log","w"), "%s(%d)[%s]: " fmt, __FILE__, __LINE__, __func__, ## args)
#endif

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
#define FEATURE_GDGL_CB_DAEMON_CALLBACK_PORT	5018
#define FEATURE_GDGL_CB_DAEMON_CALLBACK_PORT_STR	"5018"
#define FEATURE_GDGL_CPROXY_SIFT_API_PUSH_PORT 5019  //test debug
#define FEATURE_GDGL_CPROXY_SIFT_API_PUSH_PORT_STR "5019" //test debug

//yan test
#define LOCALHOST_TEST		"127.0.0.1"//"127.0.0.1"//"192.168.1.196"

// Init
#define FEATURE_GDGL_MAC_LEN 6
#define FEATURE_GDGL_IFNAME "eth0"
#define FEATURE_GDGL_MAC_PATH "/gl/etc/mac.conf"
#define FEATURE_GDGL_ALIAS_PATH "/gl/etc/alias.conf"
#define FEATURE_GDGL_PASSWD_PATH "/gl/etc/password.conf"

#define FEATURE_GDGL_TIME_SCENE_LINKAGE_DB		"/gl/etc/database/application.db"
#define TIME_ACTION_TABLE_NAME		"t_time_action"
#define LINkAGE_TABLE_NAME			"t_linkage"
#define SCENE_TABLE_NAME			"t_scene"
#define	SCENE_ACTION_TABLE_NAME		"t_scene_act"
#define	TABLE1						1
#define TABLE2						2
#define TABLE3						3
#define TABLE4						4
#define	TIME_ACTION_TABLE_FLAG		TABLE1
#define	SCENE_TABLE_FLAG			TABLE2
#define	SCENE_ACTION_TABLE_FLAG		TABLE3
#define	LINkAGE_TABLE_FLAG			TABLE4

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

/***********************api&&callback&&database data struct**************************/ //add by yanly
//所有规则的共性值
#define ENABLE						1
#define DISABLE						0
//callback or api error respond
#define ERROR_OPENED_DB					-1
#define ERROR_ACCESS_DB					-2			//一般可能是sql语句错误
#define ERROR_READ_DB					-3
#define ERROR_WRITE_DB					-4
#define ERROR_MDFY_NO_ID_DB 			-5
#define ERROR_PARAMETER_INVALID			-6
//#define	ERROR_GENERATE_URL_STRING		-7  //组合urlstring失败，可能是因为malloc失败，或者参数不对。
#define	ERROR_GENERATE_URL_STRING		ERROR_PARAMETER_INVALID
#define ERROR_HTTP_INVOKE				-8	//
#define ERROR_OTHER						-7  //其它错误，比如分配内存失败的情况

#define NAME_STRING_LEN				60
#define URL_STRING_LEN				200
#define IEEE_LEN					16
#define OUT_TIME_FORMAT_LEN			14
#define LINKAGE_ACT_LEN				40
#define PARA3_LEN					7
#define ACTPARA_LEN					40
#define ATTRIBUTE_LEN				30
#define REL_OPERATOR_LEN			2
#define TRIGGER_CONDITION_LEN		ATTRIBUTE_LEN+10
#define DELAY_OR_REPEAT_TIME_FLAG_LEN	PARA3_LEN

//action type
#define DEV_BYPASS_ACT_TYPE				1
#define ALL_BYPASS_ACT_TYPE				2
#define MAIN_OUTLET_ACT_TYPE			3
#define IPC_CAPTURE_ACT_TYPE			4

//定时规则参数值
#define TIME_ACTION_DELAY_MODE		2
#define TIME_ACTION_TIMING_MODE		1
#define START_TIMEACTION			ENABLE
#define STOP_TIMEACTION				DISABLE
#define TIME_ACTION_REPEAT_ENABLE	ENABLE
#define	TIME_ACTION_REPEAT_DISABLE	DISABLE
#define TID_MAX			100

//场景规则参数值
#define SCENEACTION_MAX_LEN		1000

//联动规则参数值
//eq-等于；bt-大于；lt-小于；be-大于等于；le-小于等于
//视频操作标记
#define LNK_IPC_CAPTURE_RULE_FLAG		3

//COMMON
typedef struct{
	char urlstring[URL_STRING_LEN+1];
	char actobj[IEEE_LEN+1];
	char actiontype;
}url_act_st;

//scene
typedef struct{
	int sid;
	char scnname[NAME_STRING_LEN];
	int scnindex;
	char scnaction[SCENEACTION_MAX_LEN];
}scene_base_st, *scene_base_stPtr;
typedef struct{
	int list_total;
	scene_base_stPtr scene_base;
}scene_base_list_st;
typedef struct{
//	int sid;
	char acttotal;     							//总操作的个数
	char actobj[IEEE_LEN+1];						//操作对象
	char urlstring[URL_STRING_LEN+1];				//操作url串
	char actpara[ACTPARA_LEN+1];
}scene_action_st, *scene_action_stPtr;
typedef struct{
	int urltotal;     							//总操作的个数
	char urlstring[URL_STRING_LEN];				//操作url串
}url_string_st;
typedef struct{
	scene_base_st scene_base;
	scene_action_st *scene_action;
}scene_st;

//time action
typedef struct{
	int  tid;
	char actname[NAME_STRING_LEN];
	char actpara[ACTPARA_LEN];
	int actmode;
	char para1[OUT_TIME_FORMAT_LEN];
	int para2;
	char para3[PARA3_LEN];
	int enable;
}time_action_base_st;
typedef struct{
	time_action_base_st ta_base;
	url_act_st urlobject;
}time_action_st, *time_action_stPtr;
typedef struct{
	int tid;	//empty is -1.
	char mode;	//1~timing, 2~delay
	char excute_time[OUT_TIME_FORMAT_LEN+1];
	char repeat;	//1~repeat, 0~not repeat
	char process_time[DELAY_OR_REPEAT_TIME_FLAG_LEN+1]; //表示每周重复时间设置或延时时间
}time_list_st; //定时主循环模块用到的结构体
typedef struct{
	int list_total;
	time_action_base_st *time_action_base;
}time_action_alllist_st;
typedef struct{
	int  lid;
	char lnkname[NAME_STRING_LEN+1];
	char trgieee[IEEE_LEN+1];
	char trgep[2+1];
	char trgcnd[TRIGGER_CONDITION_LEN+1];
	char lnkact[ACTPARA_LEN+1];
	char enable;
}linkage_base_st;
typedef struct{
	char attribute[ATTRIBUTE_LEN]; //属性
	char operator[REL_OPERATOR_LEN];		//运算符
	int value;			//值
}linkage_trgcnd_st;
//linkage
typedef struct{
	linkage_base_st lnk_base;
	linkage_trgcnd_st lnk_condition;
	url_act_st urlobject;
}linkage_st;
typedef struct
{
	int lid;
	char trgieee[IEEE_LEN+1];
	char trgep[2+1];
	char attribute[ATTRIBUTE_LEN+1]; //属性
	char operator[REL_OPERATOR_LEN+1];		//运算符
	int value;			//值
	char effect_status;  //该条规则当前生效的状态。生效时置1，1s后状态恢复置0；
}linkage_loop_st;
typedef struct
{
	linkage_loop_st linkage_member;
	struct list_head list;
}list_linkage_st;

/************************************************************************************/

#endif //SMARTGATEWAY_H__
