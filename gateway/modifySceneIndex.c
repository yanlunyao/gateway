/*
 *	File name   : modifySceneIndex.c
 *  Created on  : Apr 2, 2015
 *  Author      : yanly
 *  Description :
 *  Version     : v1.0
 *  History     : <author>		<time>			<version>		<desc>
 *                yanly			2015-04-07		v1.00			modifySceneIndex
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cgic.h"
#include "cJSON.h"
#include "sqliteOperator.h"
#include "glCalkProtocol.h"

static void api_response(int res, const char *idall, const char *indexall)
{
    cJSON *root;
    char *json_out;

    root=cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", res);

    if(res >=0){
    	cJSON_AddStringToObject(root, "sidall", idall);
    	cJSON_AddStringToObject(root, "indexall", indexall);
    }

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

	char idall[100];
	char indexall[100];
	char id_org[100], index_org[100];

	cgiHeaderContentType("application/json"); //MIME

	//read id all
	cgi_re = cgiFormString("sidall", id_org, 100);
	strcpy(idall, id_org);

	//read index all
	cgi_re = cgiFormString("scnindexall", index_org, 100);
	strcpy(indexall, index_org);

	res = db_init();
	if(res<0){
		goto all_over;
	}
	//strsep idall and indexall
	char *p_id, *p_index;
	int idcnt=0, indexcnt=0;
	p_id = idall; p_index = indexall;

	int pp_id[100], pp_indx[100];
	char *temp;
    while ((temp =strsep(&p_id, "-"))){									   		//分割'-'
    	pp_id[idcnt] = atoi(temp);
    	idcnt++;
    }
    while ((temp = strsep(&p_index, "-"))){									   		//分割'-'
    	pp_indx[indexcnt] = atoi(temp);
    	indexcnt++;
    }
    if(idcnt!=indexcnt){
    	res = ERROR_PARAMETER_INVALID;
    	goto all_over;
    }
//    int i;
//    for(i=0; i<idcnt; i++){
//    	URLAPI_DEBUG("id:%d,index:%d\n",pp_id[i], pp_indx[i]);
//    }
//	modify scene index in database
	res = modify_scene_index(pp_id, pp_indx, idcnt);
	if(res<0){
		goto all_over;
	}

all_over:
	db_close();
    api_response(res, id_org, index_org);
    //push to cb_daemon
#ifdef NO_CALLBAK_DEBUG
#else
	if((send_cb_len = cJsonScene_callback(send_cb_string, NULL, index_org, id_org, MODIFYIDX_SUBID_SCENE, res)) >=0) {
		push_to_CBDaemon(send_cb_string, send_cb_len);
	}
#endif
	return 0;
}
