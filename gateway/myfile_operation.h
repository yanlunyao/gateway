/*
 *	File name   : myfile_operation.h
 *  Created on  : Aug 19, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef GATEWAY_MYFILE_OPERATION_H_
#define GATEWAY_MYFILE_OPERATION_H_

#define READ_FILE_OK							0
#define OPENED_FILE_FAILED						-1
#define FILE_DATA_NOT_JSON						-2
#define FILE_NOT_DATA							-3
#define FILE_WRITE_DATA_FAILED					-4


/*
 * description: 加锁读取文件内的所有内容。
 * in： path--文件名（包含路径），data--获取到的数据存放区
 * out：	0--success，<0--failed
 * */
int read_lock_file_data(const char *path, char *data);
/*
 * description: 加锁，清空文件内容后写新的内容进文件。
 * in： path--文件名（包含路径），data--待写的数据存放区
 * out：	0--success，<0--failed
 * */
int write_lock_file_data(const char *path, const char *data);

#endif /* GATEWAY_MYFILE_OPERATION_H_ */
