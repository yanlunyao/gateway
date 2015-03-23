/*
 * glCalkProtocol.c
 *
 *  Created on: Mar 12, 2015
 *      Author: yanly
 */

#include <string.h>
#include <stdlib.h>

#include "cJSON.h"
#include "glCalkProtocol.h"
#include "smartgateway.h"

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
		GDGL_DEBUG("create cjson failed\n");
		return -1;
	}
	cJSON_AddNumberToObject(root, MSGTYPE_STRING, 0);
	cJSON_AddNumberToObject(root, GL_MAINID, GL_MAINID_USER_MANAGE);
	cJSON_AddNumberToObject(root, GL_SUBID, GL_SUBID_M_ALIAS);
	cJSON_AddNumberToObject(root, GL_STAUTS, status_value);

    if((out = cJSON_PrintUnformatted(root)) == 0 ){
    	GDGL_DEBUG("print cjson failed\n");
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
		GDGL_DEBUG("create cjson failed\n");
		return -1;
	}
	cJSON_AddNumberToObject(root, MSGTYPE_STRING, 0);
	cJSON_AddNumberToObject(root, GL_MAINID, GL_MAINID_USER_MANAGE);
	cJSON_AddNumberToObject(root, GL_SUBID, GL_SUBID_M_PSW);
	cJSON_AddNumberToObject(root, GL_STAUTS, status_value);

    if((out = cJSON_PrintUnformatted(root)) == 0 ){
    	GDGL_DEBUG("print cjson failed\n");
		cJSON_Delete(root);
		return -1;
    }
    nwrite = snprintf(text, GL_CALLBACK_MAX_SIZE, "%s", out);
    nwrite = nwrite+1; // 加上结束符'\0'

    cJSON_Delete(root);
	free(out);;
    return nwrite;
}

