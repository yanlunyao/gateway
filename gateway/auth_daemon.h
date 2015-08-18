/*
 *	File name   : auth_daemon.h
 *  Created on  : Aug 11, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef GATEWAY_AUTH_DAEMON_H_
#define GATEWAY_AUTH_DAEMON_H_

#define  PROJECT_VERSION			"Auth_Daemon-V01-00"
#define  APP_NAME					"Auth_Daemon"
#define  APP_VERSION				"V01-00"

//default auth.json file
#define AUTH_STATE_DEFAULT			1
#define AUTH_AVAILABLE_DEFAULT		1
#define AUTH_EXPIRETIME_DEFAULT		"2025-01-01 00:00:00"
#define AUTH_BEFORE_REMIND_DEFAULT	3
#define AUTH_AFTER_REMIND_DEFAULT	3

#define REMIND_PERIOD_TIME			2*5   //2 hour unit[s]
#define AUTH_READ_TIMEOUT			10	//10s unit[s]

#define	MAX_REQUEST 				2048

//state
#define INSIDER_TEST_STATE			0
#define NORMAL_STATE				1
#define NONACTIVATED_STATE			-1
#define ARREARAGE_STATE				-2
#define LOGOFF_STATE				-3

#define ADD_MAPPING_VALUE			10
#define INSIDER_TEST_STATE_MAP		INSIDER_TEST_STATE +	ADD_MAPPING_VALUE
#define NORMAL_STATE_MAP			NORMAL_STATE +			ADD_MAPPING_VALUE
#define NONACTIVATED_STATE_MAP		NONACTIVATED_STATE +	ADD_MAPPING_VALUE
#define ARREARAGE_STATE_MAP			ARREARAGE_STATE +		ADD_MAPPING_VALUE
#define LOGOFF_STATE_MAP			LOGOFF_STATE + 			ADD_MAPPING_VALUE


//available
#define AVAILABLE_VALID				1
#define AVAILABLE_INVALID			0



typedef struct{
	int state;
	char expire_time[30];
	int available;
	int remind_before;
	int remind_after;
	time_t expire_timestamp;   			//授权时间到期的时间戳
	time_t nolonger_remind_time; 		//到期后不再提醒的时间戳 = expire_timestamp + remind_after
}auth_st;

extern auth_st auth_limit;

void setting_auth_limit_st(const auth_st temp);
void setting_auth_limit_state(int i);
void setting_auth_limit_available(int i);

void update_auth_json_file_by_string(const char *wtext);
void update_auth_json_file_by_auth_st(const auth_st *temp_auth);

#endif /* GATEWAY_AUTH_DAEMON_H_ */
