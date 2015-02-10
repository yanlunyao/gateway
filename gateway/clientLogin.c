/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName: clientLogin.c
  Author: fengqiuchao      
  Version : V1.0         
  Date: 2014/02/24
  Description: Client login
  History:
      <author>       <time>     <version >   <desc>
      fengqiuchao    2014/02/24     1.0     build this moudle  
***************************************************************************/

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
    char account[FEATURE_GDGL_ACCOUNT_MAX_LEN + 1]; //16 + terminating null
	char password[FEATURE_GDGL_PASSWD_MAX_LEN + 1]; //16 + terminating null
	char gateway_id[FEATURE_GDGL_ID_LEN + 1]; //12 + terminating null
	char gateway_alias[FEATURE_GDGL_ACCOUNT_MAX_LEN + 1];
	char gateway_passwd[FEATURE_GDGL_PASSWD_MAX_LEN + 1];
	cgiFormResultType cgi_re;
	int res;

	cgiHeaderContentType("application/json"); //MIME

    // Read ID
    res = read_id(gateway_id);
	if (res != 0) {
		client_admin_response("noid", res, clientAdminResultStr[res]);
		return res;
	}
	
	// Read alias
	res = read_alias(gateway_alias);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		return res;
	}

    // Read password
	res = read_password(gateway_passwd);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		return res;
	}

    // Check account
    cgi_re = cgiFormString("account", account, FEATURE_GDGL_ACCOUNT_MAX_LEN + 1);
	res = check_account(cgi_re, account, gateway_id, gateway_alias);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		return res;
	}

	// Check password
	cgi_re = cgiFormString("password", password, FEATURE_GDGL_PASSWD_MAX_LEN + 1);
	res = check_password(cgi_re, password, gateway_passwd);
	if (res != 0) {
		client_admin_response(gateway_id, res, clientAdminResultStr[res]);
		return res;
	}

    // account & password all right	
    client_admin_response(gateway_id, clientAdminSuccess, clientAdminResultStr[clientAdminSuccess]);
	return 0;
}
