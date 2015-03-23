/*
 * glProtocol.h
 *
 *  Created on: Mar 12, 2015
 *      Author: yanly
 */

#ifndef GLPROTOCOL_H_
#define GLPROTOCOL_H_

/*******************************************************/  //cjson fields
//general
#define MSGTYPE_STRING			"msgtype"
#define GL_MAINID				"mainid"
#define GL_SUBID				"subid"

//specific
#define GL_STAUTS				"status"

/*******************************************************/  //cjson values
//mainid values
#define GL_MAINID_USER_MANAGE	1

//subid values
#define	GL_SUBID_M_PSW			1
#define	GL_SUBID_M_ALIAS		2

/*******************************************************/  //status values
#define	GL_MODIFY_ACTION_OK		0    //modify alias or password success

/*******************************************************/
#define GL_CALLBACK_MAX_SIZE	256

/*******************************************************/
int cJsonAlias_callback(char *text, int status_value);
int cJsonPsw_callback(char *text, int status_value);

#endif /* GLPROTOCOL_H_ */

