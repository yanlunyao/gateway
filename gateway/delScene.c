/*
 *	File name   : delScene.c
 *  Created on  : Apr 2, 2015
 *  Author      : yanly
 *  Description :
 *  Version     : v1.0
 *  History     : <author>		<time>			<version>		<desc>
 *                yanly			2015-04-07		v1.00			delScene
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cgic.h"
#include "cJSON.h"
#include "sqliteOperator.h"
#include "glCalkProtocol.h"

//#define NO_CALLBAK_DEBUG
#include <time.h>

#define SCENE_DEBUG(fmt, args...)	fprintf( fopen("/gl/log/api.log","a+"), "%s(%d)[%s]: " fmt, __FILE__, __LINE__, __func__, ## args)

static void api_response(int res, int sid)  //if res<0, we don't need to care about 'sid'
{
    cJSON *root;
    char *json_out;

    root=cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", res);

    if(res >=0)
    	cJSON_AddNumberToObject(root, "sid", sid);

    json_out=cJSON_Print(root);
    cJSON_Delete(root);

    fprintf(cgiOut,"%s\n", json_out);
    free(json_out);
}

int cgiMain()
{
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;

	cgiFormResultType cgi_re;
	int res;
	int sid;

	cgiHeaderContentType("application/json"); //MIME
	char *ipaddr = getenv("REMOTE_ADDR");
	SCENE_DEBUG("remote ip=%s\n",ipaddr);

	//read sid
	cgi_re = cgiFormInteger("sid", &sid, 0);

	res = db_init();
	if(res<0){
		goto all_over;
	}
	//delete database
	res = del_scene_db(sid);
all_over:
	db_close();
	//respond
    api_response(res, sid);

    //push to cb_daemon
#ifdef NO_CALLBAK_DEBUG
#else
	if((send_cb_len = cJsonDelDoScene_callback(send_cb_string, sid, DEL_SUBID_SCENE, res)) >=0) {
		push_to_CBDaemon(send_cb_string, send_cb_len);
		SCENE_DEBUG("[time]=%ld,callback=%s\n", time(NULL), send_cb_string);
	}
#endif
	return 0;
}
