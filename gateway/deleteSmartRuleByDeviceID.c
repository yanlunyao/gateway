/*
 *	File name   : deleteSmartRuleByDeviceID.c
 *  Created on  : Aug 24, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

//API: http://192.168.1.162/cgi-bin/rest/network/DeleteSmartRuleByDeviceID.cgi?id=123456
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "cgic.h"
#include "cJSON.h"
#include "sqliteOperator.h"
#include "glCalkProtocol.h"
#include "callbackProtocolField.h"

#define DEL_ID_MAX	100 //删除设备同时允许影响最多的id数目
int cgiMain()
{
	char isdeleted_ieee[IEEE_LEN+1] = {0};
	int will_del_id[DEL_ID_MAX] = {0};
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;
	int i;
	int res = 0;

	cgiFormResultType cgi_re;

	cgiHeaderContentType("application/json"); //MIME
	cgi_re = cgiFormString("id", isdeleted_ieee, IEEE_LEN + 1);
	isdeleted_ieee[IEEE_LEN] = '\0';

	db_init();
	//判断是否在定时规则里
	if(del_timeaction_by_isdeletedieee(will_del_id, isdeleted_ieee) >0) {
		for(i=0; i<DEL_ID_MAX; i++) {
			if(will_del_id[i] == 0)
				break;
			URLAPI_DEBUG("[devBeDeleted] -tid=%d\n",will_del_id[i]);
			if((send_cb_len = cJsonTimeAction_callback(send_cb_string, SUBID_DEL_TA, 1, will_del_id[i], NULL)) >=0) {
				push_to_CBDaemon(send_cb_string, send_cb_len);
				usleep(300000); //300ms //发送太快调试工具接收不到
				//GDGL_DEBUG("send tima success\n");
			}
		}
	}
	memset(will_del_id, 0, DEL_ID_MAX);
	//判断是否在场景规则里
	if(del_scene_by_isdeletedieee(will_del_id, isdeleted_ieee) >0) {
		scene_base_st base_buf;
		for(i=0; i<DEL_ID_MAX; i++) {
			if(will_del_id[i] == 0)
				break;
			//GDGL_DEBUG("will changed sid:%d\n",will_del_id[i]);
			//printf("[devBeDeleted] -sid=%d\n",will_del_id[i]);
			res = read_t_scene_base_byid(&base_buf, will_del_id[i]);
			if(res <0) {
				if(res == ERROR_MDFY_NO_ID_DB) //如果没有这条id规则了，代表这条规则已被删除，发送删除的callback
				{
					URLAPI_DEBUG("[devBeDeleted] -sid=%d\n",will_del_id[i]);
					if((send_cb_len = cJsonDelDoScene_callback(send_cb_string, will_del_id[i], SUBID_DEL_SCENE, 1)) >=0) {
						push_to_CBDaemon(send_cb_string, send_cb_len);
						usleep(300000); //300ms //发送太快调试工具接收不到
					}

				}
				continue;
			}
			URLAPI_DEBUG("[devBeDeleted] edit sid=%d\n",will_del_id[i]);
			if((send_cb_len = cJsonScene_callback(send_cb_string, &base_buf, NULL, NULL, SUBID_EDIT_SCENE, 1)) >=0) {
				push_to_CBDaemon(send_cb_string, send_cb_len);
				usleep(300000); //300ms //发送太快调试工具接收不到
			}
		}
	}
	memset(will_del_id, 0, DEL_ID_MAX);
	//判断是否在联动规则里
	if(del_linkage_by_isdeletedieee(will_del_id, isdeleted_ieee) >0) {
		for(i=0; i<DEL_ID_MAX; i++) {
			if(will_del_id[i] == 0)
				break;
			URLAPI_DEBUG("[devBeDeleted] -lid=%d\n",will_del_id[i]);
			if((send_cb_len = cJsonLinkage_callback(send_cb_string, SUBID_DEL_LINK, 1, will_del_id[i], NULL)) >=0) {
				push_to_CBDaemon(send_cb_string, send_cb_len);
				usleep(300000); //300ms //发送太快调试工具接收不到
			}
		}
	}
	db_close();
	fprintf(cgiOut,"{\n	\"status\": %d\n}\n", 0);
	return 0;
}
