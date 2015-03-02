/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName: clientAdmin.c
  Author: fengqiuchao      
  Version : V1.0         
  Date: 2014/02/24
  Description: Client Admin gloable define
  History:
      <author>       <time>     <version >   <desc>
      fengqiuchao    2014/02/24     1.0     build this moudle  
***************************************************************************/
/*
 * modify history:
 * version2: modify modify_alias(add push to CB_Daemon),
 * 			 modify modify_password(add push to CB_Daemon),
 * 			 --by yanly.
 * version3:
 *
*/

#include "unp.h"
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"
#include "lockFile.h"
#include <sys/msg.h>
#include "smartgateway.h"
#include "clientAdmin.h"

static int construct_msg(clientAdminMsgType msgtype, const char *text_str, struct client_admin_msgbuf *pmsg);

typedef enum {
    modifyPasswordType,
	modifyAliasType
} modifyType;

// must map to clientAdminResultType in clientAdmin.h
const char * const clientAdminResultStr[] = {
	"success",
	"ID file open error",
	"ID file lock error",
	"ID file read error",
	"alias file open error",
	"alias file lock error",
	"alias file read error",
	"alias file write error",
	"alias file seek error",
	"alias file truncate error",
	"password file open error",
	"password file lock error",
	"password file read error",
	"password file write error",
	"password file seek error",
	"password file truncate error",
	"ID length error",
	"ID empty error",
	"ID not found error",
	"ID check error",
	"account too long error",
	"account too short error",
	"account empty error",
	"account not found error",
	"account check error",
	"password too long error",
	"password too short error",
	"password empty error",
	"password not found error",
	"password check error",
	"new password too long error",
	"new password too short error",
	"new password empty error",
	"new password not found error",
	"two passwords equal",
	"alias too long error",
	"alias too short error",
	"alias empty error",
	"alias not found error",
	"alias check error",
	"new alias too long error",
	"new alias too short error",
	"new alias empty error",
	"new alias not found error",
	"two alias equal",
	"push to cloud error",
	"push to cloud old password error",
	"push to cloud password same error",
	"push to cloud old alias not exist error",
	"push to cloud new alias already exist error",
	"push to cloud alias same error",
	"push to CB_Daemon error"   //add yanly150211
};

/***************************************************************************
  Function: client_admin_response
  Description: 
  Input: id_str
            status_num
            status_msg_str
  Output: 
  Return: none
  Others:  none
***************************************************************************/
void client_admin_response(const char *id_str, const int status_num, const char *status_msg_str)
{
    cJSON *root, *response;
    char *json_out;
	
    root=cJSON_CreateObject();
    cJSON_AddStringToObject(root, "id", id_str);
    cJSON_AddItemToObject(root, "response_params", response=cJSON_CreateObject());
    cJSON_AddNumberToObject(response, "status", status_num);
    cJSON_AddStringToObject(response, "status_msg",	status_msg_str);
    json_out=cJSON_Print(root);
    cJSON_Delete(root);	
    fprintf(cgiOut,"%s\n", json_out);	
    free(json_out);
}

/***************************************************************************
  Function: read_id
  Description: read smartgateway ID from mac.conf
  Input:  
  Output: id_str, null terminated
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int read_id(char * id_str)
{
    int fd_id;
	int res;
	
    fd_id = open(FEATURE_GDGL_MAC_PATH, O_RDONLY, 0777);
	if (fd_id < 0) {
		return clientAdminIDFileOpenErr;
	}
	res = read_lock(fd_id, 0, SEEK_SET, 0);
	if (res == -1) {
		close(fd_id);
		return clientAdminIDFileLockErr;
	}
	res = read(fd_id, id_str, FEATURE_GDGL_ID_LEN);
	if (res != FEATURE_GDGL_ID_LEN) {
		close(fd_id);
		return clientAdminIDFileReadErr;
	}
	close(fd_id); //also unlock
	id_str[FEATURE_GDGL_ID_LEN] = '\0';

	return 0;
}

/***************************************************************************
  Function: read_alias
  Description: read smartgateway alias from alias.conf
  Input:  
  Output: alias_str, null terminated
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int read_alias(char * alias_str)
{
    int fd_alias;
	int res;
	
    fd_alias = open(FEATURE_GDGL_ALIAS_PATH, O_RDONLY, 0777);
	if (fd_alias < 0) {
		return clientAdminAliasFileOpenErr;
	}
	res = read_lock(fd_alias, 0, SEEK_SET, 0);
	if (res == -1) {
		close(fd_alias);
		return clientAdminAliasFileLockErr;
	}
	res = read(fd_alias, alias_str, FEATURE_GDGL_ACCOUNT_MAX_LEN);
	if (res < 0) {
		close(fd_alias);
		return clientAdminAliasFileReadErr;
	}
	close(fd_alias); //also unlock
	alias_str[res] = '\0';

	return 0;
}

/***************************************************************************
  Function: read_password
  Description: read smartgateway password from password.conf
  Input:  
  Output: passwd_str, null terminated
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int read_password(char * passwd_str)
{
    int fd_passwd;
	int res;
	
    fd_passwd = open(FEATURE_GDGL_PASSWD_PATH, O_RDONLY, 0777);
	if (fd_passwd < 0) {
		return clientAdminPasswdFileOpenErr;
	}
	res = read_lock(fd_passwd, 0, SEEK_SET, 0);
	if (res == -1) {
		close(fd_passwd);
		return clientAdminPasswdFileLockErr;
	}
	res = read(fd_passwd, passwd_str, FEATURE_GDGL_PASSWD_MAX_LEN);
	if (res < 0) {
		close(fd_passwd);
		return clientAdminPasswdFileReadErr;
	}
	close(fd_passwd); //also unlock
	passwd_str[res] = '\0';

	return 0;
}

/***************************************************************************
  Function: check_id
  Description: check id from client URL
  Input:  cgi_result, the return value of cgiFormString
             received_id_str, the output of cgiFormString
             gateway_id_str, the smartgateway id
  Output: none
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int check_id(cgiFormResultType cgi_result, const char * received_id_str, const char * gateway_id_str)
{
    int ret;
	
    switch (cgi_result) {
		case cgiFormSuccess:
			if (strlen(received_id_str) != FEATURE_GDGL_ID_LEN) {
			    ret = clientAdminIDLenErr;
				break;
			}
			if (strcmp(received_id_str, gateway_id_str) != 0) {
				ret = clientAdminIDCheckErr;
			}
			else {
			    ret = 0;
			}
			break;
			
		case cgiFormTruncated:
			ret = clientAdminIDLenErr;
			break;
			
		case cgiFormEmpty:
			ret = clientAdminIDEmptyErr;
			break;
			
		case cgiFormNotFound:
		default:
			ret = clientAdminIDNotFoundErr;
			break;
	}

	return ret;
}

/***************************************************************************
  Function: check_account
  Description: check account from client URL
  Input:  cgi_result, the return value of cgiFormString
             account_str, the output of cgiFormString
             id_str, the smartgateway id
             alias_str, the smartgateway alias
  Output: none
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int check_account(cgiFormResultType cgi_result, const char * account_str, const char * id_str, const char * alias_str)
{
    int ret;
	
    switch (cgi_result) {
		case cgiFormSuccess:
			if (strlen(account_str) < FEATURE_GDGL_ACCOUNT_MIN_LEN) {
			    ret = clientAdminAccountTooShortErr;
				break;
			}
			if ( (strcmp(account_str, id_str) != 0) && (strcmp(account_str, alias_str) != 0) ) {
				ret = clientAdminAccountCheckErr;
			}
			else {
			    ret = 0;
			}
			break;
			
		case cgiFormTruncated:
			ret = clientAdminAccountTooLongErr;
			break;
			
		case cgiFormEmpty:
			ret = clientAdminAccountEmptyErr;
			break;
			
		case cgiFormNotFound:
		default:
			ret = clientAdminAccountNotFoundErr;
			break;
	}

	return ret;
}

/***************************************************************************
  Function: check_password
  Description: check password from client URL
  Input:  cgi_result, the return value of cgiFormString
             received_passwd_str, the output of cgiFormString
             gateway_passwd_str, the smartgateway password
  Output: none
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int check_password(cgiFormResultType cgi_result, const char * received_passwd_str, const char * gateway_passwd_str)
{
    int ret;
	
    switch (cgi_result) {
		case cgiFormSuccess:
			if (strlen(received_passwd_str) < FEATURE_GDGL_PASSWD_MIN_LEN) {
			    ret = clientAdminPasswdTooShortErr;
				break;
			}
			if (strcmp(received_passwd_str, gateway_passwd_str) != 0) {
				ret = clientAdminPasswdCheckErr;
			}
			else {
			    ret = 0;
			}
			break;
			
		case cgiFormTruncated:
			ret = clientAdminPasswdTooLongErr;
			break;
			
		case cgiFormEmpty:
			ret = clientAdminPasswdEmptyErr;
			break;
			
		case cgiFormNotFound:
		default:
			ret = clientAdminPasswdNotFoundErr;
			break;
	}

	return ret;
}

/***************************************************************************
  Function: check_new_password
  Description: check new password from client URL
  Input:  cgi_result, the return value of cgiFormString
             new_passwd_str, the new password
  Output: none
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int check_new_password(cgiFormResultType cgi_result, const char * new_passwd_str)
{
    int ret;
	
    switch (cgi_result) {
		case cgiFormSuccess:
			if (strlen(new_passwd_str) < FEATURE_GDGL_PASSWD_MIN_LEN) {
			    ret = clientAdminNewPasswdTooShortErr;
				break;
			}
			// check OK
			ret = 0;
			break;
			
		case cgiFormTruncated:
			ret = clientAdminNewPasswdTooLongErr;
			break;
			
		case cgiFormEmpty:
			ret = clientAdminNewPasswdEmptyErr;
			break;
			
		case cgiFormNotFound:
		default:
			ret = clientAdminNewPasswdNotFoundErr;
			break;
	}

	return ret;
}

#define CA_PUSH_TO_CLOUD_LEN 200
#define PUSH_TO_CLOUD_TIMEOUT 5 //seconds

//#define ENABLE_CA_DEBUG

#ifdef ENABLE_CA_DEBUG
#define CA_DEBUG(fmt, args...)	printf("%s(%d)[%s]: " fmt, __FILE__, __LINE__, __func__, ## args)
#else
#define CA_DEBUG(fmt, args...)
#endif

/***************************************************************************
  Function: push_to_cloud
  Description: 
  Input: mt
            action_str
            old_str
            new_str
  Output: none
  Return: 0:receive status[1(if modifypassword) or 2(if modifyalias)]            
              positive:receive status[(-9,-10)(if modifypassword) or (-11,-12,-13)(if modifyalias)]
              negative:other error
  Others:  none
***************************************************************************/
static int push_to_cloud(modifyType mt, const char *action_str, const char *old_str, const char *new_str)
{
    cJSON *send_json, *receive_json, *status_json;
    char *json_send_out;
	char send_str[CA_PUSH_TO_CLOUD_LEN], receive_str[CA_PUSH_TO_CLOUD_LEN];
	int fd, nwrite, nread, re, nfd;
	struct sockaddr_in	servaddr;

	// Section1: prepare for push
    send_json = cJSON_CreateObject();
	if (!send_json) {
		CA_DEBUG("create send_json failed\n");
		return -1;
	}
	cJSON_AddStringToObject(send_json, "action", action_str);
	switch (mt) {
		case modifyPasswordType:
			cJSON_AddStringToObject(send_json, "old_password", old_str);
			cJSON_AddStringToObject(send_json, "new_password", new_str);
			break;
		case modifyAliasType:
			cJSON_AddStringToObject(send_json, "old_alias", old_str);
			cJSON_AddStringToObject(send_json, "new_alias", new_str);
			break;
		default:
			CA_DEBUG("undefined modifyType:%d\n", mt);
			cJSON_Delete(send_json);
			return -1;
	}
    if ( (json_send_out = cJSON_PrintUnformatted(send_json)) == 0 ) {
		CA_DEBUG("%d print send_json failed\n", mt);
		cJSON_Delete(send_json);
		return -1;
    }
	
    cJSON_Delete(send_json);	
    nwrite = snprintf(send_str, CA_PUSH_TO_CLOUD_LEN, "%s", json_send_out);
	nwrite += 1; // including the terminated null 
    free(json_send_out);

    // Section2: send and receive
    // reWrite the next line
	//fd = Tcp_connect("192.168.1.121", FEATURE_GDGL_CPROXY_CA_PUSH_PORT_STR);
	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        CA_DEBUG("%d socket error\n", mt);
		return -2;
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(FEATURE_GDGL_CPROXY_CA_PUSH_PORT);
	//if ( (re = inet_pton(AF_INET, "192.168.1.238", &servaddr.sin_addr)) <= 0) {
	if ( (re = inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr)) <= 0) {
		CA_DEBUG("%d inet_pton error:%d\n", mt, re);
		close(fd);
		return -2;
	}	
	if ( connect(fd, (SA *) &servaddr, sizeof(servaddr)) < 0 ) {
		CA_DEBUG("%d connect error\n", mt);
		close(fd);
		return -2;
	}	
	
    if ( writen(fd, send_str, nwrite) != nwrite ) {
		CA_DEBUG("%d write error\n", mt);
		close(fd);
		return -2;
    }

    if ( (nfd = readable_timeo(fd, PUSH_TO_CLOUD_TIMEOUT)) < 0) {
		CA_DEBUG("%d readable_timeo error\n", mt);
		close(fd);
		return -2;
	}
	else if (nfd == 0) { //timeout, close connection
		CA_DEBUG("%d readable_timeo timeout\n", mt);
		close(fd);
		return -2;
	}

	while (1) {
	    if ( (nread = read(fd, receive_str, CA_PUSH_TO_CLOUD_LEN - 1)) < 0) {
			if (errno == EINTR) 
				continue;
			else {
		        CA_DEBUG("%d read error\n", mt);
		        close(fd);
		        return -2;
			}
	    }
		else if (nread == 0) {
			CA_DEBUG("%d closed by other end\n", mt);
		    close(fd);
		    return -2;
		}
		else
			break;
	}
	close(fd);
	receive_str[nread] = 0; // add the terminated null 
	CA_DEBUG("%d receive:\n%s\n", mt, receive_str);

	// Section3: parse result
	receive_json = cJSON_Parse(receive_str);
	if (!receive_json) {
		CA_DEBUG("%d receive_json parse Error before:%s\n", mt, cJSON_GetErrorPtr());
		return -3;
	}

   	status_json = cJSON_GetObjectItem(receive_json, "status");
	if (!status_json) {
		cJSON_Delete(receive_json);
		CA_DEBUG("%d receive no status\n", mt);
		return -3;
	}
    if (status_json->type != cJSON_Number) {		
		CA_DEBUG("%d receive status but not a number\n", mt);
		cJSON_Delete(receive_json);
		return -3;
	}
	switch (status_json->valueint) {
		case PUSH_TO_CLOUD_RESPONSE_JSON_PARSE_FAILED:
			CA_DEBUG("%d receive [json parse failed]\n", mt);
		    cJSON_Delete(receive_json);
		    return -3;

		case PUSH_TO_CLOUD_RESPONSE_REQ_HANDLER_INVALID:
			CA_DEBUG("%d receive [request handler invalid]\n", mt);
		    cJSON_Delete(receive_json);
		    return -3;

		case PUSH_TO_CLOUD_RESPONSE_INVALID_ACTION:
			CA_DEBUG("%d receive [invalid action]\n", mt);
		    cJSON_Delete(receive_json);
		    return -3;
			
		case PUSH_TO_CLOUD_RESPONSE_UNKNOWN_ACTION:
			CA_DEBUG("%d receive [unknown action]\n", mt);
		    cJSON_Delete(receive_json);
		    return -3;
									
		default:
			break;
	}
	if (mt == modifyPasswordType) {
	    switch (status_json->valueint) {
		    case PUSH_TO_CLOUD_RESPONSE_PASSWD_SUCCESS:
			    cJSON_Delete(receive_json);
	            return 0;

		    case PUSH_TO_CLOUD_RESPONSE_INVALID_OLD_PASSWD:
			    CA_DEBUG("%d receive [invalid old_password]\n", mt);
		        cJSON_Delete(receive_json);
		        return -3;

		    case PUSH_TO_CLOUD_RESPONSE_INVALID_NEW_PASSWD:
			    CA_DEBUG("%d receive [invalid new_password]\n", mt);
		        cJSON_Delete(receive_json);
		        return -3;
						
		    case PUSH_TO_CLOUD_RESPONSE_OLD_PASSWD_ERR:
			    CA_DEBUG("%d receive [old password not correct]\n", mt);
		        cJSON_Delete(receive_json);
		        return clientAdminPushToCloudOldPasswdErr;
			
		    case PUSH_TO_CLOUD_RESPONSE_PASSWD_SAME:
			    CA_DEBUG("%d receive [same passwords]\n", mt);
		        cJSON_Delete(receive_json);
		        return clientAdminPushToCloudPasswdSame;
						
		    default:
			    CA_DEBUG("%d receive unsupported status:[%d]\n", mt, status_json->valueint);
		        cJSON_Delete(receive_json);
		        return -3;
	    }
	}
	else if (mt == modifyAliasType) {
	    switch (status_json->valueint) {
		    case PUSH_TO_CLOUD_RESPONSE_ALIAS_SUCCESS:
			    cJSON_Delete(receive_json);
	            return 0;
			    				
		    case PUSH_TO_CLOUD_RESPONSE_INVALID_OLD_ALIAS:
			    CA_DEBUG("%d receive [invalid old_alias]\n", mt);
		        cJSON_Delete(receive_json);
		        return -3;

		    case PUSH_TO_CLOUD_RESPONSE_INVALID_NEW_ALIAS:
			    CA_DEBUG("%d receive [invalid new_alias]\n", mt);
		        cJSON_Delete(receive_json);
		        return -3;
						
		    case PUSH_TO_CLOUD_RESPONSE_OLD_ALIAS_NOT_EXIST:
			    CA_DEBUG("%d receive [old alias not exist]\n", mt);
		        cJSON_Delete(receive_json);
		        return clientAdminPushToCloudOldAliasNotExist;
			
		    case PUSH_TO_CLOUD_RESPONSE_NEW_ALIAS_EXIST:
			    CA_DEBUG("%d receive [new alias already exist]\n", mt);
		        cJSON_Delete(receive_json);
		        return clientAdminPushToCloudNewAliasExist;
			
		    case PUSH_TO_CLOUD_RESPONSE_ALIAS_SAME:
			    CA_DEBUG("%d receive [same aliases]\n", mt);
		        cJSON_Delete(receive_json);
		        return clientAdminPushToCloudAliasSame;
			
		    default:
			    CA_DEBUG("%d receive unsupported status:[%d]\n", mt, status_json->valueint);
		        cJSON_Delete(receive_json);
		        return -3;
	    }
	}
	else {
		CA_DEBUG("undefined modifyType:%d\n", mt);
		cJSON_Delete(receive_json);
		return -3;
	}
}
//add by yanly
#define PUSH_TO_CB_DAEMON_MAX_LEN  256
/*
* Function: push_to_CBDaemon
* Description:
* Input: send
* Output: none
* Return: 0>>push success;
*	  	  -1>>push error,socket send error
* Others:  none
*/
static int push_to_CBDaemon(char *send_text, int send_size)
{
	int fd;
	struct sockaddr_in	servaddr;

	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        CA_DEBUG("%d socket error\n", mt);
		return -1;
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(FEATURE_GDGL_CPROXY_CB_PUSH_PORT);
	servaddr.sin_addr.s_addr = inet_addr(LOCALHOST_TEST); //debug yan

	if ( connect(fd, (SA *) &servaddr, sizeof(servaddr)) < 0 ) {
		CA_DEBUG("%d connect error\n", mt);
		close(fd);
		return -1;
	}
	//sleep(1); //debug yan
    if ( writen(fd, send_text, send_size) != send_size ) {
		CA_DEBUG("%d write error\n", mt);
		close(fd);
		return -1;
    }

    //sleep(3); //debug yan
	close(fd);

	return 0;
}
/*
 * Function: generate_push_to_CB_string_alias
 * Input:  old_alias_str,new_alias_str
 * Output: string
 * Return: success: string size
 * 		   error: <0
 * Others: none
*/
static int generate_push_to_CB_string_alias(char *string, const char *old_alias_str,const char *new_alias_str)
{
    cJSON *send_json;
    char *json_send_out;
    int nwrite;

    send_json = cJSON_CreateObject();
	if (!send_json) {
		CA_DEBUG("create send_json failed\n");
		return -1;
	}
	cJSON_AddNumberToObject(send_json, "msgtype", 10000);
	cJSON_AddStringToObject(send_json, "old_alias", old_alias_str);
	cJSON_AddStringToObject(send_json, "new_alias", new_alias_str);

    if((json_send_out = cJSON_PrintUnformatted(send_json)) == 0 ){
		cJSON_Delete(send_json);
		return -1;
    }

    cJSON_Delete(send_json);
    nwrite = snprintf(string, PUSH_TO_CB_DAEMON_MAX_LEN, "%s", json_send_out);
	nwrite += 1; // including the terminated null
    free(json_send_out);
    return nwrite;
}
/*
 * Function: generate_push_to_CB_string_psw
 * Input:  old_alias_str,new_alias_str
 * Output: string
 * Return: success: string size
 * 		   error: <0
 * Others: none
*/
static int generate_push_to_CB_string_psw(char *string, const char *old_str,const char *new_str)
{
    cJSON *send_json;
    char *json_send_out;
    int nwrite;

    send_json = cJSON_CreateObject();
	if (!send_json) {
		CA_DEBUG("create send_json failed\n");
		return -1;
	}
	cJSON_AddNumberToObject(send_json, "msgtype", 10001);
	cJSON_AddStringToObject(send_json, "old_password", old_str);
	cJSON_AddStringToObject(send_json, "new_password", new_str);

    if((json_send_out = cJSON_PrintUnformatted(send_json)) == 0 ){
		cJSON_Delete(send_json);
		return -1;
    }

    cJSON_Delete(send_json);
    nwrite = snprintf(string, PUSH_TO_CB_DAEMON_MAX_LEN, "%s", json_send_out);
	nwrite += 1; // including the terminated null
    free(json_send_out);
    return nwrite;
}

//end add by yanly
/***************************************************************************
  Function: modify_password
  Description: read old password and check, write new password to password.conf & push to cloud server
  Input: cgi_result, the return value of cgiFormString
            old_passwd_str, null terminated, not check yet
            new_passwd_str, null terminated, already check
  Output: 
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int modify_password(cgiFormResultType cgi_result, const char * old_passwd_str, const char * new_passwd_str)
{
    int fd_passwd;
	char gateway_passwd[FEATURE_GDGL_PASSWD_MAX_LEN + 1];
	int res;
	int len, tries = 5;
	int mqid;
	struct client_admin_msgbuf mesg;
	int send_cb_len;
	char send_cb_string[PUSH_TO_CB_DAEMON_MAX_LEN];
	
    fd_passwd = open(FEATURE_GDGL_PASSWD_PATH, O_RDWR, 0777);
	if (fd_passwd < 0) {
		return clientAdminPasswdFileOpenErr;
	}
	res = write_lock(fd_passwd, 0, SEEK_SET, 0);
	if (res == -1) {
		close(fd_passwd);
		return clientAdminPasswdFileLockErr;
	}
	// Read old password
	res = read(fd_passwd, gateway_passwd, FEATURE_GDGL_PASSWD_MAX_LEN);
	if (res < 0) {
		close(fd_passwd);
		return clientAdminPasswdFileReadErr;
	}
	gateway_passwd[res] = '\0';
    // Check old password
	res = check_password(cgi_result, old_passwd_str, gateway_passwd);
	if (res != 0) {
		close(fd_passwd);
		return res;
	}
	// Check if old == new
	if (strcmp(old_passwd_str, new_passwd_str) == 0) {
		close(fd_passwd);
		return clientAdminTwoPasswdEqualErr;
	}
		
	// Push to cloud
	//if push failed, return push error
	if ( (res = push_to_cloud(modifyPasswordType, "modifypassword", old_passwd_str, new_passwd_str)) < 0 ) {
		close(fd_passwd);
		return clientAdminPushToCloudErr;
	}
	else if (res > 0) {
		close(fd_passwd);
		return res;
	}  //debug yan
	
	// Push to CB_Daemon  //add yanly150211
	// generate push string //debug
	if( (send_cb_len = generate_push_to_CB_string_psw(send_cb_string, old_passwd_str, new_passwd_str)) <0) {
		return clientAdminPushToCBDaemonErr;
	}
	//if push failed, return push error
	if ( (res = push_to_CBDaemon(send_cb_string, send_cb_len)) < 0 ) {
		close(fd_passwd);
		return clientAdminPushToCBDaemonErr;
	}

	// Write new password
	len = strlen(new_passwd_str);
	while (tries > 0) {
		// seek to beginning of file
		res = lseek(fd_passwd, 0, SEEK_SET);
		if (res < 0) {
		    close(fd_passwd);
		    return clientAdminPasswdFileSeekErr;
	    }
		// truncate the file to 0 byte
	    res = ftruncate(fd_passwd, 0);
	    if (res < 0) {
		    close(fd_passwd);
		    return clientAdminPasswdFileTruncErr;
	    }
	    res = writen(fd_passwd, new_passwd_str, len);
	    if (res != len) {
			tries--;		    
	    }
		else {
			break;
		}
	}
	// ����ظ���Ȼʧ�ܣ���ʱ�����ϴ洢����Ϣ������ģ�Ӧ����ô����????
	if (tries == 0) {
	    close(fd_passwd);
		return clientAdminPasswdFileWriteErr;
	}
	
	close(fd_passwd); //also unlock

	// Send IPC msg
	mqid = msgget(CLIENTADMIN_MQ_KEY, 0);
	if (mqid == -1) {
		CA_DEBUG("msgget error %d:%s\n", mqid, strerror(errno));
	}
	else {
		len = construct_msg(clientAdmintMsgPassword, new_passwd_str, &mesg);
		if (len > 0) {
			res = msgsnd(mqid, &mesg, len, 0);
			if (res == -1) {
				CA_DEBUG("msgsnd error %s\n", strerror(errno));
			}
		}		
	}
	return 0;
}

/***************************************************************************
  Function: check_alias
  Description: check alias from client URL
  Input:  cgi_result, the return value of cgiFormString
             received_alias_str, the output of cgiFormString
             gateway_alias_str, the smartgateway alias
  Output: none
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int check_alias(cgiFormResultType cgi_result, const char * received_alias_str, const char * gateway_alias_str)
{
    int ret;
	
    switch (cgi_result) {
		case cgiFormSuccess:
			if (strlen(received_alias_str) < FEATURE_GDGL_ACCOUNT_MIN_LEN) {
			    ret = clientAdminAliasTooShortErr;
				break;
			}
			if (strcmp(received_alias_str, gateway_alias_str) != 0) {
				ret = clientAdminAliasCheckErr;
			}
			else {
			    ret = 0;
			}
			break;
			
		case cgiFormTruncated:
			ret = clientAdminAliasTooLongErr;
			break;
			
		case cgiFormEmpty:
			ret = clientAdminAliasEmptyErr;
			break;
			
		case cgiFormNotFound:
		default:
			ret = clientAdminAliasNotFoundErr;
			break;
	}

	return ret;
}

/***************************************************************************
  Function: check_new_alias
  Description: check new alias from client URL
  Input:  cgi_result, the return value of cgiFormString
             new_alias_str, the new alias
  Output: none
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
int check_new_alias(cgiFormResultType cgi_result, const char * new_alias_str)
{
    int ret;
	
    switch (cgi_result) {
		case cgiFormSuccess:
			if (strlen(new_alias_str) < FEATURE_GDGL_ACCOUNT_MIN_LEN) {
			    ret = clientAdminNewAliasTooShortErr;
				break;
			}
			// check OK
			ret = 0;
			break;
			
		case cgiFormTruncated:
			ret = clientAdminNewAliasTooLongErr;
			break;
			
		case cgiFormEmpty:
			ret = clientAdminNewAliasEmptyErr;
			break;
			
		case cgiFormNotFound:
		default:
			ret = clientAdminNewAliasNotFoundErr;
			break;
	}

	return ret;
}

/***************************************************************************
  Function: modify_alias
  Description: read old alias and check, write new alias to alias.conf & push to cloud server &push to CB_Daemon
  Input: cgi_result, the return value of cgiFormString
            old_alias_str, null terminated, not check yet
            new_alias_str, null terminated, already check
  Output: 
  Return: 0 OK, other Error
  Others:  none
***************************************************************************/
//origin:
//int modify_alias(cgiFormResultType cgi_result, const char * old_alias_str, const char * new_alias_str)
//{
//    int fd_alias;
//	char gateway_alias[FEATURE_GDGL_ACCOUNT_MAX_LEN + 1];
//	int res;
//	int len, tries = 5;
//	int mqid;
//	struct client_admin_msgbuf mesg;
//
//    fd_alias = open(FEATURE_GDGL_ALIAS_PATH, O_RDWR, 0777);
//	if (fd_alias < 0) {
//		return clientAdminAliasFileOpenErr;
//	}
//	res = write_lock(fd_alias, 0, SEEK_SET, 0);
//	if (res == -1) {
//		close(fd_alias);
//		return clientAdminAliasFileLockErr;
//	}
//	// Read old alias
//	res = read(fd_alias, gateway_alias, FEATURE_GDGL_ACCOUNT_MAX_LEN);
//	if (res < 0) {
//		close(fd_alias);
//		return clientAdminAliasFileReadErr;
//	}
//	gateway_alias[res] = '\0';
//    // Check old alias
//	res = check_alias(cgi_result, old_alias_str, gateway_alias);
//	if (res != 0) {
//		close(fd_alias);
//		return res;
//	}
//	// Check if old == new
//	if (strcmp(old_alias_str, new_alias_str) == 0) {
//		close(fd_alias);
//		return clientAdminTwoAliasEqualErr;
//	}
//
//	// Push to cloud
//	//if push failed, return push error
//	if ( (res = push_to_cloud(modifyAliasType, "modifyalias", old_alias_str, new_alias_str)) < 0 ) {
//		close(fd_alias);
//		return clientAdminPushToCloudErr;
//	}
//	else if (res > 0) {
//		close(fd_alias);
//		return res;
//	}
//
//	// Write new alias
//	len = strlen(new_alias_str);
//	while (tries > 0) {
//		// seek to beginning of file
//		res = lseek(fd_alias, 0, SEEK_SET);
//		if (res < 0) {
//		    close(fd_alias);
//		    return clientAdminAliasFileSeekErr;
//	    }
//		// truncate the file to 0 byte
//	    res = ftruncate(fd_alias, 0);
//	    if (res < 0) {
//		    close(fd_alias);
//		    return clientAdminAliasFileTruncErr;
//	    }
//	    res = writen(fd_alias, new_alias_str, len);
//	    if (res != len) {
//			tries--;
//	    }
//		else {
//			break;
//		}
//	}
//	if (tries == 0) {
//	    close(fd_alias);
//		return clientAdminAliasFileWriteErr;
//	}
//
//	close(fd_alias); //also unlock
//
//	// Send IPC msg
//	mqid = msgget(CLIENTADMIN_MQ_KEY, 0);
//	if (mqid == -1) {
//		CA_DEBUG("msgget error %d:%s\n", mqid, strerror(errno));
//	}
//	else {
//		len = construct_msg(clientAdmintMsgAlias, new_alias_str, &mesg);
//		if (len > 0) {
//			res = msgsnd(mqid, &mesg, len, 0);
//			if (res == -1) {
//				CA_DEBUG("msgsnd error %s\n", strerror(errno));
//			}
//		}
//	}
//	return 0;
//}
int modify_alias(cgiFormResultType cgi_result, const char * old_alias_str, const char * new_alias_str)
{
    int fd_alias;
	char gateway_alias[FEATURE_GDGL_ACCOUNT_MAX_LEN + 1];
	int res;
	int len, tries = 5;
	int mqid;
	struct client_admin_msgbuf mesg;
	
	int send_cb_len;
	char send_cb_string[PUSH_TO_CB_DAEMON_MAX_LEN];

    fd_alias = open(FEATURE_GDGL_ALIAS_PATH, O_RDWR, 0777);
	if (fd_alias < 0) {
		return clientAdminAliasFileOpenErr;
	}
	res = write_lock(fd_alias, 0, SEEK_SET, 0);
	if (res == -1) {
		close(fd_alias);
		return clientAdminAliasFileLockErr;
	}
	// Read old alias
	res = read(fd_alias, gateway_alias, FEATURE_GDGL_ACCOUNT_MAX_LEN);
	if (res < 0) {
		close(fd_alias);
		return clientAdminAliasFileReadErr;
	}
	gateway_alias[res] = '\0';
    // Check old alias
	res = check_alias(cgi_result, old_alias_str, gateway_alias);
	if (res != 0) {
		close(fd_alias);
		return res;
	}
	// Check if old == new
	if (strcmp(old_alias_str, new_alias_str) == 0) {
		close(fd_alias);
		return clientAdminTwoAliasEqualErr;
	}
		
	// Push to cloud
	//if push failed, return push error
	if ( (res = push_to_cloud(modifyAliasType, "modifyalias", old_alias_str, new_alias_str)) < 0 ) {
		close(fd_alias);
		return clientAdminPushToCloudErr;
	}
	else if (res > 0) {
		close(fd_alias);
		return res;
	} //debug yan
	
	// Push to CB_Daemon
	// generate push string  //debug
	if( (send_cb_len = generate_push_to_CB_string_alias(send_cb_string, old_alias_str, new_alias_str)) <0) {
		return clientAdminPushToCBDaemonErr;
	}
	//if push failed, return push error
	if ( (res = push_to_CBDaemon(send_cb_string, send_cb_len)) < 0 ) {
		close(fd_alias);
		return clientAdminPushToCBDaemonErr;
	}

	// Write new alias
	len = strlen(new_alias_str);
	while (tries > 0) {
		// seek to beginning of file
		res = lseek(fd_alias, 0, SEEK_SET);
		if (res < 0) {
		    close(fd_alias);
		    return clientAdminAliasFileSeekErr;
	    }
		// truncate the file to 0 byte
	    res = ftruncate(fd_alias, 0);
	    if (res < 0) {
		    close(fd_alias);
		    return clientAdminAliasFileTruncErr;
	    }
	    res = writen(fd_alias, new_alias_str, len);
	    if (res != len) {
			tries--;		    
	    }
		else {
			break;
		}
	}
	if (tries == 0) {
	    close(fd_alias);
		return clientAdminAliasFileWriteErr;
	}
	
	close(fd_alias); //also unlock

	// Send IPC msg
	mqid = msgget(CLIENTADMIN_MQ_KEY, 0);
	if (mqid == -1) {
		CA_DEBUG("msgget error %d:%s\n", mqid, strerror(errno));
	}
	else {
		len = construct_msg(clientAdmintMsgAlias, new_alias_str, &mesg);
		if (len > 0) {
			res = msgsnd(mqid, &mesg, len, 0);
			if (res == -1) {
				CA_DEBUG("msgsnd error %s\n", strerror(errno));
			}
		}		
	}
	return 0;
}

/***************************************************************************
  Function: construct_msg
  Description: construct IPC message
  Input:  msgtype
             text_str
  Output: pmsg
  Return: negative:error
              positive:the length of message data, including null
  Others:  none
***************************************************************************/
static int construct_msg(clientAdminMsgType msgtype, const char *text_str, struct client_admin_msgbuf *pmsg)
{
    int re;
	char *out;
	cJSON *msg_json;

    if ( (msgtype < clientAdmintMsgPassword) || (msgtype > clientAdmintMsgAlias) ) {
		CA_DEBUG("msgtype error %d\n", msgtype);
		return -1;
    }
	if (text_str == NULL) {
		CA_DEBUG("text_str NULL\n");
		return -1;
    }

    pmsg->mtype = msgtype;
	msg_json = cJSON_CreateObject();
	if (!msg_json) {
		CA_DEBUG("Create msg_json failed\n");
		return -2;
	}
	switch (msgtype) {
		case clientAdmintMsgPassword:
		    cJSON_AddStringToObject(msg_json, "password", text_str);
			break;
	    case clientAdmintMsgAlias:
	        cJSON_AddStringToObject(msg_json, "alias", text_str);
			break;
		default: // already check, just fo completed
			cJSON_Delete(msg_json);
			CA_DEBUG("msgtype error %d\n", msgtype);
			return -2;
	}
	out = cJSON_PrintUnformatted(msg_json);
	cJSON_Delete(msg_json);
	if (!out) {
		CA_DEBUG("cJSON_PrintUnformatted msg_json failed\n");
		return -3;
	}
	re = snprintf(pmsg->mtext, CLIENTADMIN_MSG_LEN, "%s", out);	
	free(out);

	return (re+1); //including null
}
