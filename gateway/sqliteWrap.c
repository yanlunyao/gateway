/*
 *	File name   : sqliteWrap.c
 *  Created on  : Apr 27, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "sqliteWrap.h"
#include "smartgateway.h"
/*
 * function: t_standard_by_stmt()
 * description: 根据数据库名, 标准sql语句,执行标准对象操作
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int t_standard_by_stmt(sqlite3 *target_db, const char *sql)
{
	int res;
	int rc;
	int repeat_cnt=0;
	sqlite3_stmt* stmt = NULL;
//	printf("sql=%s\n",sql);
	if ((rc = sqlite3_prepare_v2(target_db, sql, strlen(sql), &stmt, NULL)) != SQLITE_OK) {	//构建插入数据的sqlite3_stmt对象

		while(rc == SQLITE_BUSY) {
			repeat_cnt++;
			if(repeat_cnt >DB_BUSY_REPEAT_CNT)
				break;
			usleep(DB_BUSY_WAIT_TIME*1000);
			rc = sqlite3_prepare_v2(target_db, sql, strlen(sql), &stmt, NULL);
		}
		if(rc != SQLITE_OK) {
			if(stmt)
				sqlite3_finalize(stmt);
			return (res = ERROR_ACCESS_DB);
		}
	}

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		sqlite3_finalize(stmt);
		return (res = ERROR_WRITE_DB);
	}
	sqlite3_finalize(stmt);

	return (res = 0);
}
/*=========================================================================================*/  //scene operator
/*
 * function: t_insert_retid()
 * description: 根据数据库名, 标准sql语句插入数据行，并输出自增栏的自增id
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int t_insert_retid(sqlite3 *db_target, const char *sql, int *id)
{
	int res;
	res = t_standard_by_stmt(db_target, sql);
	if(res <0)
		return res;

	*id = sqlite3_last_insert_rowid(db_target);
	URLAPI_DEBUG("insert id = %d", *id);
	//over
	return (res = 0);
}
/*
 * function: t_update_delete_and_change_check()
 * description: 根据数据库名，表名，标准sql语句(更新/删除)指定行，并且校验有没有生效
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others: sqlite3_changes: This function returns the number of rows modified,inserted or deleted by the most recently completed
							INSERT, UPDATE or DELETE statement on the database connection specified by the only parameter.
							Executing any other type of SQL statement does not modify the value returned by this function.
 * */
int t_update_delete_and_change_check(sqlite3 *db_target, const char *sql)
{
	int res;
	int row_affected;
//	printf("sql=%s\n",sql);
	res = t_standard_by_stmt(db_target, sql);
	if(res <0)
		return res;

	row_affected = sqlite3_changes(db_target);  //This function returns the number of rows modified 如果没被影响说明没这个id
	if(row_affected <=0){
		URLAPI_DEBUG("update failed, row affected is : %d", row_affected);
//		printf("update failed, row affected is : %d", row_affected);
		return (res = ERROR_MDFY_NO_ID_DB);
	}
	return (res = 0);
}
/*
 * function: t_standard_beagin_transaction()
 * description: 调用t_standard_by_stmt开启一个事务
 */
int t_standard_beagin_transaction(sqlite3 *db_target)
{
	int res;
	//1). 通过执行BEGIN TRANSACTION语句手工开启一个事物
	const char* beginSQL = "BEGIN TRANSACTION";

	res = t_standard_by_stmt(db_target, beginSQL);
	if(res <0)
		return res;
	return 0;
}
/*
 * function: t_standard_beagin_transaction()
 * description: 调用t_standard_by_stmt提交一个事务
 */
int t_standard_commit_transaction(sqlite3 *db_target)
{
	int res;
	//4). 完成后通过执行COMMIT语句提交事物。
	const char* commitSQL = "COMMIT";

	res = t_standard_by_stmt(db_target, commitSQL);
	if(res <0)
		return res;
	return 0;
}
