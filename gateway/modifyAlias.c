/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName: modifyAlias.c
  Author: fengqiuchao      
  Version : V1.0         
  Date: 2014/03/05
  Description: modify alias
  History:
      <author>       <time>     <version >   <desc>
      fengqiuchao    2014/03/05     1.0     build this moudle  
***************************************************************************/
/*
 * modify history:
 * version2: add push msg to callback module
 * 		     	--modify by yanly
 * versino3:
*/

#include "unp.h"
#include <stdio.h>
#include "cgic.h"
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
    char id[FEATURE_GDGL_ID_LEN + 1]; //12 + terminating null
    char password[FEATURE_GDGL_PASSWD_MAX_LEN + 1]; //16 + terminating null
    char old_alias[FEATURE_GDGL_ACCOUNT_MAX_LEN + 1]; //16 + terminating null
    char new_alias[FEATURE_GDGL_ACCOUNT_MAX_LEN + 1]; //16 + terminating null
	char gateway_id[FEATURE_GDGL_ID_LEN + 1]; //12 + terminating null
	char gateway_passwd[FEATURE_GDGL_PASSWD_MAX_LEN + 1];
	cgiFormResultType cgi_re;
	int res;

	cgiHeaderContentType("application/json"); //MIME

    // Read ID
    res = read_id(gateway_id);
	if (res != 0) {
		client_admin_response("noid", res, clientAdminResultStr[res]);
		mAliasPushToCb(res);
		return res;
	}

	// Check ID
	cgi_re = cgiFormString("id", id, FEATURE_GDGL_ID_LEN + 1);
	res = check_id(cgi_re, id, gateway_id);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		mAliasPushToCb(res);
		return res;
	}

	// Read password
	res = read_password(gateway_passwd);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		mAliasPushToCb(res);
		return res;
	}

	// Check password
	cgi_re = cgiFormString("password", password, FEATURE_GDGL_PASSWD_MAX_LEN + 1);
	res = check_password(cgi_re, password, gateway_passwd);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		mAliasPushToCb(res);
		return res;
	}

	// Check new alias
	cgi_re = cgiFormString("new_alias", new_alias, FEATURE_GDGL_ACCOUNT_MAX_LEN + 1);
	res = check_new_alias(cgi_re, new_alias);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		mAliasPushToCb(res);
		return res;
	}

	// Modify alias, first check old, second push to cloud, third write new
	cgi_re = cgiFormString("old_alias", old_alias, FEATURE_GDGL_ACCOUNT_MAX_LEN + 1);
	res = modify_alias(cgi_re, old_alias, new_alias);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		mAliasPushToCb(res);
		return res;
	}
	
	res = mAliasPushToCb(0);
	if(res !=0){
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		return res;
	}

	//增解逻辑：kill模块gdgl_channel
	system("killall gdgl_channel");
    // all right	
    client_admin_response(gateway_id, clientAdminSuccess, clientAdminResultStr[clientAdminSuccess]);
	return 0;
}
