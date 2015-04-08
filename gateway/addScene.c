/*
 *	File name   : addScene.c
 *  Created on  : Apr 2, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#include "unp.h"
#include <stdio.h>
#include <string.h>
#include "cgic.h"
#include "cJSON.h"
#include "sqliteOperator.h"
#include "glCalkProtocol.h"

//define
#define URL_PARA_NUM_ERROR	-1
#define URL_PARA_TYPE_ERROR		-2

#define URL_TYPE_DEV_BYPASS		1
#define URL_TYPE_ALL_BYPASS		2
#define	URL_TYPE_OUTLET			3

//
scene_action_stPtr scnaction_st_gener_malloc(const char *text);

#if 0
static void api_response(const scene_base_st *scene_base, const scene_action_stPtr action)
{
    cJSON *root;
    char *json_out;
    int i;

    root=cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "status", 0);
    cJSON_AddNumberToObject(root, "sid", scene_base->sid);
    cJSON_AddStringToObject(root, "scnname", scene_base->scnname);
    cJSON_AddStringToObject(root, "scnaction", scene_base->scnaction);
    cJSON_AddNumberToObject(root, "scnindex", scene_base->scnindex);
    //
    for(i=0; i<action[0].acttotal; i++){

        cJSON_AddStringToObject(root, "actobj", action[i].actobj);
        cJSON_AddStringToObject(root, "url", action[i].urlstring);
    }

    //
    json_out=cJSON_Print(root);
    cJSON_Delete(root);

    fprintf(cgiOut,"%s\n", json_out);
    free(json_out);
}
#else
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
#endif

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

/*
 * fuction     : gnerate_url_string
 * input       : type, para, para_cnt
 * output      : *url, *actobj
 * return      : success return url size,failed return ERROR FLAG
 * description : 如果解析出来的操作参数不能组成有效的url字符串，则返回错误。
 */
int gnerate_url_string(int type, const char* para, char para_cnt, char *url, char *actobj)
{
	int nwrite;
	char *p1, *p2, *p3;

	switch(type){

		case URL_TYPE_DEV_BYPASS:													//参数3个
			if(para_cnt != 3)
				return URL_PARA_NUM_ERROR;
			p1 = (char *)para;				//ieee string
			p2 = p1+ strlen(p1) +1;  //ep value			//why +1, becasue add '/0'
			p3 = p2+ strlen(p2) +1;	//0-bypass 1-unbypass

			if(atoi(p3)){
				nwrite = snprintf(url, URL_STRING_LEN, "http://127.0.0.1/cgi-bin/rest/network/"
						"localIASCIEUnByPassZone.cgi?zone_ieee=%s&zone_ep=%s&callback=1234&encodemethod=NONE&sign"
						"=AAA", p1,p2);
			}else{

				nwrite = snprintf(url, URL_STRING_LEN, "http://127.0.0.1/cgi-bin/rest/network/"
						"localIASCIEByPassZone.cgi?zone_ieee=%s&zone_ep=%s&callback=1234&encodemethod=NONE&sign"
						"=AAA", p1,p2);
			}
			memcpy(actobj, p1, 16);
			break;

		case URL_TYPE_ALL_BYPASS:

			if(para_cnt !=1 )
				return URL_PARA_NUM_ERROR;

			p1 = (char *)para;				//1-allbypass, 0-allunbypass
			int status = atoi(p1)+6;

			nwrite = snprintf(url, URL_STRING_LEN, "http://127.0.0.1/cgi-bin/rest/network/localIASCIEOp"
					"eration.cgi?operatortype=%d&param1=1&param2=2&param3=3&callback=1234&encodemethod=NONE&sign=AAA", status);//7-allbypass,6-allunbypass

			actobj= NULL;
			break;

		case URL_TYPE_OUTLET:

			if(para_cnt != 3)
				return URL_PARA_NUM_ERROR;
			p1 = (char *)para;				//ieee string
			p2 = p1+ strlen(p1) +1;  //ep value			//why +1, becasue add '/0'
			p3 = p2+ strlen(p2) +1;	//0-open, 1-close, 2-invert
			//remapping operatortype
			int opt = atoi(p3);
			if(opt ==0){
				opt = 1;
			}
			else if(opt == 1){
				opt = 0;
			}
			else{
				opt = 2;
			}

			nwrite = snprintf(url, URL_STRING_LEN, "http://127.0.0.1/cgi-bin/rest/network/"
					"mainsOutLetOperation.cgi?ieee=%s&ep=%s&operatortype=%d&"
					"param1=1&param2=2&param3=3&callback=1234&encodemethod=NONE&sign=AAA", p1, p2, opt);

			memcpy(actobj, p1, 16);
			break;

		default:
			return URL_PARA_TYPE_ERROR;
			break;
	}
    nwrite = nwrite+1; // 加上结束符'\0'
//   printf("urlstring:%s\n",url);
    return nwrite;
}
int cgiMain()
{
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;

	scene_base_st scene_base;
	scene_action_stPtr scene_action;

	cgiFormResultType cgi_re;
	int string_len=100;
	int res;

	cgiHeaderContentType("application/json"); //MIME

	//read name
	cgi_re = cgiFormString("scnname", scene_base.scnname, NAME_STRING_LEN + 1);

	//read action para
	cgi_re = cgiFormString("scnaction", scene_base.scnaction, string_len+1);

	//read index
	cgi_re = cgiFormInteger("scnindex", &scene_base.scnindex, 0);

	//generate scene_action_st
	scene_action = scnaction_st_gener_malloc(scene_base.scnaction);							//malloc1
	if(scene_action == NULL){
		res = -6;
		goto all_over;
	}

	//wrtie database
	res = add_scene_db(&scene_base, scene_action);
	if(res<0){
		goto all_over;
	}

all_over:
    //all right
#if 0
    api_response(&scene_base, scene_action);
#else
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
	}
#endif
    if(scene_action){
    	free(scene_action);																		//free1
    }
	return 0;
}
/*
 * fuction     : scnaction_st_gener_malloc
 * input       : text---is scene_base.scnaction
 * output      :
 * return      : ok--scene_action_stPtr[];failed--NULL
 * description : atter using this scene_action_stPtr[] of return value must be free() this struct*
 */
scene_action_stPtr scnaction_st_gener_malloc(const char *text)
{
    int copy_len;
    char optCnt=0, paraCnt=0;
    int i=0; //分割次数
    char *copy, *copy_ptr;

    char *actpara, *acttype, *para;
    scene_action_stPtr action;

    copy_len = strlen(text) + 1;
    if (!(copy = (char*)malloc(copy_len))) return NULL;				//malloc 1
    memcpy(copy, text, copy_len);
    //strsep()被分割字串要被改变，所以不能操作存放在静态存储区的字串常量//strsep之后指向的字符串里分隔符会变成"/0"
    copy_ptr = copy;

    /***********************************************************/  									//分割并组url串
    while (strsep(&copy_ptr, "@")){									   		//分割'@'
    	optCnt++;
    }
    if(optCnt >0){															//malloc 2
    	action = (scene_action_stPtr)malloc(optCnt*(sizeof(scene_action_st)));		//free在外层释放
    	if(!action){
    		free(copy);
    		return NULL;
    	}
    }
    else{
    	free(copy);
    	return NULL;
    }

    for (copy_ptr = copy; copy_ptr< (copy + copy_len);) {

    	actpara = acttype = copy_ptr;
        for (copy_ptr += strlen(copy_ptr); copy_ptr < (copy +copy_len) && !*copy_ptr; copy_ptr++);
        acttype = strsep(&actpara,":");							//分割':', only once

        paraCnt =0;
        para =actpara;
//        paralen = strlen(actpara);
        while (strsep(&para, "-")){								//分割'-'
        	paraCnt++;
        }

        if(i> optCnt){											//组串的次数大于操作总数会出错
        	free(action);
        	free(copy);
        	return NULL;
        }
        if(gnerate_url_string(atoi(acttype), actpara, paraCnt, action[i].urlstring, action[i].actobj) <0){
        	URLAPI_DEBUG("gnerate url faied!!!!!!!!!!!!!!!!!!!!!!!\n");
        	free(action);
        	free(copy);
        	return NULL;//根据操作类型和操作参数组串
        }
        i++;
    }
    if(i!= optCnt){												//组串的次数和操作总数相同才对
    	free(action);
    	free(copy);
    	return NULL;
    }

    for(i=0; i<optCnt; i++){
    	action[i].acttotal = optCnt;
    }

    free(copy);													//free 1
    return action;
}







