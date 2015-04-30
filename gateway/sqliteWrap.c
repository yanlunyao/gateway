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
	sqlite3_stmt* stmt = NULL;

	if (sqlite3_prepare_v2(target_db, sql, strlen(sql), &stmt, NULL) != SQLITE_OK) {	//构建插入数据的sqlite3_stmt对象
		if(stmt)
			sqlite3_finalize(stmt);
		return (res = ERROR_ACCESS_DB);
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

	res = t_standard_by_stmt(db_target, sql);
	if(res <0)
		return res;

	row_affected = sqlite3_changes(db_target);  //This function returns the number of rows modified 如果没被影响说明没这个id
	if(row_affected <=0){
		URLAPI_DEBUG("update failed, row affected is : %d", row_affected);
		return (res = ERROR_MDFY_NO_ID_DB);
	}
	return (res = 0);
}



