/*
 *	File name   : callbackProtocolField.h
 *  Created on  : Apr 23, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#ifndef CALLBACKPROTOCOLFIELD_H_
#define CALLBACKPROTOCOLFIELD_H_

//field
//==========================================================================================================
#define FIELD_MSGTYPE			"msgtype"
#define FIELD_MAINID			"mainid"
#define FIELD_SUBID				"subid"
#define FIELD_STATUS			"status"
#define FIELD_TID				"tid"
#define FIELD_ACTNAME			"actname"
#define FIELD_ACTPARA			"actpara"
#define FIELD_ACTMODE			"actmode"
#define FIELD_PARA1				"para1"
#define FIELD_PARA2				"para2"
#define FIELD_PARA3				"para3"
#define FIELD_ENABLE			"enable"

#define FIELD_LID				"lid"
#define FIELD_LNKNAME			"lnkname"
#define FIELD_TRGIEEE			"trgieee"
#define FIELD_TRGEP				"trgep"
#define FIELD_TRGCND			"trgcnd"
#define FIELD_LNKACT			"lnkact"

//value
//==========================================================================================================
//
#define MAINID1		1
#define MAINID2		2
#define MAINID3		3
#define MAINID4		4
#define MAINID5		5
#define MAINID6		6
//
#define SUBID1		1
#define SUBID2		2
#define SUBID3		3
#define SUBID4		4
#define SUBID5		5
#define SUBID6		6
#define SUBID7		7

//
#define GL_MSGTYPE						0
#define DEL_DEVICE_MSGTYPE				34

//
#define MAINID_TIMEACTION				MAINID3
#define	MAINID_SCENE					MAINID4
#define	MAINID_LINKAGE					MAINID5

//subid timeaction
#define SUBID_ADD_TA					SUBID1
#define SUBID_EDIT_TA					SUBID2
#define SUBID_DEL_TA					SUBID3
#define SUBID_ENABLE_TA					SUBID4

//subid linkage
#define SUBID_ADD_LINK					SUBID1
#define SUBID_EDIT_LINK					SUBID2
#define SUBID_DEL_LINK					SUBID3
#define SUBID_ENABLE_LINK				SUBID4


#endif /* CALLBACKPROTOCOLFIELD_H_ */

