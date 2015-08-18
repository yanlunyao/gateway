/*
 *	File name   : getAuthState.c
 *  Created on  : Aug 8, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "cJSON.h"
#include "cgic.h"
#include "smartgateway.h"


char *response[] = {"ok", "open file failed", "file data not json"};
struct flock* file_lock(short type, short whence)
{
    static struct flock ret;
    ret.l_type = type ;
    ret.l_start = 0;
    ret.l_whence = whence;
    ret.l_len = 0;
    ret.l_pid = getpid();
    return &ret;
}

int cgiMain()
{
	int res=0;

	cJSON* json_all;
    FILE* fp;
    int fd;
    int file_size;
    char file_buf[1024];
    char* tmp;

    cgiHeaderContentType("application/json"); //MIME

    //read sw_version
    fp = fopen(FEATURE_GDGL_AUTH_PATH, "a+");
    if (fp == NULL)
    {
        fclose(fp);
        res = 1;
        goto over;
    }

    fd = fileno(fp);
    fcntl(fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    if (file_size > 0)
    {
        fread(file_buf, sizeof(char), file_size, fp);
        json_all = cJSON_Parse(file_buf);
        if(!json_all)
        {
        	res = 2;
            fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
            fclose(fp);
        	goto over;
        }
        file_buf[file_size] = '\0';
        fprintf(cgiOut,"%s\n", file_buf);
    }
    else
    {
        json_all = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_all, "state", 0);
        cJSON_AddStringToObject(json_all, "expire_time", "2000-00-00 00:00:00");
        cJSON_AddNumberToObject(json_all, "available", 1);
        cJSON_AddNumberToObject(json_all, "remind_before_expire", 3);
        cJSON_AddNumberToObject(json_all, "remind_after_expire", 3);
        tmp = cJSON_Print(json_all);
        fwrite(tmp, sizeof(char), strlen(tmp), fp);
        fprintf(cgiOut,"%s\n", tmp);
        free(tmp);
    }
    fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
    fclose(fp);
    cJSON_Delete(json_all);

over:
	if(res!=0)
		fprintf(cgiOut,"{\"status\":\"%s\"}\n", response[res]);
	return 0;
}
