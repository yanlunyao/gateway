/*
 *	File name   : sqliteWrap.h
 *  Created on  : Apr 27, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#ifndef SQLITEWRAP_H_
#define SQLITEWRAP_H_

#include "sqlite3.h"

#define DB_ALLOW_HANDLE_TIME		500 //500ms
#define DB_BUSY_WAIT_TIME			5	//5ms
#define DB_BUSY_REPEAT_CNT			(DB_ALLOW_HANDLE_TIME/DB_BUSY_WAIT_TIME)

#define BEAGIN_TRANSCATION				"BEGIN TRANSACTION"
#define COMMIT_TRANSCATION				"COMMIT"
#define ROLLBACK						"ROLLBACK"

int t_standard_by_stmt(sqlite3 *target_db, const char *sql);
int t_insert_retid(sqlite3 *db_target, const char *sql, int *id);
int t_update_delete_and_change_check(sqlite3 *db_target, const char *sql);
int t_standard_beagin_transaction(sqlite3 *db_target);
int t_standard_commit_transaction(sqlite3 *db_target);

#endif /* SQLITEWRAP_H_ */
