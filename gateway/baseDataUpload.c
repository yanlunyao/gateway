/*
 *	File name   : baseDataUpload.c
 *  Created on  : May 12, 2015
 *  Author      : yanly
 *  Description : 获取相关api返回信息，并上传到云代理
 *  			  相关api：getendpoint，getAllRoomInfo，getIPClist，localIASCIEOperation，GetLocalCIEList，GetAllBindList
 *  			  增加上传网关软硬件版本信息；
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

/*
 * 模块使用方法：
 * 执行命令：BaseDataUpload <参数1> <参数2> ...
 * 参数1：api的num，非必须,发送全部api时，留空。1就是GET_ENDPOINT_NUM
 * 参数2：api需要引入的参数，非必须。
 * */

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
#include "baseDataUpload.h"


#define USE_LIBCURL
#define APP_VERSION			"V01-01"

//#define DEBUG_SOCKET_TCP_TESTER
#ifdef	DEBUG_SOCKET_TCP_TESTER
#define PUSH_SERVER_IP		"192.168.1.238"
#else
#define PUSH_SERVER_IP		"127.0.0.1"
#endif


//////////////////
int reconnect_cnt =0;  //重连云代理次数，重连超过20次后，延时1小时候再重连。

#ifdef USE_RF_FUNCTION
char api_all_upload_enable[API_AMOUNT] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0X01}; //基础数据上传所有api返回结果集，1表示需要，0表示不需要
#else
char api_all_upload_enable[API_AMOUNT] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0X00}; //基础数据上传所有api返回结果集，1表示需要，0表示不需要
#endif
#ifdef USE_LIBCURL
char sift_api[][URL_STRING_LEN] = {
	"http://127.0.0.1/cgi-bin/rest/network/getendpoint.cgi?callback=1234&encodemethod=NONE&sign=AAA",
	"http://127.0.0.1/cgi-bin/rest/network/getAllRoomInfo.cgi?callback=1234&encodemethod=NONE&sign=AAA",
	"http://127.0.0.1/cgi-bin/rest/network/getIPClist.cgi?callback=1234&encodemethod=NONE&sign=AAA",
	"http://127.0.0.1/cgi-bin/rest/network/localIASCIEOperation.cgi?operatortype=5&param1=1&param2=2&param3=3&callback=1234&encodemethod=NONE&sign=AAA",
	"http://127.0.0.1/cgi-bin/rest/network/GetLocalCIEList.cgi?callback=1234&encodemethod=NONE&sign=AAA",
	"http://127.0.0.1/cgi-bin/rest/network/GetAllBindList.cgi?callback=1234&encodemethod=NONE&sign=AAA",
//	"http://127.0.0.1/cgi-bin/rest/network/getIeeeEndPoint.cgi?callback=1234&encodemethod=NONE&sign=AAA&ieee=",
	"http://127.0.0.1/cgi-bin/rest/network/GetEndpointByIeee.cgi?callback=1234&encodemethod=NONE&sign=AAA&ieee=",//需要读取参数，动态上传的api respond
	"http://127.0.0.1/cgi-bin/rest/network/GetSwVersion.cgi",
	"http://127.0.0.1/cgi-bin/rest/network/GetHwVersion.cgi",
	"http://127.0.0.1/cgi-bin/rest/network/GetRFDevList.cgi",
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
////////////////////////////////
int upload_2_server(const char *send_text, int send_size);
int invoke_api_and_upload(const char *url_str, int api_num);
void wati_server_init_ok();
/*
 * 入口参数： 0表示所有ape全部上传；其他表示对应的api上传
 * */
int main(int argc, char* argv[])
{
	CURLcode rc;
	int res;
	int i;

	printf("===============BaseDataUpload version:%s, COMPILE_TIME[%s: %s]\n", APP_VERSION, __DATE__, __TIME__);
	printf("=========================================upload api msg start=================================================\n");
	rc = curl_global_init(CURL_GLOBAL_ALL);
	if (rc != CURLE_OK) {
		GDGL_DEBUG("curl_global_init error\n");
		exit (1);
	}

	printf("entry parameter:%s\n",argv[1]);

	if(argv[1] ==NULL) {
		sleep(8); //add yanly150528  开机时启动此程序，需要延时，等待网络环境稳定
		wati_server_init_ok();
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
			if(argv[2] != NULL) {
				strcat(sift_api[entry_para-1], argv[2]); //+ ieee值
				GDGL_DEBUG("url_str: %s\n", sift_api[entry_para-1]);
				res = invoke_api_and_upload(sift_api[entry_para-1], entry_para);
			}
			else
				GDGL_DEBUG("GET_IEEEENDPOINT_NUM parameter2 invalid\n");
		}
		else {
			GDGL_DEBUG("url_str: %s\n", sift_api[entry_para-1]);
			res = invoke_api_and_upload(sift_api[entry_para-1], entry_para);
		}
	}

	curl_global_cleanup();
	printf("===============upload api msg over\n");
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

//	if ( connect(fd, (SA *) &servaddr, sizeof(servaddr)) < 0 ) {
//		GDGL_DEBUG("connect error, push addr:%s,port:%d\n",PUSH_SERVER_IP,FEATURE_GDGL_CPROXY_SIFT_API_PUSH_PORT);
//		close(fd);
//		return -1;
//	}
	//modify by yanly150528
	while ( connect(fd, (SA *) &servaddr, sizeof(servaddr)) < 0 ) {
		GDGL_DEBUG("connect error, push addr:%s,port:%d\n",PUSH_SERVER_IP,FEATURE_GDGL_CPROXY_SIFT_API_PUSH_PORT);
		reconnect_cnt++;
		if(reconnect_cnt > RECONNECT_MAX_CNT) {
			sleep(3600);
			reconnect_cnt = 0;
		}
		else
			sleep(1);
	}
	reconnect_cnt = 0;
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
void wati_server_init_ok()
{
	int fd;
	struct sockaddr_in	servaddr;

	while ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        GDGL_DEBUG("socket error\n");
		sleep(1);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(FEATURE_GDGL_CPROXY_SIFT_API_PUSH_PORT);

	while ( (inet_pton(AF_INET, PUSH_SERVER_IP, &servaddr.sin_addr)) <= 0) {
		GDGL_DEBUG("inet_pton error\n");
		sleep(1);
	}
	while ( connect(fd, (SA *) &servaddr, sizeof(servaddr)) < 0 ) {
		GDGL_DEBUG("connect error, push addr:%s,port:%d\n",PUSH_SERVER_IP,FEATURE_GDGL_CPROXY_SIFT_API_PUSH_PORT);
		reconnect_cnt++;
		if(reconnect_cnt > RECONNECT_MAX_CNT) {
			GDGL_DEBUG("sleep 3600 second\n");
			sleep(3600);
			reconnect_cnt = 0;
		}
		else
			sleep(1);
	}
	reconnect_cnt = 0;
	close(fd);
	GDGL_DEBUG("sever wait init ok\n");
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

	wati_server_init_ok();
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
		GDGL_DEBUG("new_json_str strlen:%d, string:%s\n", strlen(new_json_str), new_json_str);
		rt = upload_2_server(new_json_str, strlen(new_json_str));
		free(new_json_str);
		cJSON_Delete(new_json);
		return rt;
	}
	cJSON_Delete(response_json);
	return rt;
}
