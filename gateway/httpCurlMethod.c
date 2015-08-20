/*
 *	File name   : httpCurlMethod.c
 *  Created on  : Apr 9, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
//#include "cJSON.h"
#include "smartgateway.h"
#include "httpCurlMethod.h"


#define	MAX_RESPONSE	65536
#define MAX_RESULT 65436
#define MAX_URL_LEN 4096
/***************************************************************************
  Function: get_json_str
  Description: get substring between first '{' and last '}' //截取json字符串
  Input:  raw_str
  Output: json_str
  Return: 0:OK / -1:Fail
  Others: none
***************************************************************************/
int get_json_str(char* json_str, char* raw_str)
{
    char* s_begin;
    char* s_end;
    int len;
    s_begin = strchr(raw_str, '{');
    s_end = strrchr(raw_str, '}');

    if (!s_begin || !s_end)
    {
        return -1;
    }

    len = s_end - s_begin + 1;
	if (len <= 0)
	{
		return -1;
	}
    strncpy(json_str, s_begin, len);
    json_str[len] = '\0';
    return 0;
}
/***************************************************************************
  Function: write_function
  Description: http request write callback
  Input:  ptr
             size
             nmemb
  Output: userdata
  Return: size*nmemb:OK
              other:Failed
  Others:  none
***************************************************************************/
static size_t write_function( char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t write_num;
	unsigned int len;

    write_num = size * nmemb;
	len = strlen(userdata);
	if ( (write_num + len) >= MAX_RESULT)
		return 0;

	memcpy(userdata + len, ptr, write_num);
	*((char *)userdata + len + write_num) = '\0';

	return write_num;
}

/***************************************************************************
  Function: child_perform_http_request
  Description: child perform http request  : 这是个http 同步get的方法,多进程调用也要等待respond
  Input:  url
  Output: result
  Return: 0:OK
              other:Failed
  Others:  none
***************************************************************************/
//static int child_perform_http_request(char * url, char * result)
//{
//
//    CURL *ctx;
//	CURLcode rc;
////	pid_t c_pid;
//
////	c_pid = getpid();
//
//	// create a context, sometimes known as a handle.
//	ctx = curl_easy_init();
//	if (NULL == ctx) {
////		GDGL_DEBUG("child %ld curl_easy_init Error\n", (long)c_pid);
//		return 1;
//	}
//
//	//curl_easy_setopt( ctx , CURLOPT_VERBOSE, 1 );
//
//	// target url:
//	rc = curl_easy_setopt( ctx , CURLOPT_URL,  url );
//	if (CURLE_OK != rc) {
////		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_URL Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
//		curl_easy_cleanup( ctx );
//		return 2;
//	}
//
//	// no progress bar:
//	rc = curl_easy_setopt( ctx , CURLOPT_NOPROGRESS , 1 );
//	if (CURLE_OK != rc) {
////		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_NOPROGRESS Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
//		curl_easy_cleanup( ctx );
//		return 2;
//	}
//
//	// include the header in the body output:
//	rc = curl_easy_setopt( ctx , CURLOPT_HEADER,  1 );
//	if (CURLE_OK != rc) {
////		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_HEADER Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
//		curl_easy_cleanup( ctx );
//		return 2;
//	}
//
//	rc = curl_easy_setopt( ctx , CURLOPT_PROTOCOLS,  CURLPROTO_HTTP );
//	if (CURLE_OK != rc) {
////		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_PROTOCOLS Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
//		curl_easy_cleanup( ctx );
//		return 2;
//	}
//
//    rc = curl_easy_setopt( ctx , CURLOPT_WRITEFUNCTION , write_function );
//	if (CURLE_OK != rc) {
////		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_WRITEFUNCTION Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
//		curl_easy_cleanup( ctx );
//		return 2;
//	}
////
//	rc = curl_easy_setopt( ctx , CURLOPT_WRITEDATA , (void *)result );
//	if (CURLE_OK != rc) {
////		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_WRITEDATA Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
//		curl_easy_cleanup( ctx );
//		return 2;
//	}
//
//	rc = curl_easy_perform( ctx );
//	if (CURLE_OK != rc) {
////		GDGL_DEBUG("child %ld curl_easy_perform Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
//		curl_easy_cleanup( ctx );
//		return 3;
//	}
//
//	curl_easy_cleanup( ctx );
//	return 0;
//}
/*
 * return: 0-success, <0-failed
 * */
int http_curl_get(const char *url, char *result)
{
    CURL *curl;
	CURLcode rc;
	int res = 0;

	curl = curl_easy_init();
	if(curl) {
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		//curl_easy_setopt(curl, CURLOPT_HEADER,  1);
		rc = curl_easy_setopt(curl, CURLOPT_URL, url);
		rc = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
		rc = curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP);
	    rc = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
		rc = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)result);
		rc = curl_easy_perform(curl);
		if (CURLE_OK != rc) {
			GDGL_DEBUG("curl_easy_perform Error: %s\n", curl_easy_strerror( rc ));
			exit(1);
			res = CURL_PERFORM_ERR;
		}
	}
	else {
		GDGL_DEBUG("curl_easy_init Error\n");
		exit(1);
		res = CURL_INIT_ERR;
	}
	curl_easy_cleanup(curl);
	return res;
}
///*
// * return: 0-success, <0-failed
// * */
//int http_curl_post(const char *url, const char *send_data, char *recv_data)
//{
//    CURL *curl;
//	CURLcode rc;
//	int res = 0;
//
//	ctx = curl_easy_init();
//	if(ctx) {
//		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
//		//curl_easy_setopt(curl, CURLOPT_HEADER,  1);
//		rc = curl_easy_setopt(curl, CURLOPT_URL, url);
//		rc = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
//		rc = curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP);
//	    rc = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);
//		rc = curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)result);
//		rc = curl_easy_perform(curl);
//		if (CURLE_OK != rc) {
//			GDGL_DEBUG("curl_easy_perform Error: %s\n", curl_easy_strerror( rc ));
//			res = CURL_PERFORM_ERR;
//		}
//	}
//	else {
//		GDGL_DEBUG("curl_easy_init Error\n");
//		res = CURL_INIT_ERR;
//	}
//	curl_easy_cleanup(curl);
//	return res;
//}
///*
// * function: http_invoke_get()
// * description: 只调用，返回信息不处理
// * input:
// * output:
// * return: 0-ok, <0-failed
// * others:
// * */
//int http_sysc_get(char* url_str)
//{
//	int res;
//	char result_str[65436];
//
//	//execute
//	res = child_perform_http_request(url_str, result_str);
//	if (res != 0) {
//		URLAPI_DEBUG("child_perform_http_request error, error value: %d\n",res);
//		return (res = ERROR_HTTP_INVOKE);
//	}
//	else {
//
//		//URLAPI_DEBUG("result:\n[%s]\n", result_str);
//		res = 0;
//	}
//
//	return res;
//}
///*
// * function: http_make_over()
// * description:
// * return: 0-ok, <0-failed
// * others:
// * */
//int http_make_start()
//{
//	int res;
//	CURLcode rc;
//	// global libcURL init
//	rc = curl_global_init( CURL_GLOBAL_ALL );
//	if (rc != CURLE_OK) {
//		URLAPI_DEBUG("curl_global_init error\n");
//		return res = ERROR_HTTP_INVOKE;
//	}
//	return 0;
//}
///*
// * function: http_make_over()
// * description: if need to over handle http or quit the process must invoke this function
// * */
//void http_make_over()
//{
//	curl_global_cleanup();
//}




