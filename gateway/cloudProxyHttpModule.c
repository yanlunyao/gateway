/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName: cloudProxyHttpModule.c
  Author: fengqiuchao      
  Version : V1.0         
  Date: 2014/03/17
  Description: cloud proxy http module
  History:
      <author>       <time>     <version >   <desc>
      fengqiuchao    2014/03/17     1.0     build this moudle  
***************************************************************************/

#include "unp.h"
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include <curl/curl.h>
#include "smartgateway.h"

static pid_t child_make(int listenfd, int addrlen);
static void child_doit(int sockfd);
static int child_response(const int status, const char * result_str, char * response_str);
static int child_perform_http_request(char * url, char * result);
static size_t write_function( char *ptr, size_t size, size_t nmemb, void *userdata);
static int get_json_str(char* json_str, char* raw_str);


#define CHILD_NUM 20
#define	MAX_RESPONSE	65536
#define MAX_RESULT 65436
#define MAX_URL_LEN 4096
#define CLOUD_PROXY_HTTP_RESPONSE_PERFORM_REQ_SUCCESS 1
#define CLOUD_PROXY_HTTP_RESPONSE_JSON_PARSE_FAILED (-1)
#define CLOUD_PROXY_HTTP_RESPONSE_URL_NOT_EXIST (-2)
#define CLOUD_PROXY_HTTP_RESPONSE_URL_NOT_STRING (-3)
#define CLOUD_PROXY_HTTP_RESPONSE_PERFORM_REQ_ERROR (-4)
#define CLOUD_PROXY_HTTP_RESPONSE_REQ_HANDLER_INVALID (-5)
#define CLOUD_PROXY_HTTP_RESPONSE_RES_JSON_PARSE_FAILED (-6)


/***************************************************************************
  Function: main
  Description: 
  Input:  
  Output: 
  Return: 
  Others:  none
***************************************************************************/
int main(int argc, char **argv)
{
	int 		listenfd, i;
	socklen_t	addrlen;

	listenfd = Tcp_listen(NULL, FEATURE_GDGL_CPROXY_HTTPMODULE_PORT_STR, &addrlen);
	
	for (i = 0; i < CHILD_NUM; i++)
		child_make(listenfd, addrlen); /* parent returns */

	for ( ; ; )
		pause();	/* everything done by children */
}

/***************************************************************************
  Function: child_make
  Description: create child process
  Input:  listenfd
            addrlen
  Output: none
  Return: child pid
  Others:  none
***************************************************************************/
static pid_t child_make(int listenfd, int addrlen)
{
	pid_t	pid;
	int				connfd;
	socklen_t		clilen;
	struct sockaddr	*cliaddr;
	CURLcode rc;

	if ( (pid = Fork()) > 0)
		return(pid);		/* parent */

	cliaddr = Malloc(addrlen);

	GDGL_DEBUG("child %ld starting\n", (long) getpid());
	
	// global libcURL init	
	rc = curl_global_init( CURL_GLOBAL_ALL );
	if (rc != CURLE_OK) {
		GDGL_DEBUG("child %ld curl_global_init error\n", (long) getpid());
		exit (1);
	}
	
	for ( ; ; ) {
		clilen = addrlen;
		connfd = Accept(listenfd, cliaddr, &clilen);

		child_doit(connfd);		/* process the request */
		Close(connfd);
	}

	curl_global_cleanup();
}

/***************************************************************************
  Function: child_doit
  Description: child process the request
  Input:  sockfd
  Output: none
  Return: none
  Others:  none
***************************************************************************/
static void child_doit(int sockfd)
{
	int			rt;
	ssize_t		nread, nwrite;
	char		request[MAXLINE], url_str[MAX_URL_LEN], response[MAX_RESPONSE] = {0}, result_str[MAX_RESULT] = {0};
	char		response_json_str[MAX_RESULT] = {0}, *response_json_str_reduce;
	cJSON 		*request_json, *url_json, *response_json;
	pid_t c_pid;

	c_pid = getpid();

again:  
	if ( (nread = read(sockfd, request, MAXLINE-1)) < 0 ) {
		if (errno == EINTR)
			goto again;		/* call read() again */
		else {
			GDGL_DEBUG("child %ld read error\n", (long)c_pid);
			return;
		}
    }
	else if ( nread == 0) {
		GDGL_DEBUG("child %ld connection closed by other end\n", (long)c_pid);
		return;		/* connection closed by other end */
	}
	request[nread] = 0; // add null

	request_json = cJSON_Parse(request);
	if (!request_json) {
		GDGL_DEBUG("child %ld request_json parse Error before: (%s)\n", (long)c_pid, cJSON_GetErrorPtr());
		nwrite = child_response(CLOUD_PROXY_HTTP_RESPONSE_JSON_PARSE_FAILED, NULL, response);
	}
	else {
		url_json = cJSON_GetObjectItem(request_json, "url");
		if (!url_json) {
			GDGL_DEBUG("child %ld url key not exist\n", (long)c_pid);
			cJSON_Delete(request_json);
			nwrite = child_response(CLOUD_PROXY_HTTP_RESPONSE_URL_NOT_EXIST, NULL, response);
			goto end;
		}
		if (url_json->type != cJSON_String) {
		    GDGL_DEBUG("child %ld url is not a string\n", (long)c_pid);
			cJSON_Delete(request_json);
		    nwrite = child_response(CLOUD_PROXY_HTTP_RESPONSE_URL_NOT_STRING, NULL, response);
			goto end;
	    }

		snprintf(url_str, MAX_URL_LEN, "%s", url_json->valuestring);
		cJSON_Delete(request_json);
		GDGL_DEBUG("child %ld URL:\n[%s]\n", (long)c_pid, url_str);

		rt = child_perform_http_request(url_str, result_str);
		if (rt != 0) {
		    nwrite = child_response(CLOUD_PROXY_HTTP_RESPONSE_PERFORM_REQ_ERROR, NULL, response);
		}
		else {
			GDGL_DEBUG("child %ld result:\n[%s]\n", (long)c_pid, result_str);
			get_json_str(response_json_str, result_str);
			response_json = cJSON_Parse(response_json_str);
			if (!response_json) 
			{
				GDGL_DEBUG("child %ld response_json parse Error before: (%s)\n", (long)c_pid, cJSON_GetErrorPtr());
				//GDGL_DEBUG("child %ld response_json parse Error 111\n", (long)c_pid);
				nwrite = child_response(CLOUD_PROXY_HTTP_RESPONSE_RES_JSON_PARSE_FAILED, NULL, response);
			}
			else
			{
				response_json_str_reduce = cJSON_PrintUnformatted(response_json);
				nwrite = child_response(CLOUD_PROXY_HTTP_RESPONSE_PERFORM_REQ_SUCCESS, response_json_str_reduce, response);
				free(response_json_str_reduce);
			}
			cJSON_Delete(response_json); 
		}
	}

end:
    if (nwrite > 0) {
	    rt = writen(sockfd, response, nwrite);
		if (rt != nwrite)
			GDGL_DEBUG("child %ld write error, %d/%d\n", (long)c_pid, rt, nwrite);
    }
	return;
}

/***************************************************************************
  Function: child_response
  Description: child construct response JSON string
  Input:  status
             result_str
  Output: response_str
  Return: negative:error
              positive:the length of response_str, including null
  Others:  none
***************************************************************************/
static int child_response(const int status, const char * result_str, char * response_str)
{
    char *out;
	cJSON *response_json;
	int re;
	pid_t c_pid;

	c_pid = getpid();

	if ( (status > 0) && (!result_str) ) {
		GDGL_DEBUG("child %ld status OK but result_str is NULL\n", (long)c_pid);
		return -1;
	}
	
	response_json = cJSON_CreateObject();
	if (!response_json) {
		GDGL_DEBUG("child %ld Create response_json failed\n", (long)c_pid);
		return -2;
	}
	cJSON_AddNumberToObject(response_json, "status", status);
	if (status > 0) {
		cJSON_AddStringToObject(response_json, "result", result_str);
	}
	
	out = cJSON_PrintUnformatted(response_json);
	cJSON_Delete(response_json);
	if (!out) {
		GDGL_DEBUG("child %ld cJSON_PrintUnformatted response_json failed\n", (long)c_pid);
		return -3;
	}
	re = snprintf(response_str, MAX_RESPONSE, "%s", out);	
	free(out);

	return (re+1); //including null
}

/***************************************************************************
  Function: child_perform_http_request
  Description: child perform http request
  Input:  url
  Output: result
  Return: 0:OK
              other:Failed
  Others:  none
***************************************************************************/
static int child_perform_http_request(char * url, char * result)
{
    
    CURL *ctx;
	CURLcode rc;
	pid_t c_pid;

	c_pid = getpid();
	
	// create a context, sometimes known as a handle.
	ctx = curl_easy_init();	
	if (NULL == ctx) {
		GDGL_DEBUG("child %ld curl_easy_init Error\n", (long)c_pid);
		return 1;
	}

	//curl_easy_setopt( ctx , CURLOPT_VERBOSE, 1 );

	// target url:	
	rc = curl_easy_setopt( ctx , CURLOPT_URL,  url );
	if (CURLE_OK != rc) {
		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_URL Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
		curl_easy_cleanup( ctx );
		return 2;
	}

	// no progress bar:	
	rc = curl_easy_setopt( ctx , CURLOPT_NOPROGRESS , 1 );
	if (CURLE_OK != rc) {
		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_NOPROGRESS Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
		curl_easy_cleanup( ctx );
		return 2;
	}

	// include the header in the body output:	
	rc = curl_easy_setopt( ctx , CURLOPT_HEADER,  1 );
	if (CURLE_OK != rc) {
		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_HEADER Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
		curl_easy_cleanup( ctx );
		return 2;
	}

	rc = curl_easy_setopt( ctx , CURLOPT_PROTOCOLS,  CURLPROTO_HTTP );
	if (CURLE_OK != rc) {
		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_PROTOCOLS Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
		curl_easy_cleanup( ctx );
		return 2;
	}

    rc = curl_easy_setopt( ctx , CURLOPT_WRITEFUNCTION , write_function );
	if (CURLE_OK != rc) {
		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_WRITEFUNCTION Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
		curl_easy_cleanup( ctx );
		return 2;
	}
	
	rc = curl_easy_setopt( ctx , CURLOPT_WRITEDATA , (void *)result );
	if (CURLE_OK != rc) {
		GDGL_DEBUG("child %ld curl_easy_setopt CURLOPT_WRITEDATA Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
		curl_easy_cleanup( ctx );
		return 2;
	}	

	rc = curl_easy_perform( ctx );
	if (CURLE_OK != rc) {
		GDGL_DEBUG("child %ld curl_easy_perform Error: %s\n", (long)c_pid, curl_easy_strerror( rc ));
		curl_easy_cleanup( ctx );
		return 3;
	}

	curl_easy_cleanup( ctx );
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
  Function: get_json_str
  Description: get substring between first '{' and last '}'
  Input:  raw_str
  Output: json_str
  Return: 0:OK / -1:Fail
  Others: none
***************************************************************************/
static int get_json_str(char* json_str, char* raw_str)
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


