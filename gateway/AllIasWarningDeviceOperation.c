/*
 *	File name   : AllIasWarningDeviceOperation.c
 *  Created on  : Aug 17, 2015
 *  Author      : yanly
 *  Description : 所有大洋报警器的统一操作，如停止所有大洋报警器报警，开启所有报警，API调用例子：
 *  http://192.168.1.160/cgi-bin/rest/network/AllIasWarningDeviceOperation.cgi?param1=0&param2=0&param3=0&operatortype=0；
 *  参数param1，param2，param3，operatortype值描述与API的iasWarningDeviceOperation一致。
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cgic.h"
#include "smartgateway.h"
#include "cgic.h"
#include "cJSON.h"
#include <curl/curl.h>
#include "httpCurlMethod.h"


const char getzbnode[]= "http://127.0.0.1/cgi-bin/rest/network/getZBNode.cgi?callback=1234&encodemethod=NONE&sign=AAA";

static void api_response(int res)
{
	cJSON *root;
	char *json_out;

	root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "status", res);
	json_out=cJSON_Print(root);
	cJSON_Delete(root);
	fprintf(cgiOut,"%s\n", json_out);
	free(json_out);
}

int cgiMain()
{
	int res=0;
	int param1, param2, param3, operatortype;

	int rt;
	CURLcode rc;
	char		result_str[MAX_RESULT] = {0};
	char		response_json_str[MAX_RESULT] = {0};
	cJSON 		*response_json;

	cgiFormResultType cgi_re;
	cgiHeaderContentType("application/json"); //MIME

	//获取
	cgi_re = cgiFormInteger("operatortype", &operatortype, -1);
	cgi_re = cgiFormInteger("param1", &param1, -1);
	cgi_re = cgiFormInteger("param2", &param2, -1);
	cgi_re = cgiFormInteger("param3", &param3, -1);

	//调api
	rc = curl_global_init(CURL_GLOBAL_ALL);
	if (rc != CURLE_OK) {
		printf("curl_global_init error\n");
		res = -1;
		goto over;
	}

	rt = http_curl_get(getzbnode, result_str);
	if(rt <0) {
		printf("invoke api failed\n");
		res =-1;
		goto over;
	}
	get_json_str(response_json_str, result_str);
	response_json = cJSON_Parse(response_json_str);
	if (!response_json)
	{
		printf("json parse Error before: (%s)\n", cJSON_GetErrorPtr());
		res= -2;
		goto over;
	}
    cJSON* json_array;
    cJSON* json_node;
    cJSON* json_tmp;

    json_array = cJSON_GetObjectItem(response_json, "response_params");
    if (!json_array)
    {
        cJSON_Delete(response_json);
		res= -2;
		goto over;
    }
    int array_size = cJSON_GetArraySize(json_array);
    if (array_size == 0)
    {
        cJSON_Delete(response_json);
		res= -2;
		goto over;
    }

    char* ieee;
    char* modeid;
    char request[200] = {0};
    int i = 0;
    for (i = 0; i < array_size; ++i)
    {
        // 对于其中某个设备操作失败直接忽略
        json_tmp = cJSON_GetArrayItem(json_array, i);
        if (!json_tmp)
        {
            continue;
        }
        json_node = cJSON_GetObjectItem(json_tmp, "node");
        if (!json_node)
        {
            continue;
        }
        json_tmp = cJSON_GetObjectItem(json_node, "model_id");
        if (!json_tmp)
        {
            continue;
        }
        modeid = json_tmp->valuestring;
        if(memcmp(modeid, "Z602A", 5) !=0)
        {
        	continue;
        }
        json_tmp = cJSON_GetObjectItem(json_node, "ieee");
        if (!json_tmp)
        {
            continue;
        }
        ieee = json_tmp->valuestring;
        snprintf(request, sizeof(request), "http://127.0.0.1/cgi-bin/rest/network/iasWarningDeviceOperation.cgi?"
        		"ep=01&ieee=%s&param1=%d&param2=%d&param3=%d&operatortype=%d&callback=1234&encodemethod=NONE&sign=AAA",
				ieee, param1, param2, param3, operatortype);
    	rt = http_curl_get(request, result_str);
    	if(rt <0) {
    		printf("invoke api failed\n");
    	}
    }
	cJSON_Delete(response_json);

over:
	api_response(res);
	curl_global_cleanup();
	return 0;
}
