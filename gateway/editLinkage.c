/*
 *	File name   : editLinkage.c
 *  Created on  : Apr 24, 2015
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
#include "callbackProtocolField.h"


static void api_response(int res, int id_value ,const linkage_base_st *link_base);
extern int gnerate_linkage_trgcnd_st(linkage_trgcnd_st *condition_st, const char *text);
extern int gnerate_per_urlobj(url_act_st *object, const char *text);

int cgiMain()
{
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;

	linkage_st lnk_st;
	int enable_flag;

	cgiFormResultType cgi_re;
	int res=0;

	cgiHeaderContentType("application/json"); //MIME

	cgi_re = cgiFormInteger(FIELD_LID, &lnk_st.lnk_base.lid, 0);
	cgi_re = cgiFormString(FIELD_LNKNAME, lnk_st.lnk_base.lnkname, NAME_STRING_LEN + 1);
	lnk_st.lnk_base.lnkname[NAME_STRING_LEN] = '\0';
	cgi_re = cgiFormString(FIELD_TRGIEEE, lnk_st.lnk_base.trgieee, IEEE_LEN + 1);
	lnk_st.lnk_base.trgieee[IEEE_LEN] = '\0';
	cgi_re = cgiFormString(FIELD_TRGEP, lnk_st.lnk_base.trgep, 2 + 1);
	lnk_st.lnk_base.trgep[2] = '\0';
	cgi_re = cgiFormString(FIELD_TRGCND, lnk_st.lnk_base.trgcnd, TRIGGER_CONDITION_LEN + 1);
	lnk_st.lnk_base.trgcnd[TRIGGER_CONDITION_LEN] = '\0';
	cgi_re = cgiFormString(FIELD_LNKACT, lnk_st.lnk_base.lnkact, ACTPARA_LEN + 1);
	lnk_st.lnk_base.lnkact[ACTPARA_LEN] = '\0';
	cgi_re = cgiFormInteger(FIELD_ENABLE, &enable_flag, 0);
	lnk_st.lnk_base.enable = (char)enable_flag;

	res = gnerate_per_urlobj(&lnk_st.urlobject, lnk_st.lnk_base.lnkact);
	if(res <0){
		goto all_over;
	}
	res = gnerate_linkage_trgcnd_st(&lnk_st.lnk_condition, lnk_st.lnk_base.trgcnd);
	if(res <0){
		goto all_over;
	}
//	wrtie database
	res = db_init();
	if(res<0){
		goto all_over;
	}
	res = edit_linkage_db(&lnk_st);
	if(res<0){
		goto all_over;
	}
all_over:
	db_close();
    api_response(res, lnk_st.lnk_base.lid, &lnk_st.lnk_base);
    //push to cb_daemon
	if((send_cb_len = cJsonLinkage_callback(send_cb_string, SUBID_EDIT_LINK, res, lnk_st.lnk_base.lid, &lnk_st.lnk_base)) >=0) {
		//if push failed, not handle temporary
		push_to_CBDaemon(send_cb_string, send_cb_len);
	}
	return 0;
}
static void api_response(int res, int id_value ,const linkage_base_st *link_base)
{
	cJSON *root;
	char *json_out;

	root=cJSON_CreateObject();

	cJSON_AddNumberToObject(root, FIELD_STATUS, res);
	if(res >=0){
		cJSON_AddNumberToObject(root, FIELD_LID, id_value);
		if(link_base !=NULL)
		{
			cJSON_AddStringToObject(root, FIELD_LNKNAME, link_base->lnkname);
			cJSON_AddStringToObject(root, FIELD_TRGIEEE, link_base->trgieee);
			cJSON_AddStringToObject(root, FIELD_TRGEP, link_base->trgep);
			cJSON_AddStringToObject(root, FIELD_TRGCND, link_base->trgcnd);
			cJSON_AddStringToObject(root, FIELD_LNKACT, link_base->lnkact);
			cJSON_AddNumberToObject(root, FIELD_ENABLE, link_base->enable);
		}
	}

	json_out=cJSON_Print(root);
	cJSON_Delete(root);

	fprintf(cgiOut,"%s\n", json_out);
	free(json_out);
}

