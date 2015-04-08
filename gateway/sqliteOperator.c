/*
 * sqliteOperator.c
 *
 *  Created on: Mar 31, 2015
 *      Author: yanly
 */

#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#include "sqliteOperator.h"


#define ERROR_OPENED_DB					-1
#define ERROR_ACCESS_DB					-2
#define ERROR_READ_DB					-3
#define ERROR_WRITE_DB					-4
#define ERROR_MDFY_NO_ID_DB 			-5
#define	ERROR_GENERATE_URL_STRING		-6


sqlite3 * db=NULL;

/*
 * function: t_time_action_create()
 * description: create the table of timing
 * input:
 * output:
 * return: 0-ok, -1-failed
 * others:
 * */
int t_time_action_create()
{
	int result;
    int ret = 0;
    char* errmsg=NULL;

    const char *sql="create table if not exists t_time_action(tid integer primary key autoincrement, \
    		actname nvarchar(30),acttype char(1),actpara varchar(40),actmode char(1),para1 varchar(14),\
    		para2 char(1),para3 char(7),enable char(1),urlstring varchar(200),actobj char(16))";

    //表不存在就建表
    result = sqlite3_exec(db,sql, NULL, NULL, &errmsg);
	if(result != SQLITE_OK )
	{
		URLAPI_DEBUG("create table failed:%s\n",errmsg);
		ret = ERROR_ACCESS_DB;
	}
    sqlite3_free(errmsg);
    return ret;
}
/*
 * function: t_scene_create()
 * description: create the table of scene
 * input:
 * output:
 * return: 0-ok, -1-failed
 * others:
 * */
int t_scene_create()
{
	int result;
    int ret = 0;
    char* errmsg=NULL;

    const char *sql="create table if not exists t_scene(sid integer primary key autoincrement,scnname nvarchar(30),\
    		scnindex int(1),scnaction varchar(100))";

    //表不存在就建表
    result = sqlite3_exec(db,sql, NULL, NULL, &errmsg);
	if(result != SQLITE_OK )
	{
		URLAPI_DEBUG("create table failed:%s\n",errmsg);
		ret = ERROR_ACCESS_DB;
	}
    sqlite3_free(errmsg);
    return ret;
}
/*
 * function: t_scene_act_create()
 * description: create the table of scene action
 * input:
 * output:
 * return: 0-ok, -1-failed
 * others:
 * */
int t_scene_act_create()
{
	int result;
    int ret = 0;
    char* errmsg=NULL;

    const char *sql="create table if not exists t_scene_act(sid integer(1),urlstring varchar(200),actobj char(16))";

    //表不存在就建表
    result = sqlite3_exec(db,sql, NULL, NULL, &errmsg);
	if(result != SQLITE_OK )
	{
		URLAPI_DEBUG("create table failed:%s\n",errmsg);
		ret = ERROR_ACCESS_DB;
	}
    sqlite3_free(errmsg);
    return ret;
}
/*
 * function: t_linkage_create()
 * description: create the table of linkage
 * input:
 * output:
 * return: 0-ok, -1-failed
 * others:
 * */
int t_linkage_create()
{
	int result;
    int ret = 0;
    char* errmsg=NULL;

    const char *sql="create table if not exists t_linkage(lid integer primary key autoincrement,lnkname nvarchar(30),\
    		trgieee char(16),trgep char(2),trgcnd varchar(30),lnkact varchar(40),enable char(1),\
    		actobj char(16),urlstring varchar(200))";

    //表不存在就建表
    result = sqlite3_exec(db,sql, NULL, NULL, &errmsg);
	if(result != SQLITE_OK )
	{
		URLAPI_DEBUG("create table failed:%s\n",errmsg);
		ret = ERROR_ACCESS_DB;
	}
    sqlite3_free(errmsg);
    return ret;
}
/*
 * function: db_init()
 * description: init the datebase of timing,scene,linkage
 * input:
 * output:
 * return:	0-ok, <0-failed
 * others:
 * */
int db_init()
{
	int result;
	int ret =0;  //init ok

    result=sqlite3_open(FEATURE_GDGL_TIME_SCENE_LINKAGE_DB, &db);
	if(result !=SQLITE_OK)
	{
		//数据库打开失败
		URLAPI_DEBUG("open database failed,fail value:%d\n", result);
		ret = ERROR_OPENED_DB;
	}

    return ret;
}
/*
 * function: db_close()
 * description: close the datebase of timing,scene,linkage
 * input:
 * output:
 * return:
 * others:
 * */
void db_close()
{
	sqlite3_close(db);
}
/*=========================================================================================*/  //scene operator
/*
 * function: t_scene_insert()
 * description: insert scene
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int t_scene_insert(scene_base_st* base, sqlite3 *scene_db)			//table: t_scene
{
	int res;
	sqlite3_stmt* stmt = NULL;

	const char* insertSQL = "INSERT INTO t_scene VALUES (null,'%s', %d, '%s')";
	char sql[256];

	//插入新场景记录
	sprintf(sql, insertSQL, base->scnname, base->scnindex, base->scnaction);

	if (sqlite3_prepare_v2(scene_db, sql, strlen(sql), &stmt, NULL) != SQLITE_OK) {	//构建插入数据的sqlite3_stmt对象
		if(stmt)
			sqlite3_finalize(stmt);
		return (res = ERROR_WRITE_DB);
	}
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		sqlite3_finalize(stmt);
		return (res = ERROR_WRITE_DB);
	}
	sqlite3_finalize(stmt);

	//获取刚刚插入的场景id
	const char *selectsql = "SELECT last_insert_rowid() newid";
	sqlite3_stmt* stmt2 = NULL;

	if (sqlite3_prepare_v2(scene_db, selectsql, strlen(selectsql), &stmt2, NULL) != SQLITE_OK){
		sqlite3_finalize(stmt2);
		return (res = ERROR_WRITE_DB);
	}

//	int fieldCount = sqlite3_column_count(stmt2);
	int r = sqlite3_step(stmt2);

	do{
		if(r == SQLITE_ROW) {
//			int vtype = sqlite3_column_type(stmt2, 0);
			int v = sqlite3_column_int(stmt2, 0);
			base->sid = v;
//			URLAPI_DEBUG("insert sid = %d", base->sid);
			break;
		}
		else if(r == SQLITE_DONE){
			break;
		}
		else{
			sqlite3_finalize(stmt2);
			return (res = ERROR_WRITE_DB);
		}
	}while(1);
	sqlite3_finalize(stmt2);

	//over
	return (res = 0);
}
/*
 * function: t_scene_action_insert()
 * description: 高效的批量数据插入
    1). 通过执行BEGIN TRANSACTION语句手工开启一个事物。
    2). 准备插入语句及相关的绑定变量。
    3). 迭代式插入数据。
    4). 完成后通过执行COMMIT语句提交事物。

    说明: 基于变量绑定的方式准备待插入的数据，这样可以节省大量的sqlite3_prepare_v2函数调用次数，从而节省了多次将同一SQL语句编译成SQLite内部识别的字节码所用的时间。
    事实上，SQLite的官方文档中已经明确指出，在很多时候sqlite3_prepare_v2函数的执行时间要多于sqlite3_step函数的执行时间，
    因此建议使用者要尽量避免重复调用sqlite3_prepare_v2函数。在我们的实现中，如果想避免此类开销，只需将待插入的数据以变量的形式绑定到SQL语句中，
    这样该SQL语句仅需调用sqlite3_prepare_v2函数编译一次即可，其后的操作只是替换不同的变量数值。
    在完成所有的数据插入后显式的提交事物。提交后，SQLite会将当前连接自动恢复为自动提交模式。
 * input:
 * output:
 * return:	0-ok, <0-failed
 * others:
 * */
int t_scene_action_insert(const scene_action_stPtr action, int sid, sqlite3 *scene_db)		//tabel: t_scene_act
{
	int res;

	//1). 通过执行BEGIN TRANSACTION语句手工开启一个事物
	sqlite3_stmt* stmt_begin = NULL;
	const char* beginSQL = "BEGIN TRANSACTION";
	if (sqlite3_prepare_v2(scene_db, beginSQL, strlen(beginSQL), &stmt_begin, NULL) != SQLITE_OK) {
		if(stmt_begin)
			sqlite3_finalize(stmt_begin);
		return (res = ERROR_WRITE_DB);
	}
	if (sqlite3_step(stmt_begin) != SQLITE_DONE) {
		sqlite3_finalize(stmt_begin);
		return (res = ERROR_WRITE_DB);
	}
	sqlite3_finalize(stmt_begin);

	//2). 准备插入语句及相关的绑定变量。
	sqlite3_stmt* stmt = NULL;
	const char* insertSQL = "INSERT INTO t_scene_act VALUES (?, ?, ?)"; //sid, urlstring, actobj
	if (sqlite3_prepare_v2(scene_db, insertSQL, strlen(insertSQL), &stmt, NULL) != SQLITE_OK) {	//构建插入数据的sqlite3_stmt对象
		if(stmt)
			sqlite3_finalize(stmt);
		return (res = ERROR_WRITE_DB);
	}

	//3). 基于已有的SQL语句，迭代的绑定不同的变量数据
	int insertCount = action[0].acttotal;		//插入多少条记录
	int i;
	for(i=0; i < insertCount; i++) {
		//在绑定时，最左面的变量索引值是1。
		sqlite3_bind_int(stmt, 1, sid);
		sqlite3_bind_text(stmt, 2, action[i].urlstring, strlen(action[i].urlstring), SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 3, action[i].actobj, strlen(action[i].actobj), SQLITE_TRANSIENT);
		if (sqlite3_step(stmt) != SQLITE_DONE) {
			sqlite3_finalize(stmt);
			return (res = ERROR_WRITE_DB);
		}
		//重新初始化该sqlite3_stmt对象绑定的变量。
		 sqlite3_reset(stmt);
	}
	sqlite3_finalize(stmt);

	//4). 完成后通过执行COMMIT语句提交事物。
	const char* commitSQL = "COMMIT";
    sqlite3_stmt* stmt_commit = NULL;
      if (sqlite3_prepare_v2(scene_db, commitSQL, strlen(commitSQL), &stmt_commit, NULL) != SQLITE_OK) {
          if (stmt_commit)
              sqlite3_finalize(stmt_commit);
          return (res = ERROR_WRITE_DB);
      }
      if (sqlite3_step(stmt_commit) != SQLITE_DONE) {
          sqlite3_finalize(stmt_commit);
          return (res = ERROR_WRITE_DB);
      }
      sqlite3_finalize(stmt_commit);

      //over
      return (res = 0);
}
/*
 * function: add_scene_db()
 * description: add_scene_db
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int add_scene_db(scene_base_st* base, const scene_action_stPtr action)
{
	int res;

	//准备
	res = db_init();
	if(res<0){
		db_close();
		return res;
	}
	res = t_scene_create();
	if(res<0){
		db_close();
		return res;
	}
	res = t_scene_act_create();
	if(res<0){
		db_close();
		return res;
	}

	//写
	res = t_scene_insert(base, db);
	if(res<0){
		db_close();
		return res;
	}
	res = t_scene_action_insert(action, base->sid, db);
	if(res<0){
		db_close();
		return res;
	}

	//关闭数据库
	db_close();
	return (res = 0);
}

