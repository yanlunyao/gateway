/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  File name: clientAdmin.h
  Description: client admin common define 
  Author: fengqiuchao
  Version: 1.0      
  Date: 2014/03/03
  History:   
    1. Date:
       Author:
       Modification:
    2. ...
***************************************************************************/

#ifndef CLIENTADMIN_H__
#define CLIENTADMIN_H__

// must map to clientAdminResultStr in clientAdmin.c
typedef enum {
	clientAdminSuccess = 0,
	clientAdminIDFileOpenErr,
	clientAdminIDFileLockErr,
	clientAdminIDFileReadErr,
	clientAdminAliasFileOpenErr,
	clientAdminAliasFileLockErr,
	clientAdminAliasFileReadErr,
	clientAdminAliasFileWriteErr,
	clientAdminAliasFileSeekErr,
	clientAdminAliasFileTruncErr,
	clientAdminPasswdFileOpenErr,
	clientAdminPasswdFileLockErr,
	clientAdminPasswdFileReadErr,
	clientAdminPasswdFileWriteErr,
	clientAdminPasswdFileSeekErr,
	clientAdminPasswdFileTruncErr,
	clientAdminIDLenErr,
	clientAdminIDEmptyErr,
	clientAdminIDNotFoundErr,
	clientAdminIDCheckErr,
	clientAdminAccountTooLongErr,
	clientAdminAccountTooShortErr,
	clientAdminAccountEmptyErr,
	clientAdminAccountNotFoundErr,
	clientAdminAccountCheckErr,
	clientAdminPasswdTooLongErr,
	clientAdminPasswdTooShortErr,
	clientAdminPasswdEmptyErr,
	clientAdminPasswdNotFoundErr,
	clientAdminPasswdCheckErr,
	clientAdminNewPasswdTooLongErr,
	clientAdminNewPasswdTooShortErr,
	clientAdminNewPasswdEmptyErr,
	clientAdminNewPasswdNotFoundErr,
	clientAdminTwoPasswdEqualErr,
	clientAdminAliasTooLongErr,
	clientAdminAliasTooShortErr,
	clientAdminAliasEmptyErr,
	clientAdminAliasNotFoundErr,
	clientAdminAliasCheckErr,
	clientAdminNewAliasTooLongErr,
	clientAdminNewAliasTooShortErr,
	clientAdminNewAliasEmptyErr,
	clientAdminNewAliasNotFoundErr,
	clientAdminTwoAliasEqualErr,
	clientAdminPushToCloudErr,
	clientAdminPushToCloudOldPasswdErr,
	clientAdminPushToCloudPasswdSame,
	clientAdminPushToCloudOldAliasNotExist,
	clientAdminPushToCloudNewAliasExist,
	clientAdminPushToCloudAliasSame,
	clientAdminPushToCBDaemonErr    //add yan150211
} clientAdminResultType;

// push to cloud response status
#define PUSH_TO_CLOUD_RESPONSE_PASSWD_SUCCESS 1
#define PUSH_TO_CLOUD_RESPONSE_ALIAS_SUCCESS 2

#define PUSH_TO_CLOUD_RESPONSE_JSON_PARSE_FAILED (-1)
#define PUSH_TO_CLOUD_RESPONSE_REQ_HANDLER_INVALID (-2)
#define PUSH_TO_CLOUD_RESPONSE_INVALID_ACTION (-3)
#define PUSH_TO_CLOUD_RESPONSE_INVALID_OLD_PASSWD (-4)
#define PUSH_TO_CLOUD_RESPONSE_INVALID_NEW_PASSWD (-5)
#define PUSH_TO_CLOUD_RESPONSE_INVALID_OLD_ALIAS (-6)
#define PUSH_TO_CLOUD_RESPONSE_INVALID_NEW_ALIAS (-7)
#define PUSH_TO_CLOUD_RESPONSE_UNKNOWN_ACTION (-8)
#define PUSH_TO_CLOUD_RESPONSE_OLD_PASSWD_ERR (-9)
#define PUSH_TO_CLOUD_RESPONSE_PASSWD_SAME (-10)
#define PUSH_TO_CLOUD_RESPONSE_OLD_ALIAS_NOT_EXIST (-11)
#define PUSH_TO_CLOUD_RESPONSE_NEW_ALIAS_EXIST (-12)
#define PUSH_TO_CLOUD_RESPONSE_ALIAS_SAME (-13)

extern const char * const clientAdminResultStr[];

void client_admin_response(const char *id_str, const int status_num, const char *status_msg_str);
int read_id(char * id_str);
int read_alias(char * alias_str);
int read_password(char * passwd_str);
int check_id(cgiFormResultType cgi_result, const char * received_id_str, const char * gateway_id_str);
int check_account(cgiFormResultType cgi_result, const char * account_str, const char * id_str, const char * alias_str);
int check_password(cgiFormResultType cgi_result, const char * received_passwd_str, const char * gateway_passwd_str);
int check_new_password(cgiFormResultType cgi_result, const char * new_passwd_str);
int modify_password(cgiFormResultType cgi_result, const char * old_passwd_str, const char * new_passwd_str);
int check_alias(cgiFormResultType cgi_result, const char * received_alias_str, const char * gateway_alias_str);
int check_new_alias(cgiFormResultType cgi_result, const char * new_alias_str);
int modify_alias(cgiFormResultType cgi_result, const char * old_alias_str, const char * new_alias_str);
int mAliasPushToCb(int status);
int mPswPushToCb(int status);

#endif //CLIENTADMIN_H__

