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



int read_lock_file_data(const char *path, char *data);
int write_lock_file_data(const char *path, const char *data);

#endif /* GATEWAY_MYFILE_OPERATION_H_ */
