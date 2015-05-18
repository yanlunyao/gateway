/*
 *	File name   : baseDataUpload.c
 *  Created on  : May 12, 2015
 *  Author      : yanly
 *  Description : 获取相关api返回信息，并上传到云代理
 *  			  相关api：getendpoint，getAllRoomInfo，getIPClist，localIASCIEOperation，GetLocalCIEList，GetAllBindList
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include "unp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <curl/curl.h>
#include <time.h>
#include "smartgateway.h"
#include "cJSON.h"
#include "httpCurlMethod.h"


#define USE_LIBCURL

#define DEBUG_SOCKET_TCP_TESTER
#ifdef	DEBUG_SOCKET_TCP_TESTER
#define PUSH_SERVER_IP		"192.168.1.149"
#else
#define PUSH_SERVER_IP		"127.0.0.1"
#endif

#define GET_ENDPOINT_NUM					1
#define GET_ALLROOMINFO_NUM					2
#define GET_IPCLIST_NUM						3
#define GET_LOCALIASCIE_NUM					4
#define GET_CIELIST_NUM						5
#define GET_ALLBINDLIST_NUM					6
#define GET_IEEEENDPOINT_NUM				7
#define API_AMOUNT							GET_IEEEENDPOINT_NUM

char api_all_upload_enable[API_AMOUNT] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00}; //上传所有api返回结果集，1表示需要，0表示不需要

#ifdef USE_LIBCURL
char sift_api[][URL_STRING_LEN] = {
	"http://127.0.0.1/cgi-bin/rest/network/getendpoint.cgi?callback=1234&encodemethod=NONE&sign=AAA",
	"http://127.0.0.1/cgi-bin/rest/network/getAllRoomInfo.cgi?callback=1234&encodemethod=NONE&sign=AAA",
	"http://127.0.0.1/cgi-bin/rest/network/getIPClist.cgi?callback=1234&encodemethod=NONE&sign=AAA",
	"http://127.0.0.1/cgi-bin/rest/network/localIASCIEOperation.cgi?operatortype=5&param1=1&param2=2&param3=3&callback=1234&encodemethod=NONE&sign=AAA",
	"http://127.0.0.1/cgi-bin/rest/network/GetLocalCIEList.cgi?callback=1234&encodemethod=NONE&sign=AAA",
	"http://127.0.0.1/cgi-bin/rest/network/GetAllBindList.cgi?callback=1234&encodemethod=NONE&sign=AAA",
	"http://127.0.0.1/cgi-bin/rest/network/getIeeeEndPoint.cgi?callback=1234&encodemethod=NONE&sign=AAA&ieee=", //需要读取参数，动态上传的api respond
	"\0"
};
#else
const char sift_api[API_AMOUNT][] = {

	"GET /cgi-bin/rest/network/getendpoint.cgi?callback=1234&encodemethod=NONE&sign=AAA "
	"HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n",

	"GET /cgi-bin/rest/network/getAllRoomInfo.cgi?callback=1234&encodemethod=NONE&sign=AAA "
	"HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n",

	"GET /cgi-bin/rest/network/getIPClist.cgi?callback=1234&encodemethod=NONE&sign=AAA "
	"HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n",

	"GET /cgi-bin/rest/network/localIASCIEOperation.cgi?operatortype=5&param1=1&param2=2&param3=3&callback=1234&encodemethod=NONE&sign=AAA "
	"HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n",

	"GET /cgi-bin/rest/network/GetLocalCIEList.cgi?callback=1234&encodemethod=NONE&sign=AAA "
	"HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n",

	"GET /cgi-bin/rest/network/GetAllBindList.cgi?callback=1234&encodemethod=NONE&sign=AAA "
	"HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"
};
#endif

int upload_2_server(const char *send_text, int send_size);
int invoke_api_and_upload(const char *url_str, int api_num);
/*
 * 入口参数： 0表示所有api全部上传；其他表示对应的api上传
 * */
int main(int argc, char* argv[])
{
	CURLcode rc;
	int res;
	int i;

	sleep(1);
	printf("=========================================upload api msg start=================================================\n");
	rc = curl_global_init(CURL_GLOBAL_ALL);
	if (rc != CURLE_OK) {
		GDGL_DEBUG("curl_global_init error\n");
		exit (1);
	}

	printf("entry parameter:%s\n",argv[1]);

	if(argv[1] ==NULL) {
		for(i=0; i<API_AMOUNT; i++) {
			if(api_all_upload_enable[i] == 0x01) {
				GDGL_DEBUG("url_str: %s\n", sift_api[i]);
				res = invoke_api_and_upload(sift_api[i], i+1);
			}
		}
	}
	else {

		int entry_para = atoi(argv[1]);
		if(entry_para > API_AMOUNT) {
			GDGL_DEBUG("entry parameter invalid\n");
		}
		else if(entry_para == 0) {
			for(i=0; i<API_AMOUNT; i++) {
				if(api_all_upload_enable[i] == 0x01) {
					GDGL_DEBUG("url_str: %s\n", sift_api[i]);
					res = invoke_api_and_upload(sift_api[i], i+1);
				}
			}
//			GDGL_DEBUG("entry parameter invalid\n");
		}
		else if(entry_para == GET_IEEEENDPOINT_NUM) {
			strcat(sift_api[entry_para-1], argv[2]); //+ ieee值
			GDGL_DEBUG("url_str: %s\n", sift_api[entry_para-1]);
			res = invoke_api_and_upload(sift_api[entry_para-1], entry_para);
		}
		else {
			GDGL_DEBUG("url_str: %s\n", sift_api[entry_para-1]);
			res = invoke_api_and_upload(sift_api[entry_para-1], entry_para);
		}
	}

	curl_global_cleanup();
	printf("=========================================upload api msg over=================================================\n");
	return 0;
}

/*
* Function: upload_2_server
* Description:
* Input: send
* Output: none
* Return: 0>>push success;
*	  	  -1>>push error,socket send error
* Others:  none
*/

int upload_2_server(const char *send_text, int send_size)
{
	int fd;
	struct sockaddr_in	servaddr;

	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        GDGL_DEBUG("socket error\n");
		return -1;
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(FEATURE_GDGL_CPROXY_SIFT_API_PUSH_PORT);

	if ( (inet_pton(AF_INET, PUSH_SERVER_IP, &servaddr.sin_addr)) <= 0) {
		GDGL_DEBUG("inet_pton error\n");
		close(fd);
		return -2;
	}

	if ( connect(fd, (SA *) &servaddr, sizeof(servaddr)) < 0 ) {
		GDGL_DEBUG("connect error\n");
		close(fd);
		return -1;
	}
	#ifdef DEBUG_SOCKET_TCP_TESTER
//	sleep(1); //debug yan
	#endif
    if ( writen(fd, send_text, send_size) != send_size ) {
    	GDGL_DEBUG("write error\n");
		close(fd);
		return -1;
    }
	#ifdef DEBUG_SOCKET_TCP_TESTER
//	sleep(3); //debug yan
	#endif
	close(fd);

	return 0;
}
/*
 * 调api后将数据上传至云代理
 * */
int invoke_api_and_upload(const char *url_str, int api_num)
{
	time_t		timestamp;
//	char timestamp_str[10] = {0};

	int			rt =0;
	char		result_str[MAX_RESULT] = {0};
	char		response_json_str[MAX_RESULT] = {0};

	cJSON 		*response_json;
	cJSON	    *new_json;
	char 		*new_json_str;

	timestamp = time(NULL);
//	snprintf(timestamp_str, sizeof(timestamp_str), "%ld", timestamp);

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
	else
	{
		new_json = cJSON_CreateObject();
		cJSON_AddNumberToObject(new_json, "datatype", api_num);
		cJSON_AddItemToObject(new_json, "data", response_json);
		cJSON_AddNumberToObject(new_json, "uploadtime", timestamp);
		new_json_str = cJSON_PrintUnformatted(new_json);
//		printf("new_json_str:%s, strlen:%d\n", new_json_str, strlen(new_json_str));
		GDGL_DEBUG("new_json_str strlen:%d\n", strlen(new_json_str));
		rt = upload_2_server(new_json_str, strlen(new_json_str));
		free(new_json_str);
		cJSON_Delete(new_json);
		return rt;
	}
	cJSON_Delete(response_json);
	return rt;
}
