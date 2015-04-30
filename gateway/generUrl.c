/*
 *	File name   : generUrl.c
 *  Created on  : Apr 8, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "smartgateway.h"

/***************************************************************************************/
//define
#define URL_PARA_NUM_ERROR	-1
#define URL_PARA_TYPE_ERROR		-2

#define URL_TYPE_DEV_BYPASS		1
#define URL_TYPE_ALL_BYPASS		2
#define	URL_TYPE_OUTLET			3

/***************************************************************************************/
/*
 * fuction     : gnerate_url_string
 * input       : type, para, para_cnt
 * output      : *url, *actobj
 * return      : success return url size,failed return ERROR FLAG
 * description : 如果解析出来的操作参数不能组成有效的url字符串，则返回错误。
 */
#if 0
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
#else


#define OPT_ENABLE				1
#define OPT_DISABLE				0
#define OPT_INVERTSOCKET		2
#define OPT_OPENSOCKET			OPT_ENABLE
#define OPT_CLOSESOCKET			OPT_DISABLE
#define OPT_UNBYPASS			OPT_ENABLE
#define OPT_BYPASS				OPT_DISABLE
#define OPT_ALLUNBYPASS			OPT_ENABLE
#define OPT_ALLBYPASS			OPT_DISABLE
int gnerate_url_string(int type, const char* para, char para_cnt, char *url, char *actobj)
{
	int nwrite;
	char *p1, *p2, *p3;
	int status;

	switch(type){

		case URL_TYPE_DEV_BYPASS:													//参数3个
			if(para_cnt != 3)
				return URL_PARA_NUM_ERROR;
			p1 = (char *)para;				//ieee string
			p2 = p1+ strlen(p1) +1;  //ep value			//why +1, becasue add '/0'
			p3 = p2+ strlen(p2) +1;	//0-bypass 1-unbypass

			if(atoi(p3) == OPT_UNBYPASS){
				nwrite = snprintf(url, URL_STRING_LEN, "GET /cgi-bin/rest/network/"
						"localIASCIEUnByPassZone.cgi?zone_ieee=%s&zone_ep=%s&callback=1234&encodemethod=NONE&sign"
						"=AAA HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", p1,p2);
			}else if(atoi(p3) == OPT_BYPASS){

				nwrite = snprintf(url, URL_STRING_LEN, "GET /cgi-bin/rest/network/"
						"localIASCIEByPassZone.cgi?zone_ieee=%s&zone_ep=%s&callback=1234&encodemethod=NONE&sign"
						"=AAA HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", p1,p2);
			}
			else
				return URL_PARA_NUM_ERROR;
//			memcpy(actobj, p1, 16);
			snprintf(actobj, IEEE_LEN+1, p1);
			break;

		case URL_TYPE_ALL_BYPASS:

			if(para_cnt !=3 )
				return URL_PARA_NUM_ERROR;

			p1 = (char *)para;				//ieee string
			p2 = p1+ strlen(p1) +1;  //ep value			//why +1, becasue add '/0'
			p3 = p2+ strlen(p2) +1;	//0-allbypass, 1-allunbypass

			if(atoi(p3) == OPT_ALLUNBYPASS)
				status = 7;
			else if(atoi(p3) == OPT_ALLBYPASS)
				status = 6;
			else
				return URL_PARA_NUM_ERROR;

			nwrite = snprintf(url, URL_STRING_LEN, "GET /cgi-bin/rest/network/localIASCIEOp"
					"eration.cgi?operatortype=%d&param1=1&param2=2&param3=3&callback=1234&encodemethod=NONE&sign=AAA HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", status);//7-allbypass,6-allunbypass

			snprintf(actobj, IEEE_LEN+1, p1);
			break;

		case URL_TYPE_OUTLET:

			if(para_cnt != 3)
				return URL_PARA_NUM_ERROR;
			p1 = (char *)para;				//ieee string
			p2 = p1+ strlen(p1) +1;  //ep value			//why +1, becasue add '/0'
			p3 = p2+ strlen(p2) +1;
			//remapping operatortype
			int opt = atoi(p3);

			if(opt == OPT_OPENSOCKET){
				opt = 1;
			}
			else if(opt == OPT_CLOSESOCKET){
				opt = 0;
			}
			else if(opt == OPT_INVERTSOCKET){
				opt = 2;
			}
			else
				return URL_PARA_NUM_ERROR;

			nwrite = snprintf(url, URL_STRING_LEN, "GET /cgi-bin/rest/network/"
					"mainsOutLetOperation.cgi?ieee=%s&ep=%s&operatortype=%d&"
					"param1=1&param2=2&param3=3&callback=1234&encodemethod=NONE&sign=AAA HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", p1, p2, opt);

//			memcpy(actobj, p1, 16);
			snprintf(actobj, IEEE_LEN+1, p1);
			break;

		default:
			return URL_PARA_TYPE_ERROR;
			break;
	}
    nwrite = nwrite+1; // 加上结束符'\0'
//   printf("urlstring:%s\n",url);
    return nwrite;
}
#endif

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

    	//add150429
    	snprintf(action[i].actpara, ACTPARA_LEN+1, copy_ptr);
    	//
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
/*
 * fuction     : gnerate_per_urlobj
 * input       : 1:00137A00000121EF-01-0
 * output      : url string and action object
 * return      :
 * description :
 */
int gnerate_per_urlobj(url_act_st *object, const char *text)
{
    int copy_len;
    char paraCnt=0;
    char *copy, *copy_ptr;
    char *actpara, *acttype;
//    url_act_st isinstance;

    copy_len = strlen(text) + 1;
    if (!(copy = (char*)malloc(copy_len))) return ERROR_OTHER;				//malloc 1
    memcpy(copy, text, copy_len);
    //strsep()被分割字串要被改变，所以不能操作存放在静态存储区的字串常量//strsep之后指向的字符串里分隔符会变成"/0"
    copy_ptr = copy;
    /***********************************************************/  									//分割并组url串
	actpara = acttype = copy_ptr;
	acttype = strsep(&copy_ptr,":");							//分割':', only once
	paraCnt =0;
	actpara = copy_ptr;
//	printf("%s, %s\n", acttype, actpara);
	while (strsep(&copy_ptr, "-")){							//分割'-'
		paraCnt++;
	}
//	printf("%d, %d,%s\n", paraCnt, atoi(acttype),actpara);
	if(gnerate_url_string(atoi(acttype), actpara, paraCnt, object->urlstring, object->actobj) <0){
		URLAPI_DEBUG("gnerate url faied!\n");
		free(copy);
		return ERROR_GENERATE_URL_STRING;//根据操作类型和操作参数组串
	}
//	printf("%s, %s\n", isinstance.urlstring, isinstance.actobj);
//	strcpy(object->urlstring, isinstance.urlstring);
//	strcpy(object->actobj, isinstance.actobj);
//all over:
	if(copy)
		free(copy);													//free 1
    return 0;

}
int gnerate_linkage_trgcnd_st(linkage_trgcnd_st *condition_st, const char *text)
{
    int copy_len;
    char paraCnt=0;
    char *copy, *copy_ptr;
    char *p1, *p2, *p3;

    copy_len = strlen(text) + 1;
    if (!(copy = (char*)malloc(copy_len))) return ERROR_OTHER;				//malloc 1
    memcpy(copy, text, copy_len);
    copy_ptr = copy;
    /***********************************************************/
	while (strsep(&copy_ptr, "@")){							//分割'@'
		paraCnt++;
	}
	if(paraCnt !=3) {
		free(copy);
		return ERROR_PARAMETER_INVALID;
	}

	p1 = (char *)copy;
	p2 = p1+ strlen(p1) +1;  //why +1, becasue add '/0'
	p3 = p2+ strlen(p2) +1;
	snprintf(condition_st->attribute, ATTRIBUTE_LEN+1, p1);
	snprintf(condition_st->operator, REL_OPERATOR_LEN+1, p2);
	condition_st->value = atoi(p3);

//free
	if(copy)
		free(copy);													//free 1
    return 0;
}


