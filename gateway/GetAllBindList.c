/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName:      GetAllBindList.c
  Author:        马晓槟
  Version :      1.0   
  Date:          2014-09-26
  Description:   返回bindlist.json保存的设备绑定信息。
                 如果bindlist.json读取失败则返回：
                 {"error_code":1,"error_msg":"bind list read error"}
  History:        
      <author>  <time>   <version >   <desc>
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


#define BUF_SIZE            1024*8
#define BIND_LIST_FILE      "/gl/etc/bindlist.json"

const char RT_Error[] = "{\"error_code\":1,\"error_msg\":\"bind list read error\"}";

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

int main()
{
    FILE* fp;
    int fd;
    int size;
    char buf[BUF_SIZE] = {0};

    printf("Content-type: text/html\n\n");      // CGI返回头

    fp = fopen(BIND_LIST_FILE, "r");
    if (fp == NULL)
    {
//        close(fp);
        printf("%s\n", RT_Error);
        perror("open bind list error");
        return -1;
    }

    fd = fileno(fp);
    fcntl(fd, F_SETLKW, file_lock(F_RDLCK, SEEK_SET));

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);
    rewind(fp);

    fread(buf, sizeof(char), size, fp);
    printf("%s\n", buf);

    fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));       
    fclose(fp);

    return 0;
}

