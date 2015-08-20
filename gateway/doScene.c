/*
 *	File name   : doScene.c
 *  Created on  : Apr 2, 2015
 *  Author      : yanly
 *  Description :
 *  Version     : v1.0
 *  History     : <author>		<time>			<version>		<desc>
 *                yanly			2015-04-07		v1.00			doScene
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "cgic.h"
#include "cJSON.h"
#include "sqliteOperator.h"
#include "glCalkProtocol.h"
#include "httpCurlMethod.h"
#include "apiComWithRFDaemon.h"

#define ALL_UNBYPASS			-1			//全部布防
#define ALL_BYPASS				-2

static int http_make_all_bypass(int scene_id);
static int http_do_scene_by_socket(const char *urlstring);
static void api_response(int res, int sid)
{
    cJSON *root;
    char *json_out;

    root=cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", res);

    if(res >=0){
    	cJSON_AddNumberToObject(root, "sid", sid);
    }

    json_out=cJSON_Print(root);
    cJSON_Delete(root);

    fprintf(cgiOut,"%s\n", json_out);
    free(json_out);
}
//int http_make(char *urlstring)
//{
//	int res;
//	pid_t	pid;
//	if ( (pid = fork()) > 0)
//		return(pid);		/* parent */
//
//	res = http_make_start();
//	if(res <0)
//		exit(1);
//	http_sysc_get(urlstring);
//	http_make_over();
//	exit(1);
//}
int cgiMain()
{
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;
	int i;
	char url[URL_STRING_LEN+1];

	url_string_st *acturl=NULL;

	cgiFormResultType cgi_re;
	int res;
	int sid;

	cgiHeaderContentType("application/json"); //MIME
	fflush(stdout);
//	fflush(stdin);

	//read sid
	cgi_re = cgiFormInteger("sid", &sid, 0);

	res = db_init();
	if(res<0){
		goto all_over;
	}
	if((sid == ALL_BYPASS)||(sid == ALL_UNBYPASS)){

		res = http_make_all_bypass(sid);
		goto all_over;
	}

	//read database
	res = read_scene_act_db(sid, &acturl);
	if(res <0){
		goto all_over;
	}
	for (i = 0; i < acturl[0].urltotal; i++){
		snprintf(url, URL_STRING_LEN+1, "GET %s HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", acturl[i].urlstring);
		http_do_scene_by_socket(url);
//		http_do_scene_by_socket(acturl[i].urlstring);
		//http_make(acturl[i].urlstring);
	}
//	if((res = http_make_start() ) ==0) {
//
//		for(i=0; i<acturl[0].urltotal; i++){
//
//			res = http_sysc_get(acturl[i].urlstring);
//			if(res <0)
//				break;
//		}
//
//		http_make_over();
//	}

	//http url handle

all_over:
	db_close();
	//respond
    api_response(res, sid);

    //free
    if(acturl){
    	free(acturl);
    	//URLAPI_DEBUG("free\n");
    }


    //push to cb_daemon
#ifdef NO_CALLBAK_DEBUG
#else
	if((send_cb_len = cJsonDelDoScene_callback(send_cb_string, sid, DO_SUBID_SCENE, res)) >=0) {
		push_to_CBDaemon(send_cb_string, send_cb_len);
	}
#endif
	return 0;
}

#define 	BUF_SIZE        1024
#define 	ERROR_DOSCENE_INVOKE_URL	-10
#define 	ERROR_SOCKET_CREAT			ERROR_DOSCENE_INVOKE_URL
#define 	ERROR_SOCKET_CONNECT		ERROR_DOSCENE_INVOKE_URL
#define 	ERROR_SOCKET_RECEIVED		ERROR_DOSCENE_INVOKE_URL
#define 	ERROR_JASON_PARSE			-11

const char GetZBNode[] = "GET /cgi-bin/rest/network/getendpoint.cgi?"
                        "callback=1234&encodemethod=NONE&sign=AAA HTTP/1.1\r\n"
                        "Host: 127.0.0.1\r\n\r\n";

const char global_bypass[] = "GET /cgi-bin/rest/network/localIASCIEOperation.cgi?"
                        "operatortype=6&param1=1&param2=2&param3=3&callback=1234&encodemethod=NONE&sign=AAA HTTP/1.1\r\n"
                        "Host: 127.0.0.1\r\n\r\n";

const char global_unbypass[] = "GET /cgi-bin/rest/network/localIASCIEOperation.cgi?"
                        "operatortype=7&param1=1&param2=2&param3=3&callback=1234&encodemethod=NONE&sign=AAA HTTP/1.1\r\n"
                        "Host: 127.0.0.1\r\n\r\n";

const char rf_bypass[] = "{\n	\"api\": \"ChangeAllRFDevArmState\",\n	\"para1\": \"0\"\n}";
const char rf_unbypass[]= "{\n	\"api\": \"ChangeAllRFDevArmState\",\n	\"para1\": \"1\"\n}";

static int http_do_scene_by_socket(const char *urlstring)
{
    struct sockaddr_in remote_addr;
    int socket_fd;
    int res;
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    remote_addr.sin_port = htons(80);

    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        close(socket_fd);
        URLAPI_DEBUG("socket creat failed\n");
        res = ERROR_SOCKET_CREAT;
        return res;
    }
    if (connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) < 0)
    {
        close(socket_fd);
        URLAPI_DEBUG("socket connect 80 failed\n");
        res = ERROR_SOCKET_CONNECT;
        return res;
    }
    send(socket_fd, urlstring, strlen(urlstring), 0);
    close(socket_fd);
    return 0;
}
static int http_make_all_bypass(int scene_id)
{
	int res=0;
	if((scene_id != ALL_BYPASS)&&(scene_id != ALL_UNBYPASS)){

		URLAPI_DEBUG("default scene id error\n");
		res = ERROR_MDFY_NO_ID_DB;
		return res;
	}
////////////////////////////////////////////////////////////////////////////////////////////start
    //=========================================================================
    // 调用getendpoint.cgi，并获取返回结果

    int socket_fd;
    int len;
    struct sockaddr_in remote_addr;
    char buf[BUF_SIZE] = {0};
    char* recv_data = NULL;
    int recv_data_size = 0;

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    remote_addr.sin_port = htons(80);

    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        close(socket_fd);
        URLAPI_DEBUG("socket creat failed\n");
        res = ERROR_SOCKET_CREAT;
        return res;
    }

    if (connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) < 0)
    {
        close(socket_fd);
        URLAPI_DEBUG("socket connect 80 failed\n");
        res = ERROR_SOCKET_CONNECT;
        return res;
    }

    send(socket_fd, GetZBNode, strlen(GetZBNode), 0);

    while ((len = recv(socket_fd, buf, BUF_SIZE, 0)) > 0)
    {
        recv_data = (char*)realloc(recv_data, sizeof(char)*(recv_data_size + len + 1));
        memcpy(recv_data + recv_data_size, buf, len);
        recv_data_size += len;
        *(recv_data + recv_data_size) = '\0';
    }

    close(socket_fd);

    if (!recv_data)
    {
    	URLAPI_DEBUG("invoke getendpoint.cgi but received nothing! \n");
        res = ERROR_SOCKET_RECEIVED;
        return res;
    }
//    URLAPI_DEBUG("getendpoint.cgi received :%s\n",recv_data);

    //=========================================================================
    //按照安防类型调用API设置安防设备的布撤防状态

    cJSON* json_recv;
    cJSON* json_array;
    cJSON* json_node;
    cJSON* json_tmp;
    cJSON* json_devparam;

    int device_id;

    char* begin = strchr(recv_data, '{');
    char* end = strrchr(recv_data, '}');

    if (!begin || !end)
    {
        free(recv_data);
        URLAPI_DEBUG("json parse error \n");
        res = ERROR_JASON_PARSE;
        return res;
    }

    int json_len = end - begin + 1;
    char* json_str = (char*)calloc(sizeof(char), json_len + 1);   // 增加一个字节保存'\0'
    strncpy(json_str, begin, json_len);
    free(recv_data);
    recv_data = NULL;
//    URLAPI_DEBUG("getendpoint.cgi received json :%s\n",json_str);
    json_recv = cJSON_Parse(json_str);
    if (!json_recv)
    {
        free(json_str);
        cJSON_Delete(json_recv);
        URLAPI_DEBUG("json parse error \n");
        res = ERROR_JASON_PARSE;
        return res;
    }
    free(json_str);
    json_str = NULL;

    json_array = cJSON_GetObjectItem(json_recv, "response_params");
    if (!json_array)
    {
        cJSON_Delete(json_recv);
        URLAPI_DEBUG("json parse error \n");
        res = ERROR_JASON_PARSE;
        return res;
    }
    int array_size = cJSON_GetArraySize(json_array);
    if (array_size == 0)
    {
        cJSON_Delete(json_recv);
        URLAPI_DEBUG("json parse error \n");
        res = ERROR_JASON_PARSE;
        return res;
    }

    char* ieee = NULL;
    char *ep;
    char request[BUF_SIZE] = {0};
    int i = 0;
    for (i = 0; i < array_size; ++i)
    {
        // 对于其中某个设备操作失败直接忽略

        json_tmp = cJSON_GetArrayItem(json_array, i);
        if (!json_tmp)
        {
            continue;
        }

        json_node = cJSON_GetObjectItem(json_tmp, "device_id");  //get device_id
        if(!json_node)
		{
		   continue;
		}
        device_id = json_node ->valueint;
        if( (device_id != 0x400)&&(device_id != 0x401)&&(device_id != 0x402)&&(device_id != 0x403) ){

        	continue;
        }

        json_devparam = cJSON_GetObjectItem(json_tmp, "devparam");
        if (!json_devparam)
        {
            continue;
        }

        json_node = cJSON_GetObjectItem(json_devparam, "ep");
        if (!json_node)
        {
            continue;
        }
        ep = json_node ->valuestring;

        json_node = cJSON_GetObjectItem(json_devparam, "node");
        if (!json_node)
        {
            continue;
        }

        json_tmp = cJSON_GetObjectItem(json_node, "ieee");
        if (!json_tmp)
        {
            continue;
        }

        ieee = json_tmp ->valuestring;
        if(scene_id == ALL_BYPASS){

        	sprintf(request, "GET /cgi-bin/rest/network/localIASCIEByPassZone.cgi?zone_ieee=%s&zone_ep=%s&"
        	           "callback=1234&encodemethod=NONE&sign=AAA HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", ieee, ep);
        }
        else{

        	sprintf(request, "GET /cgi-bin/rest/network/localIASCIEUnByPassZone.cgi?zone_ieee=%s&zone_ep=%s&"
        	        	           "callback=1234&encodemethod=NONE&sign=AAA HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", ieee, ep);
        }
		URLAPI_DEBUG("API is %s\n",request);

        if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            close(socket_fd);
            continue;
        }

        if (connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) < 0)
        {
            close(socket_fd);
            continue;
        }
        send(socket_fd, request, strlen(request), 0);
        close(socket_fd);
    }
    //=========================================================================
    //调用API设置全局布撤防状态
    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) >0){

    	if (connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) >= 0){

    		if(scene_id == ALL_BYPASS) {																	//撤防
    			send(socket_fd, global_bypass, strlen(global_bypass), 0);//全局撤防
				#ifdef USE_RF_FUNCTION
    			communicateWithRF(rf_bypass, strlen(rf_bypass)+1, NULL); //所有rf设备全部撤防
				#endif
    			//printf("%s\n", rf_bypass);
    			//printf("%s\n", global_bypass);
    		}
    		else
    		{																								//布防
    			send(socket_fd, global_unbypass, strlen(global_unbypass), 0);
				#ifdef USE_RF_FUNCTION
    			communicateWithRF(rf_unbypass, strlen(rf_unbypass)+1, NULL);//所有rf设备全部布防
				#endif
    			//printf("%s\n", rf_unbypass);
    			//printf("%s\n", global_unbypass);
    		}

    	}
    }
	close(socket_fd);

    cJSON_Delete(json_recv);
    return res;
}
