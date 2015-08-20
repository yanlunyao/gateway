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

int cgiMain()
{
	int res=0;
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;

	cJSON* json_all;
	char *tmp;
    char read_data[1024]={0};

    int operatortype;
    int duration;

    cgiHeaderContentType("application/json"); //MIME
    cgiFormResultType cgi_re;
	cgi_re = cgiFormInteger("operatortype", &operatortype, -1);
	cgi_re = cgiFormInteger("param1", &duration, -1);

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

	if(operatortype == GY_GET_SETTING_MSG) {
		cJSON_AddNumberToObject(json_all, "status", res);
		cJSON_AddNumberToObject(json_all, "operatortype", operatortype);
		tmp = cJSON_Print(json_all);
		fprintf(cgiOut,"%s\n", tmp);
		free(tmp);
	}
	else if(operatortype == GY_SET_WARNINGTIME_TYPE) {

		//写文件
		cJSON_ReplaceItemInObject(json_all, "warningduration", cJSON_CreateNumber(duration));
		tmp = cJSON_Print(json_all);
		write_lock_file_data(FEATURE_GDGL_SETTING_PATH, tmp);
		free(tmp);
		fprintf(cgiOut,"{\n	\"status\":	%d,\n	\"operatortype\":	%d\n}\n", res, operatortype);

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
		cJSON_AddNumberToObject(json_all, MSGTYPE_STRING, GL_MSGTYPE_VALUE);
		cJSON_AddNumberToObject(json_all, GL_MAINID, GL_MAINID_GATEWAY);
		cJSON_AddNumberToObject(json_all, GL_SUBID, 1);
	    tmp = cJSON_Print(json_all);
	    snprintf(send_cb_string, sizeof(send_cb_string), "%s", tmp);
	    free(tmp);
//		snprintf(send_cb_string, sizeof(send_cb_string),
//				"{\n	\"msgtype\":	%d,\n	\"mainid\":	%d,\n	\"subid\":	%d,\n	\"warningduration\":	%d\n}\n",
//				GL_MSGTYPE_VALUE, GL_MAINID_GATEWAY, 1, duration);
		send_cb_len = strlen(send_cb_string)+1;
		push_to_CBDaemon(send_cb_string, send_cb_len);

	}
	else {
		fprintf(cgiOut,"{\n	\"status\":	%d,\n	\"operatortype\":	%d\n}\n", res, operatortype);
	}
	cJSON_Delete(json_all);
	return 0;
}
