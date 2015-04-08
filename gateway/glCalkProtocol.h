/*
 * glProtocol.h
 *
 *  Created on: Mar 12, 2015
 *      Author: yanly
 */

#ifndef GLPROTOCOL_H_
#define GLPROTOCOL_H_

#include "smartgateway.h"


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
#define GL_MAINID_SCENE			4

//subid values
#define	GL_SUBID_M_PSW			1
#define	GL_SUBID_M_ALIAS		2

/*******************************************************/  //status values
#define	GL_MODIFY_ACTION_OK		0    //modify alias or password success

/*******************************************************/
#define GL_CALLBACK_MAX_SIZE	1024

/*******************************************************/
int cJsonAlias_callback(char *text, int status_value);
int cJsonPsw_callback(char *text, int status_value);
int cJsonScene_callback(char *text, const scene_base_st *scene, const char *sindexall, const char *sidall, int subid, int status_value);
int push_to_CBDaemon(const char *send_text, int send_size);

#endif /* GLPROTOCOL_H_ */

