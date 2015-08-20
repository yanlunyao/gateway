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
