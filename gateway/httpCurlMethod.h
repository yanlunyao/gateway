/*
 *	File name   : httpCurlMethod.h
 *  Created on  : Apr 9, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#ifndef HTTPCURLMETHOD_H_
#define HTTPCURLMETHOD_H_

#define CURL_INIT_ERR			-1
#define CURL_SET_OPT_ERR		-2
#define	CURL_PERFORM_ERR		-3

#define	MAX_RESPONSE	65536
#define MAX_RESULT 65436
#define MAX_URL_LEN 4096

//extern function
//void http_make_over();
//int http_make_start();
//int http_sysc_get(char* url_str);
int get_json_str(char* json_str, char* raw_str);
int http_curl_get(const char *url, char *result);
#endif /* HTTPCURLMETHOD_H_ */
