/*
 *	File name   : addScene.c
 *  Created on  : Apr 2, 2015
 *  Author      : yanly
 *  Description :
 *  Version     : v1.0
 *  History     : <author>		<time>			<version>		<desc>
 *                yanly			2015-04-07		v1.00			addScene
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cgic.h"
#include "cJSON.h"
#include "sqliteOperator.h"
#include "glCalkProtocol.h"
#include <time.h>


extern scene_action_stPtr scnaction_st_gener_malloc(const char *text);

static void api_response(const scene_base_st *scene_base)
{
    cJSON *root;
    char *json_out;

    root=cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", 0);
    cJSON_AddNumberToObject(root, "sid", scene_base->sid);
    cJSON_AddStringToObject(root, "scnname", scene_base->scnname);
    cJSON_AddStringToObject(root, "scnaction", scene_base->scnaction);
    cJSON_AddNumberToObject(root, "scnindex", scene_base->scnindex);
    json_out=cJSON_Print(root);
    cJSON_Delete(root);

    fprintf(cgiOut,"%s\n", json_out);
    free(json_out);
}

static void api_error_res(int status)
{
    cJSON *root;
    char *json_out;

    root=cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", status);

    json_out=cJSON_Print(root);
    cJSON_Delete(root);

    fprintf(cgiOut,"%s\n", json_out);
    free(json_out);
}


int cgiMain()
{
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;

	scene_base_st scene_base;
	scene_action_stPtr scene_action;

	cgiFormResultType cgi_re;
	int res;

	cgiHeaderContentType("application/json"); //MIME
	char *ipaddr = getenv("REMOTE_ADDR");
	URLAPI_DEBUG("remote ip=%s\n",ipaddr);

	cgi_re = cgiFormString("scnname", scene_base.scnname, NAME_STRING_LEN + 1);
	cgi_re = cgiFormString("scnaction", scene_base.scnaction, SCENEACTION_MAX_LEN+1);
	cgi_re = cgiFormInteger("scnindex", &scene_base.scnindex, 0);

	res = db_init();
	if(res<0){
		goto all_over;
	}
	//generate scene_action_st
	scene_action = scnaction_st_gener_malloc(scene_base.scnaction);							//malloc1
	if(scene_action == NULL){
		res = ERROR_GENERATE_URL_STRING;
		goto all_over;
	}

	//wrtie database
	res = add_scene_db(&scene_base, scene_action);
	if(res<0){
		goto all_over;
	}

all_over:
    db_close();
    if(res<0){
    	scene_base_st *p;
    	p = &scene_base;
    	p = NULL;
    	api_error_res(res);
    }
    else{
    	api_response(&scene_base);
    }

    //push to cb_daemon
	if((send_cb_len = cJsonScene_callback(send_cb_string, &scene_base, NULL, NULL, 1, res)) >=0) {
		//if push failed, not handle temporary
		push_to_CBDaemon(send_cb_string, send_cb_len);
		URLAPI_DEBUG("[time]=%ld,callback=%s\n", time(NULL), send_cb_string);
	}

    if(scene_action){
    	free(scene_action);																		//free1
    }
	return 0;
}


