/*
 *	File name   : getSceneList.c
 *  Created on  : Apr 8, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cgic.h"
#include "cJSON.h"
#include "sqliteOperator.h"
#include "glCalkProtocol.h"

//#define ERROR_NO_LIST					-8

static void api_response(int res, scene_base_list_st *list)
{
    cJSON *root;
    cJSON *list_array;
    cJSON *list_item;
    char *json_out;
    int i;

    root=cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", res);

    if(res >=0){

    	cJSON_AddItemToObject(root, "list", list_array =cJSON_CreateArray());
    	for(i=0; i< list->list_total; i++){
    		cJSON_AddItemToArray(list_array, list_item =cJSON_CreateObject());
    		cJSON_AddNumberToObject(list_item, "sid", list->scene_base[i].sid);
    		cJSON_AddStringToObject(list_item, "scnname", list->scene_base[i].scnname);
    		cJSON_AddNumberToObject(list_item, "scnindex", list->scene_base[i].scnindex);
    		cJSON_AddStringToObject(list_item, "scnaction", list->scene_base[i].scnaction);
    	}
    }

    json_out=cJSON_Print(root);
    cJSON_Delete(root);

    fprintf(cgiOut,"%s\n", json_out);
    free(json_out);
}

int cgiMain()
{
	int res;
	scene_base_list_st scene_base_list;
	/********************************************/  //init
	scene_base_stPtr *p;
	p = &scene_base_list.scene_base;
	p = NULL;
	scene_base_list.list_total = 0;
	/********************************************/

	cgiHeaderContentType("application/json"); //MIME

	res = db_init();
	if(res<0){
		goto all_over;
	}
	//delete database
	res = get_scene_db(&scene_base_list);
all_over:
	db_close();
	//respond
    api_response(res, &scene_base_list);

    //free
//    URLAPI_DEBUG("free list\n");
    if(scene_base_list.scene_base !=NULL){
    	URLAPI_DEBUG("free list\n");
    	free(scene_base_list.scene_base);
    }
	return 0;
}
