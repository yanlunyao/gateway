/*
 * glProtocol.h
 *
 *  Created on: Mar 12, 2015
 *      Author: yanly
 */

#ifndef GLPROTOCOL_H_
#define GLPROTOCOL_H_

#include "smartgateway.h"

#define NUM1	1
#define NUM2	2
#define NUM3	3
#define NUM4	4
#define NUM5	5
#define NUM6	6
#define NUM7	7
#define NUM8	8
#define NUM9	9
/*******************************************************/  //cjson fields
//general
#define MSGTYPE_STRING			"msgtype"
#define GL_MAINID				"mainid"
#define GL_SUBID				"subid"

//specific
#define GL_STAUTS				"status"

/*******************************************************/  //cjson values
#define GL_MSGTYPE_VALUE		0
//mainid values
#define GL_MAINID_USER_MANAGE	1
#define GL_MAINID_TA			NUM3
#define GL_MAINID_SCENE			NUM4
#define GL_MAINID_LINGE			NUM5
#define GL_MAINID_AUTH			NUM8
#define GL_MAINID_GATEWAY		NUM9


//subid values
#define	GL_SUBID_M_PSW			1
#define	GL_SUBID_M_ALIAS		2

//scene
#define	ADD_SUBID_SCENE			NUM1
#define	EDIT_SUBID_SCENE		NUM2
#define	DEL_SUBID_SCENE			NUM3
#define	DO_SUBID_SCENE			NUM4
#define	MODIFYIDX_SUBID_SCENE	NUM5
//timeaction
#define	ADD_TA_SUBID			NUM1
#define	EDIT_TA_SUBID			NUM2
#define	DEL_TA_SUBID			NUM3
#define	ENABLE_TA_SUBID			NUM4

/*******************************************************/  //status values
#define	GL_MODIFY_ACTION_OK		0    //modify alias or password success

/*******************************************************/
#define GL_CALLBACK_MAX_SIZE	2048

/*******************************************************/
int push_to_CBDaemon(const char *send_text, int send_size);
int cJsonAlias_callback(char *text, int status_value);
int cJsonPsw_callback(char *text, int status_value);
int cJsonScene_callback(char *text, const scene_base_st *scene, const char *sindexall, const char *sidall, int subid, int status_value);
int cJsonDelDoScene_callback(char *text, int sid, int subid, int res);
int cJsonTimeAction_callback(char *text, int subid, int res, int id_value, const time_action_base_st *time_act);
int cJsonEnableTimeAction_callback(char *text, int subid, int res, int id_value, int do_status);
int cJsonLinkage_callback(char *text, int subid, int res, int id_value, const linkage_base_st *link_base);
int cJsonEnableLinkage_callback(char *text, int subid, int res, int id_value, int enable);

#endif /* GLPROTOCOL_H_ */

