/*
 *	File name   : gatewaySettingOperation.c
 *  Created on  : Aug 18, 2015
 *  Author      : yanly
 *  Description : 网关配置，参数operatortype, param1, param2
 *
 *
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"
#include "cgic.h"
#include "smartgateway.h"
#include "myfile_operation.h"
#include "glCalkProtocol.h"
#include "httpCurlMethod.h"

#define GY_GET_SETTING_MSG			0
#define GY_SET_WARNINGTIME_TYPE		1
#define GY_SET_XMPP					2

#define PARAM1_MAXLEN				30
#define PARAM2_MAXLEN				30


//gateway operatortype
typedef enum {
    get_setting_msg =0,
	set_warningduration,
	set_xmpp_server
}gateway_operatortype;

static void setting_respond(int res_stauts, int type);

int cgiMain()
{
	int res=0;
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;

	cJSON* json_all;
	cJSON* callback_root;
	char *tmp;
    char read_data[1024]={0};

    int requsttype =-1;
    char param1[PARAM1_MAXLEN]={0}, param2[PARAM2_MAXLEN]={0};

    int duration= -1;
    gateway_operatortype operatortype;
    char xmpphost[PARAM1_MAXLEN]={0};
    int xmppport=-1;

    cgiHeaderContentType("application/json"); //MIME
    cgiFormResultType cgi_re;
	cgi_re = cgiFormInteger("operatortype", &requsttype, -1);
	operatortype = requsttype;
	cgi_re = cgiFormString("param1", param1, sizeof(param1));
	cgi_re = cgiFormString("param2", param2, sizeof(param2));

	//根据type类型获取param1信息
	switch(operatortype) {
		case get_setting_msg:
		break;
		case set_warningduration:
			duration = atoi(param1);
		break;
		case set_xmpp_server:
			memcpy(xmpphost, param1, strlen(param1));
			xmpphost[strlen(param1)] = '\0';
			xmppport = atoi(param2);
		break;
		default:
		break;
	}

	//读网关配置文件
	res = read_lock_file_data(FEATURE_GDGL_SETTING_PATH, read_data);
	if(res == 0) {
		json_all = cJSON_Parse(read_data);
		if(!json_all)
		{
			res = FILE_DATA_NOT_JSON;
			fprintf(cgiOut,"{\n	\"status\":	%d,\n	\"operatortype\":	%d\n}\n", res, operatortype);
			return 0;
		}
	}
	else {
		fprintf(cgiOut,"{\n	\"status\":	%d,\n	\"operatortype\":	%d\n}\n", res, operatortype);
		return 0;
	}

	//根据type更新网关配置文件和发送更新消息
	switch(operatortype) {
		case get_setting_msg:
			cJSON_AddNumberToObject(json_all, "status", res);
			cJSON_AddNumberToObject(json_all, "operatortype", operatortype);
			tmp = cJSON_Print(json_all);
			fprintf(cgiOut,"%s\n", tmp);
			free(tmp);
		break;
		case set_warningduration:
			//写文件
			cJSON_ReplaceItemInObject(json_all, "warningduration", cJSON_CreateNumber(duration));
			tmp = cJSON_Print(json_all);
			write_lock_file_data(FEATURE_GDGL_SETTING_PATH, tmp);
			free(tmp);
			setting_respond(res, operatortype);
	//		fprintf(cgiOut,"{\n	\"status\":	%d,\n	\"operatortype\":	%d\n}\n", res, operatortype);

			//调用zigbee报警器设置时长
			char setting_zigbee_warning[1024];
			char result_str[60000];
			snprintf(setting_zigbee_warning, sizeof(setting_zigbee_warning),
					"http://127.0.0.1/cgi-bin/rest/network/AllIasWarningDeviceOperation.cgi?param1=%d&param2=0&param3=0&operatortype=8",
					duration);
			curl_global_init(CURL_GLOBAL_ALL);
			http_curl_get(setting_zigbee_warning, result_str);
		    curl_global_cleanup();

			//callback
			callback_root = cJSON_CreateObject();
			cJSON_AddNumberToObject(callback_root, MSGTYPE_STRING, GL_MSGTYPE_VALUE);
			cJSON_AddNumberToObject(callback_root, GL_MAINID, GL_MAINID_GATEWAY);
			cJSON_AddNumberToObject(callback_root, GL_SUBID, 1);
			cJSON_AddNumberToObject(callback_root, "warningduration", duration);
		    tmp = cJSON_Print(callback_root);
		    snprintf(send_cb_string, sizeof(send_cb_string), "%s", tmp);
		    free(tmp);
		    cJSON_Delete(callback_root);
			send_cb_len = strlen(send_cb_string)+1;
			push_to_CBDaemon(send_cb_string, send_cb_len);
		break;

		case set_xmpp_server:
			//写文件
			cJSON_ReplaceItemInObject(json_all, "xmpphost", cJSON_CreateString(xmpphost));
			cJSON_ReplaceItemInObject(json_all, "xmppport", cJSON_CreateNumber(xmppport));
			tmp = cJSON_Print(json_all);
			write_lock_file_data(FEATURE_GDGL_SETTING_PATH, tmp);
			free(tmp);
			setting_respond(res, operatortype);

			//kill通道模块gdgl_channel
			system("killall gdgl_channel");

			//callback
			callback_root = cJSON_CreateObject();
			cJSON_AddNumberToObject(callback_root, MSGTYPE_STRING, GL_MSGTYPE_VALUE);
			cJSON_AddNumberToObject(callback_root, GL_MAINID, GL_MAINID_GATEWAY);
			cJSON_AddNumberToObject(callback_root, GL_SUBID, 2);
			cJSON_AddStringToObject(callback_root, "xmpphost", xmpphost);
			cJSON_AddNumberToObject(callback_root, "xmppport", xmppport);
		    tmp = cJSON_Print(callback_root);
		    snprintf(send_cb_string, sizeof(send_cb_string), "%s", tmp);
		    free(tmp);
		    cJSON_Delete(callback_root);
			send_cb_len = strlen(send_cb_string)+1;
			push_to_CBDaemon(send_cb_string, send_cb_len);
		break;
		default:
			setting_respond(res, operatortype);
		break;
	}
	cJSON_Delete(json_all);
	return 0;
}

static void setting_respond(int res_stauts, int type)
{
	fprintf(cgiOut,"{\n	\"status\":	%d,\n	\"operatortype\":	%d\n}\n", res_stauts, type);
}
