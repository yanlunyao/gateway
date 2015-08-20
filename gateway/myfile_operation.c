/*
 *	File name   : myfile_operation.c
 *  Created on  : Aug 19, 2015
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
#include "myfile_operation.h"

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

int read_lock_file_data(const char *path, char *data)
{
	int res =0;

    FILE* fp;
    int fd;
    int file_size;
    char file_buf[1024];

    fp = fopen(path, "a+");
	if (fp == NULL)
	{
		fclose(fp);
		res = OPENED_FILE_FAILED;
		return res;
	}

	fd = fileno(fp);
	fcntl(fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);

	if (file_size > 0)
	{
		fread(file_buf, sizeof(char), file_size, fp);
		file_buf[file_size] = '\0';
		memcpy(data, file_buf, file_size+1);
	}
	else
	{
		res = FILE_NOT_DATA;
	}
	fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
	fclose(fp);

	return res;
}
int write_lock_file_data(const char *path, const char *data)
{
	int res =0;

    FILE* fp;
    int fd;
    int file_size;

    fp = fopen(path, "a+");
	if (fp == NULL)
	{
		fclose(fp);
		res = OPENED_FILE_FAILED;
		return res;
	}

	fd = fileno(fp);
	fcntl(fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));
	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	rewind(fp);
	if (file_size > 0)
	{
		if(truncate(path, 0)<0) {  //清空文件内容
			perror("truncate failed\n");
			res = FILE_WRITE_DATA_FAILED;
		}
		else
			fwrite(data, sizeof(char), strlen(data)+1, fp);
	}

	fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
	fclose(fp);

	return res;
}
