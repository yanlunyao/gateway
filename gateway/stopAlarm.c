/*
 *	File name   : stopAlarm.c
 *  Created on  : Jul 1, 2015
 *  Author      : yanly
 *  Description : 停止大洋zigbee报警器报警和网关报警：获取大洋ep信息，得到报警器ieee发送停止报警api；调用killall mplayer停止网关报警；
 *  Version     : V01-00
 *  History     : <author>		<time>		<version>		<desc>
 *  				yanly		150806		V01-01		    增加停止所有RF警号报警
 *  				yanly		151228		V02-01			屏蔽Zigbee功能
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cgic.h"
#include "cJSON.h"
#include <curl/curl.h>
#include "httpCurlMethod.h"
#include "apiComWithRFDaemon.h"
#include "smartgateway.h"

static void api_response(int res)
{
	cJSON *root;
	char *json_out;

	root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "stopalarm_status", res);
	json_out=cJSON_Print(root);
	cJSON_Delete(root);
	fprintf(cgiOut,"%s\n", json_out);
	free(json_out);
}

//const char getzbnode[]= "http://127.0.0.1/cgi-bin/rest/network/getZBNode.cgi?callback=1234&encodemethod=NONE&sign=AAA";
#ifdef USE_ZIGBEE_FUNCTION
const char stopZigbeeWarning[]= "http://127.0.0.1/cgi-bin/rest/network/AllIasWarningDeviceOperation.cgi?param1=0&param2=0&param3=0&operatortype=0";
#endif
const char stopAllRFWarning[] = "{\n	\"api\": \"RFWarningDevOperation\",\n	\"rfid\": \"\",\n	\"operatortype\": 2,\n	\"param1\": 0\n}";
//{\n	\"api\": \"RFWarningDevOperation\",\n	\"rfid\": \"%s\",\n	\"operatortype\": %d,\n	\"param1\": %d\n}
int cgiMain()
{
	int res=0;
#ifdef USE_ZIGBEE_FUNCTION
	int rt;
	CURLcode rc;
	char		result_str[MAX_RESULT] = {0};
#endif
//	char		response_json_str[MAX_RESULT] = {0};
//	cJSON 		*response_json;

	cgiHeaderContentType("application/json"); //MIME

	//停止网关报警:
	system("killall mplayer");

	#ifdef USE_RF_FUNCTION
	//停止所有RF警号报警
	communicateWithRF(stopAllRFWarning, strlen(stopAllRFWarning)+1, NULL);
	RF_DEBUG("%s\n", stopAllRFWarning);
	#endif
#ifdef USE_ZIGBEE_FUNCTION
	//停止zigbee报警器报警:
	rc = curl_global_init(CURL_GLOBAL_ALL);
	if (rc != CURLE_OK) {
		printf("curl_global_init error\n");
		res = -1;
		goto over;
	}

	rt = http_curl_get(stopZigbeeWarning, result_str);
	if(rt <0) {
		printf("invoke api failed\n");
		res =-1;
		goto over;
	}
over:
#endif
    api_response(res);
    curl_global_cleanup();
	return 0;
}

//int cgiMain()
//{
//	int res=0;
//	int rt;
//	CURLcode rc;
//	char		result_str[MAX_RESULT] = {0};
//	char		response_json_str[MAX_RESULT] = {0};
//	cJSON 		*response_json;
//
//	cgiHeaderContentType("application/json"); //MIME
//
//	//停止网关报警:
//	system("killall mplayer");
//
//	#ifdef USE_RF_FUNCTION
//	//停止所有RF警号报警
//	communicateWithRF(stopAllRFWarning, strlen(stopAllRFWarning)+1, NULL);
//	RF_DEBUG("%s\n", stopAllRFWarning);
//	#endif
//
//	//停止zigbee报警器报警:
//	rc = curl_global_init(CURL_GLOBAL_ALL);
//	if (rc != CURLE_OK) {
//		printf("curl_global_init error\n");
//		res = -1;
//		goto over;
//	}
////
//	rt = http_curl_get(getzbnode, result_str);
//	if(rt <0) {
//		printf("invoke api failed\n");
//		res =-1;
//		goto over;
//	}
//	get_json_str(response_json_str, result_str);
//	response_json = cJSON_Parse(response_json_str);
//	if (!response_json)
//	{
//		printf("json parse Error before: (%s)\n", cJSON_GetErrorPtr());
//		res= -2;
//		goto over;
//	}
//    cJSON* json_array;
//    cJSON* json_node;
//    cJSON* json_tmp;
//
//    json_array = cJSON_GetObjectItem(response_json, "response_params");
//    if (!json_array)
//    {
//        cJSON_Delete(response_json);
//		res= -2;
//		goto over;
//    }
//    int array_size = cJSON_GetArraySize(json_array);
//    if (array_size == 0)
//    {
//        cJSON_Delete(response_json);
//		res= -2;
//		goto over;
//    }
//
//    char* ieee;
//    char* modeid;
//    char request[200] = {0};
//    int i = 0;
//    for (i = 0; i < array_size; ++i)
//    {
//        // 对于其中某个设备操作失败直接忽略
//        json_tmp = cJSON_GetArrayItem(json_array, i);
//        if (!json_tmp)
//        {
//            continue;
//        }
//        json_node = cJSON_GetObjectItem(json_tmp, "node");
//        if (!json_node)
//        {
//            continue;
//        }
//        json_tmp = cJSON_GetObjectItem(json_node, "model_id");
//        if (!json_tmp)
//        {
//            continue;
//        }
//        modeid = json_tmp->valuestring;
//        if(memcmp(modeid, "Z602A", 5) !=0)
//        {
//        	continue;
//        }
//        json_tmp = cJSON_GetObjectItem(json_node, "ieee");
//        if (!json_tmp)
//        {
//            continue;
//        }
//        ieee = json_tmp->valuestring;
//        //发送停止报警api
////        ieee = cJSON_PrintRawString(json_tmp);
//        snprintf(request, sizeof(request), "http://127.0.0.1/cgi-bin/rest/network/iasWarningDeviceOperation.cgi?ep=01&ieee=%s&param1=0&param2=0&param3=0&operatortype=0&callback=1234&encodemethod=NONE&sign=AAA", ieee);
////        printf("%s\n", request);
//    	rt = http_curl_get(request, result_str);
//    	if(rt <0) {
//    		printf("invoke api failed\n");
//    	}
//    }
//	cJSON_Delete(response_json);
//
//over:
//    api_response(res);
//    curl_global_cleanup();
//	return 0;
//}
