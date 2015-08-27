/*
 *	File name   : httpGetMsgSmart.c
 *  Created on  : Aug 20, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "smartgateway.h"
#include "cJSON.h"
#include "httpCurlMethod.h"
#include "httpGetMsgSmart.h"


int http_get_warningduration(int *value)
{
	int rt;
	char		result_str[MAX_RESULT] = {0};
	char		response_json_str[MAX_RESULT] = {0};
	const char url_str[]= "http://127.0.0.1/cgi-bin/rest/network/GatewaySettingOperation.cgi?operatortype=0&param1=0";
	cJSON 		*response_json, *json_temp;

	rt = http_curl_get(url_str, result_str);
	if(rt <0) {
		GDGL_DEBUG("invoke api failed\n");
		return -1;
	}
	get_json_str(response_json_str, result_str);
	response_json = cJSON_Parse(response_json_str);

	if (!response_json)
	{
		GDGL_DEBUG("json parse Error before: (%s)\n", cJSON_GetErrorPtr());
		return -2;
	}

    json_temp = cJSON_GetObjectItem(response_json, "warningduration");
	if(!json_temp) {
		GDGL_DEBUG("cJSON_GetObjectItem failed\n");
		cJSON_Delete(response_json);
		return -2;
	}
    *value = json_temp->valueint;

    cJSON_Delete(response_json);
    return 0;
}

int http_get_dev_modeid(char *mode_id, const char *ieee)
{
	int rt;
	char		result_str[MAX_RESULT] = {0};
	char		response_json_str[MAX_RESULT] = {0};
	char url_str[200];
	cJSON 		*response_json, *json_node, *json_temp;
	char *temp_mode_id;

	snprintf(url_str, sizeof(url_str), "http://127.0.0.1/cgi-bin/rest/network/zbGetZBNodeByIEEE.cgi?"
			"ieee=%s&callback=1234&encodemethod=NONE&sign=AAA", ieee);

	rt = http_curl_get(url_str, result_str);
	if(rt <0) {
		GDGL_DEBUG("invoke api failed\n");
		return -1;
	}
	get_json_str(response_json_str, result_str);
	response_json = cJSON_Parse(response_json_str);

	if (!response_json)
	{
		GDGL_DEBUG("json parse Error before: (%s)\n", cJSON_GetErrorPtr());
		return -2;
	}

    json_temp = cJSON_GetObjectItem(response_json, "response_params");
	if(!json_temp) {
		GDGL_DEBUG("cJSON_GetObjectItem failed\n");
		cJSON_Delete(response_json);
		return -2;
	}

	json_node = cJSON_GetObjectItem(json_temp, "node");
	if(!json_node) {
		GDGL_DEBUG("cJSON_GetObjectItem failed\n");
		cJSON_Delete(response_json);
		return -2;
	}

    json_temp = cJSON_GetObjectItem(json_node, "model_id");
	if(!json_temp) {
		GDGL_DEBUG("cJSON_GetObjectItem failed\n");
		cJSON_Delete(response_json);
		return -2;
	}
	temp_mode_id = json_temp->valuestring;
//	printf("temp_mode_id=%s\n", temp_mode_id);
	snprintf(mode_id, strlen(temp_mode_id)+1, "%s", temp_mode_id);

    cJSON_Delete(response_json);
    return 0;
}
