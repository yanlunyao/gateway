/*
 * glCalkProtocol.c
 *
 *  Created on: Mar 12, 2015
 *      Author: yanly
 */
#include "unp.h"
#include <string.h>
#include <stdlib.h>
//#include "smartgateway.h"
#include "cJSON.h"
#include "glCalkProtocol.h"


/*
* Function: push_to_CBDaemon
* Description:
* Input: send
* Output: none
* Return: 0>>push success;
*	  	  -1>>push error,socket send error
* Others:  none
*/
int push_to_CBDaemon(const char *send_text, int send_size)
{
	int fd;
	struct sockaddr_in	servaddr;

	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        URLAPI_DEBUG("socket error\n");
		return -1;
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(FEATURE_GDGL_CPROXY_CB_PUSH_PORT);

	if ( (inet_pton(AF_INET, LOCALHOST_TEST, &servaddr.sin_addr)) <= 0) {
		URLAPI_DEBUG("inet_pton error\n");
		close(fd);
		return -2;
	}

	if ( connect(fd, (SA *) &servaddr, sizeof(servaddr)) < 0 ) {
		URLAPI_DEBUG("connect error\n");
		close(fd);
		return -1;
	}
	//sleep(1); //debug yan
    if ( writen(fd, send_text, send_size) != send_size ) {
    	URLAPI_DEBUG("write error\n");
		close(fd);
		return -1;
    }

    //sleep(3); //debug yan
	close(fd);

	return 0;
}
/*
 * Function: cJsonAlias_callback
 * return: success>>string size,error>>error flag
 * description: none
*/
int cJsonAlias_callback(char *text, int status_value)
{
    cJSON *root;
    char *out;
    int nwrite;

	root = cJSON_CreateObject();
	if (!root) {
		URLAPI_DEBUG("create cjson failed\n");
		return -1;
	}
	cJSON_AddNumberToObject(root, MSGTYPE_STRING, 0);
	cJSON_AddNumberToObject(root, GL_MAINID, GL_MAINID_USER_MANAGE);
	cJSON_AddNumberToObject(root, GL_SUBID, GL_SUBID_M_ALIAS);
	cJSON_AddNumberToObject(root, GL_STAUTS, status_value);

    if((out = cJSON_PrintUnformatted(root)) == 0 ){
    	URLAPI_DEBUG("print cjson failed\n");
		cJSON_Delete(root);
		return -1;
    }

    nwrite = snprintf(text, GL_CALLBACK_MAX_SIZE, "%s", out);
    nwrite = nwrite+1; // 加上结束符'\0'

    cJSON_Delete(root);
	free(out);;
    return nwrite;
}
/*
 * Function: cJsonPsw_callback
 * return: success>>string size,error>>error flag
 * description: none
*/
int cJsonPsw_callback(char *text, int status_value)
{
    cJSON *root;
    char *out;
    int nwrite;

	root = cJSON_CreateObject();
	if (!root) {
		URLAPI_DEBUG("create cjson failed\n");
		return -1;
	}
	cJSON_AddNumberToObject(root, MSGTYPE_STRING, 0);
	cJSON_AddNumberToObject(root, GL_MAINID, GL_MAINID_USER_MANAGE);
	cJSON_AddNumberToObject(root, GL_SUBID, GL_SUBID_M_PSW);
	cJSON_AddNumberToObject(root, GL_STAUTS, status_value);

    if((out = cJSON_PrintUnformatted(root)) == 0 ){
    	URLAPI_DEBUG("print cjson failed\n");
		cJSON_Delete(root);
		return -1;
    }
    nwrite = snprintf(text, GL_CALLBACK_MAX_SIZE, "%s", out);
    nwrite = nwrite+1; // 加上结束符'\0'

    cJSON_Delete(root);
	free(out);;
    return nwrite;
}
/*
 * Function: cJsonScene_callback
 * return: success>>string size,error>>error flag
 * description: this callback funciton just for addScene, editScene and modifySceneIndex
*/
int cJsonScene_callback(char *text, const scene_base_st *scene, const char *sindexall, const char *sidall, int subid, int status_value)
{
    cJSON *root;
    char *out;
    int nwrite;

	root = cJSON_CreateObject();
	if (!root) {
		URLAPI_DEBUG("create cjson failed\n");
		return -1;
	}

	cJSON_AddNumberToObject(root, MSGTYPE_STRING, GL_MSGTYPE_VALUE);
	cJSON_AddNumberToObject(root, GL_MAINID, GL_MAINID_SCENE);
	cJSON_AddNumberToObject(root, GL_SUBID, subid);
	cJSON_AddNumberToObject(root, "status", status_value);

	if(status_value >=0){

		if(scene!=NULL){
			cJSON_AddNumberToObject(root, "sid", scene->sid);
			cJSON_AddStringToObject(root, "scnname", scene->scnname);
			cJSON_AddStringToObject(root, "scnaction", scene->scnaction);
			cJSON_AddNumberToObject(root, "scnindex", scene->scnindex);
		}
		if((sindexall!=NULL)&&(sidall!=NULL)){
			cJSON_AddStringToObject(root, "sidall", sidall);
			cJSON_AddStringToObject(root, "scnindexall", sindexall);
		}
	}
    if((out = cJSON_PrintUnformatted(root)) == 0 ){
    	URLAPI_DEBUG("print cjson failed\n");
		cJSON_Delete(root);
		return -1;
    }

    nwrite = snprintf(text, GL_CALLBACK_MAX_SIZE, "%s", out);
    nwrite = nwrite+1; // 加上结束符'\0'

    cJSON_Delete(root);
	free(out);;
    return nwrite;

}
int cJsonDelDoScene_callback(char *text, int sid, int subid, int res)
{
    cJSON *root;
    char *out;
    int nwrite;

	root = cJSON_CreateObject();
	if (!root) {
		URLAPI_DEBUG("create cjson failed\n");
		return -1;
	}

	cJSON_AddNumberToObject(root, MSGTYPE_STRING, GL_MSGTYPE_VALUE);
	cJSON_AddNumberToObject(root, GL_MAINID, GL_MAINID_SCENE);
	cJSON_AddNumberToObject(root, GL_SUBID, subid);
	cJSON_AddNumberToObject(root, "status", res);

	if(res >=0){
		cJSON_AddNumberToObject(root, "sid", sid);
	}

    if((out = cJSON_PrintUnformatted(root)) == 0 ){
    	URLAPI_DEBUG("print cjson failed\n");
		cJSON_Delete(root);
		return -1;
    }

    nwrite = snprintf(text, GL_CALLBACK_MAX_SIZE, "%s", out);
    nwrite = nwrite+1; // 加上结束符'\0'

    cJSON_Delete(root);
	free(out);;
    return nwrite;

}
