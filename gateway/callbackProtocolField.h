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
#define MAINID7		7
#define MAINID8		8
#define MAINID9		9
//
#define SUBID1		1
#define SUBID2		2
#define SUBID3		3
#define SUBID4		4
#define SUBID5		5
#define SUBID6		6
#define SUBID7		7
#define SUBID8		8

//
#define ALARM_LINKAGE_DEFAULT_V		0

//
#define GL_MSGTYPE						0
#define DEV_ATTRIBUTE_MSGTYPE			2
#define DEV_ALARM_MSGTYPE				3
#define IAS_DEV_CHANGE_MSGTYPE			7  //安防设备状态变化
#define DEV_ENROLL_MSGTYPE				16
#define DEV_UNENROLL_MSGTYPE			17
#define NEW_ADDED_DEVICE_MSGTYPE		29
#define DEL_DEVICE_MSGTYPE				34
//
#define MAINID_TIMEACTION				MAINID3
#define	MAINID_SCENE					MAINID4
#define	MAINID_LINKAGE					MAINID5
#define MAINID_RF						MAINID7
#define MAIND_GATEWAY_SETTING			MAINID9

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

//subid scene
#define SUBID_DEL_SCENE					SUBID3
#define SUBID_EDIT_SCENE				SUBID2

//subid rf, chge:change
#define SUBID_RF_ALARM					SUBID1
#define SUBID_RF_OPEN_STATE_CHGE		SUBID2
#define SUBID_RF_BYPASS_STATE_CHGE		SUBID3
#define SUBID_RF_NAME_CHGE				SUBID4
#define SUBID_RF_ACTIVATE_STATE_CHGE	SUBID5
#define SUBID_RF_POWER_UP				SUBID6
#define SUBID_RF_LIST_CHGE				SUBID7
#define SUBID_RF_ONLINE_STATE_CHGE		SUBID8

//GATEWAYSETTING
#define SETTING_RESPOND					SUBID1


//w_description
typedef enum {
    zgStop =0,
	zgBurglar,
	zgFire,
	zgEmergency,
	zgMute,
	zgDeviceTrouble,
	zgDoorbell,
	zgOnTime,
	zgLate,
	zgNone9,
	zgNone10,
	zgNone11,
	zgNone12,
	zgLowbattery
}zigbee_wmode;


typedef enum {
    rfWmodeNone0 =0,
	rfEmergency,
	rfGas,
	rfFire,
	rfDoorOpen,
	rfWmodeNone5,
	rfWmodeNone6,
	rfWmodeNone7,
	rfWater,
	rfPowerOn,
	rfBurglar,
	rfHeartbeat,
	rfLowbattery,
	rfDoorClose
}rf_wmode;



#endif /* CALLBACKPROTOCOLFIELD_H_ */

