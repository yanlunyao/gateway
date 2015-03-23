/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName: modifyPassword.c
  Author: fengqiuchao      
  Version : V1.0         
  Date: 2014/03/03
  Description: modify password
  History:
      <author>       <time>     <version >   <desc>
      fengqiuchao    2014/03/03     1.0     build this moudle  
***************************************************************************/

#include "unp.h"
#include <stdio.h>
#include "cgic.h"
//#include <string.h>
//#include <stdlib.h>
//#include "cJSON.h"
#include "smartgateway.h"
#include "clientAdmin.h"

/***************************************************************************
  Function: cgiMain
  Description: 
  Input:  
  Output: 
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int cgiMain()
{
    char account[FEATURE_GDGL_ACCOUNT_MAX_LEN + 1]; //16 + terminating null
	char old_password[FEATURE_GDGL_PASSWD_MAX_LEN + 1]; //16 + terminating null
	char new_password[FEATURE_GDGL_PASSWD_MAX_LEN + 1]; //16 + terminating null
	char gateway_id[FEATURE_GDGL_ID_LEN + 1]; //12 + terminating null
	char gateway_alias[FEATURE_GDGL_ACCOUNT_MAX_LEN + 1];
	cgiFormResultType cgi_re;
	int res;

	cgiHeaderContentType("application/json"); //MIME

    // Read ID
    res = read_id(gateway_id);
	if (res != 0) {
		client_admin_response("noid", res, clientAdminResultStr[res]);
		mPswPushToCb(res);
		return res;
	}
	
	// Read alias
	res = read_alias(gateway_alias);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		mPswPushToCb(res);
		return res;
	}

    // Check account
    cgi_re = cgiFormString("account", account, FEATURE_GDGL_ACCOUNT_MAX_LEN + 1);
	res = check_account(cgi_re, account, gateway_id, gateway_alias);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		mPswPushToCb(res);
		return res;
	}

	// Check new password
	cgi_re = cgiFormString("new_password", new_password, FEATURE_GDGL_PASSWD_MAX_LEN + 1);
	res = check_new_password(cgi_re, new_password);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		mPswPushToCb(res);
		return res;
	}

	// Modify password, first check old, second push to cloud, third write new
	cgi_re = cgiFormString("old_password", old_password, FEATURE_GDGL_PASSWD_MAX_LEN + 1);
	res = modify_password(cgi_re, old_password, new_password);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		mPswPushToCb(res);
		return res;
	}

	//pust to cb daemon
	res = mPswPushToCb(0);
	if(res !=0){
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		return res;
	}

    // all right	
    client_admin_response(gateway_id, clientAdminSuccess, clientAdminResultStr[clientAdminSuccess]);
	return 0;
}

