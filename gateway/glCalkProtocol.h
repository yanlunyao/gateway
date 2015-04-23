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
#define NUM2	NUM1+1
#define NUM3	NUM2+1
#define NUM4	NUM3+1
#define NUM5	NUM4+1
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
#define GL_CALLBACK_MAX_SIZE	1024

/*******************************************************/
int push_to_CBDaemon(const char *send_text, int send_size);
int cJsonAlias_callback(char *text, int status_value);
int cJsonPsw_callback(char *text, int status_value);
int cJsonScene_callback(char *text, const scene_base_st *scene, const char *sindexall, const char *sidall, int subid, int status_value);
int cJsonDelDoScene_callback(char *text, int sid, int subid, int res);
int cJsonTimeAction_callback(char *text, int subid, int res, int id_value, const time_action_base_st *time_act);
int cJsonEnableTimeAction_callback(char *text, int subid, int res, int id_value, int do_status);

#endif /* GLPROTOCOL_H_ */

