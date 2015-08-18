/*
 *	File name   : GetRFDevList.c
 *  Created on  : Aug 3, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

/*
 *	File name   : getHwVersion.c
 *  Created on  : Jul 15, 2015
 *  Author      : yanly
 *  Description : 获取网关硬件版本信息
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

char *response[] = {"ok", "open file failed", "file data not json", "file data is empty"};
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
//    char file_buf[1024];
    char *file_buf;

    cgiHeaderContentType("application/json"); //MIME

    //read sw_version
    fp = fopen(FEATURE_GDGL_RF_DEV_PATH, "r");  //已只读的方式打开文件
    if (fp == NULL)
    {
//        fclose(fp);
        res = 1;
        goto over;
    }

    fd = fileno(fp);
    fcntl(fd, F_SETLKW, file_lock(F_RDLCK, SEEK_SET));  //共享锁，只读用

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    if (file_size > 0)
    {
    	file_buf = malloc(file_size+1);   //分配内存
        fread(file_buf, sizeof(char), file_size, fp);
        json_all = cJSON_Parse(file_buf);
        if(!json_all)
        {
        	res = 2;
        	fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
        	fclose(fp);
        	free(file_buf);  //释放内存
        	goto over;
        }
        file_buf[file_size] = '\0';
        fprintf(cgiOut,"%s\n", file_buf);
        free(file_buf);		//释放内存
    }
    else
    {
        // 如果文件为空，返回告诉文件为空
    	fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
		fclose(fp);
		res = 3;
		goto over;
    }
    fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
    fclose(fp);
    cJSON_Delete(json_all);

over:
	if(res!=0)
		fprintf(cgiOut,"{\n	\"status\": \"%s\"\n}\n", response[res]);
	return 0;
}
