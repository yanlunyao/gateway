/*
 * sqliteOperator.c
 *
 *  Created on: Mar 31, 2015
 *      Author: yanly
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "sqliteOperator.h"

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

/*
 * function: t_standard_by_stmt()
 * description: 根据数据库名, 标准sql语句,执行标准对象操作
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int t_standard_by_stmt(sqlite3 *db, const char *sql)
{
	int res;
	sqlite3_stmt* stmt = NULL;

	if (sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL) != SQLITE_OK) {	//构建插入数据的sqlite3_stmt对象
		if(stmt)
			sqlite3_finalize(stmt);
		return (res = ERROR_ACCESS_DB);
	}
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		sqlite3_finalize(stmt);
		return (res = ERROR_ACCESS_DB);
	}
	sqlite3_finalize(stmt);

	return (res = 0);
}
/*=========================================================================================*/  //scene operator
/*
// * function: t_scene_insert()
// * description: insert scene
// * input:
// * output:
// * return: 0-ok, <0-failed
// * others:
// * */
//int t_scene_insert(scene_base_st* base, sqlite3 *scene_db)			//table: t_scene
//{
//	int res;
//	sqlite3_stmt* stmt = NULL;
//
//	const char* insertSQL = "INSERT INTO t_scene VALUES (null,'%s', %d, '%s')";
//	char sql[256];
//
//	//插入新场景记录
//	sprintf(sql, insertSQL, base->scnname, base->scnindex, base->scnaction);
//
//	if (sqlite3_prepare_v2(scene_db, sql, strlen(sql), &stmt, NULL) != SQLITE_OK) {	//构建插入数据的sqlite3_stmt对象
//		if(stmt)
//			sqlite3_finalize(stmt);
//		return (res = ERROR_WRITE_DB);
//	}
//	if (sqlite3_step(stmt) != SQLITE_DONE) {
//		sqlite3_finalize(stmt);
//		return (res = ERROR_WRITE_DB);
//	}
//	sqlite3_finalize(stmt);
//
//	//获取刚刚插入的场景id
////	const char *selectsql = "SELECT last_insert_rowid() newid";
////	sqlite3_stmt* stmt2 = NULL;
////
////	if (sqlite3_prepare_v2(scene_db, selectsql, strlen(selectsql), &stmt2, NULL) != SQLITE_OK){
////		sqlite3_finalize(stmt2);
////		return (res = ERROR_WRITE_DB);
////	}
////
//////	int fieldCount = sqlite3_column_count(stmt2);
////	int r = sqlite3_step(stmt2);
////
////	do{
////		if(r == SQLITE_ROW) {
//////			int vtype = sqlite3_column_type(stmt2, 0);
////			int v = sqlite3_column_int(stmt2, 0);
////			base->sid = v;
////			break;
////		}
////		else if(r == SQLITE_DONE){
////			break;
////		}
////		else{
////			sqlite3_finalize(stmt2);
////			return (res = ERROR_WRITE_DB);
////		}
////	}while(1);
////	sqlite3_finalize(stmt2);
//
//	base->sid = sqlite3_last_insert_rowid(scene_db);
//	URLAPI_DEBUG("insert sid = %d", base->sid);
//	//over
//	return (res = 0);
//}
/*
 * function: t_insert_retid()
 * description: 根据数据库名, 标准sql语句插入数据行，并输出自增栏的自增id
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int t_insert_retid(sqlite3 *db_target, const char *sql, int *id)			//table: t_scene
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
 * function: t_scene_update()
 * description: update scene
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others: sqlite3_changes: This function returns the number of rows modified,inserted or deleted by the most recently completed
							INSERT, UPDATE or DELETE statement on the database connection specified by the only parameter.
							Executing any other type of SQL statement does not modify the value returned by this function.
 * */
//int t_scene_update(const scene_base_st* base, sqlite3 *scene_db)			//table: t_scene
//{
//	int res;
//	sqlite3_stmt* stmt = NULL;
//	int row_affected;
//
//	const char* updateSQL = "UPDATE t_scene SET scnname= '%s', scnindex = %d, "
//			"scnaction = '%s' WHERE sid = %d";
//	char sql[256];
//
//	sprintf(sql, updateSQL, base->scnname, base->scnindex, base->scnaction, base->sid);
//
//	if (sqlite3_prepare_v2(scene_db, sql, strlen(sql), &stmt, NULL) != SQLITE_OK) {	//构建插入数据的sqlite3_stmt对象
////		URLAPI_DEBUG("update failed: %s", sqlite3_errmsg(scene_db));
//		if(stmt)
//			sqlite3_finalize(stmt);
//		return (res = ERROR_WRITE_DB);
//	}
//	if (sqlite3_step(stmt) != SQLITE_DONE) {
//		sqlite3_finalize(stmt);
//		return (res = ERROR_WRITE_DB);
//	}
//	sqlite3_finalize(stmt);
//
//	row_affected = sqlite3_changes(scene_db);  //This function returns the number of rows modified 如果没被影响说明没这个sid
//	if(row_affected <=0){
//		URLAPI_DEBUG("update failed, row affected is : %d", row_affected);
//		return (res = ERROR_MDFY_NO_ID_DB);
//	}
//	return (res = 0);
//}
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
int t_scene_action_insert(const scene_action_stPtr action, int sid, sqlite3 *db_target)		//tabel: t_scene_act
{
	int res;

	//1). 通过执行BEGIN TRANSACTION语句手工开启一个事物
	const char* beginSQL = "BEGIN TRANSACTION";

	res = t_standard_by_stmt(db_target, beginSQL);
	if(res <0)
		return res;

	//2). 准备插入语句及相关的绑定变量。
	sqlite3_stmt* stmt = NULL;
	const char* insertSQL = "INSERT INTO t_scene_act VALUES (?, ?, ?)"; //sid, urlstring, actobj
	if (sqlite3_prepare_v2(db_target, insertSQL, strlen(insertSQL), &stmt, NULL) != SQLITE_OK) {	//构建插入数据的sqlite3_stmt对象
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

	res = t_standard_by_stmt(db_target, commitSQL);
	if(res <0)
		return res;

      //over
      return (res = 0);
}
/*
 * function: t_scene_index_modify()
 * description: 不检查表里id是否存在，不存在则插入自动不成功，检查过多效率会变慢, 但是没有返回错误。
 * output:
 * return:	0-ok, <0-failed
 * others:
 * */
int t_scene_index_modify(sqlite3 *db_target, const int *id, const int *index, int cnt)
{
	int res;

	//1). 通过执行BEGIN TRANSACTION语句手工开启一个事物
	const char* beginSQL = "BEGIN TRANSACTION";
	res = t_standard_by_stmt(db_target, beginSQL);
	if(res <0)
		return res;

	//2). 准备插入语句及相关的绑定变量。
	sqlite3_stmt* stmt = NULL;
	const char* insertSQL = "UPDATE t_scene SET scnindex = ? WHERE sid = ?";
	if (sqlite3_prepare_v2(db_target, insertSQL, strlen(insertSQL), &stmt, NULL) != SQLITE_OK) {	//构建插入数据的sqlite3_stmt对象
		if(stmt)
			sqlite3_finalize(stmt);
		return (res = ERROR_WRITE_DB);
	}

	//3). 基于已有的SQL语句，迭代的绑定不同的变量数据
	int i;
	for(i=0; i < cnt; i++) {
		//在绑定时，最左面的变量索引值是1。
		sqlite3_bind_int(stmt, 1, index[i]);
		sqlite3_bind_int(stmt, 2, id[i]);
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
	res = t_standard_by_stmt(db_target, commitSQL);
	if(res <0)
		return res;

      //over
     return (res = 0);
}
///*
// * function: t_scene_delete()
// * description:
// * input:
// * output:
// * return:	0-ok, <0-failed
// * others:
// * */
//int t_scene_delete(int sid, sqlite3 *scene_db)		//tabel: t_scene
//{
//	int res;
//	sqlite3_stmt* stmt = NULL;
//	int row_affected;
//
//	const char* deleteSQL = "DELETE FROM t_scene WHERE sid = %d";
//	char sql[256];
//
//	sprintf(sql, deleteSQL, sid);
//
//	if (sqlite3_prepare_v2(scene_db, sql, strlen(sql), &stmt, NULL) != SQLITE_OK) {
//		URLAPI_DEBUG("delete failed: %s", sqlite3_errmsg(scene_db));
//		if(stmt)
//			sqlite3_finalize(stmt);
//		return (res = ERROR_WRITE_DB);
//	}
//	if (sqlite3_step(stmt) != SQLITE_DONE) {
//		sqlite3_finalize(stmt);
//		return (res = ERROR_WRITE_DB);
//	}
//	sqlite3_finalize(stmt);
//
//	row_affected = sqlite3_changes(scene_db);  //This function returns the number of rows modified 如果没被影响说明没这个sid
//	if(row_affected <=0){
//		URLAPI_DEBUG("delete failed, row affected is : %d", row_affected);
//		return (res = ERROR_MDFY_NO_ID_DB);
//	}
//
//	return 0;
//}
/*
 * function: t_delete()
 * description: 根据数据库，表名，id段名和id值删除所在行
 * input:
 * output:
 * return:	0-ok, <0-failed
 * others:
 * */
int t_delete(sqlite3 *db, const char *table_name, const char *id_field, int id_value)		//tabel: table_name
{
	int res;
	sqlite3_stmt* stmt = NULL;
	int row_affected;

	const char* deleteSQL = "DELETE FROM %s WHERE %s = %d";
	char sql[256];

	sprintf(sql, deleteSQL, table_name, id_field, id_value);
	URLAPI_DEBUG("sql:%s\n", sql);

	if (sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL) != SQLITE_OK) {
		URLAPI_DEBUG("delete failed: %s", sqlite3_errmsg(db));
		if(stmt)
			sqlite3_finalize(stmt);
		return (res = ERROR_WRITE_DB);
	}
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		sqlite3_finalize(stmt);
		return (res = ERROR_WRITE_DB);
	}
	sqlite3_finalize(stmt);

	row_affected = sqlite3_changes(db);  //This function returns the number of rows modified 如果没被影响说明没这个id
	if(row_affected <=0){
		URLAPI_DEBUG("delete failed, row affected is : %d", row_affected);
		return (res = ERROR_MDFY_NO_ID_DB);
	}

	return 0;
}
///*
// * function: t_scene_action_delete()
// * description:
// * input:
// * output:
// * return:	0-ok, <0-failed
// * others:
// * */
//int t_scene_action_delete(int sid, sqlite3 *scene_db)		//tabel: t_scene_act
//{
//	int res;
//	sqlite3_stmt* stmt = NULL;
//
//	const char* deleteSQL = "DELETE FROM t_scene_act WHERE sid = %d";
//	char sql[256];
//
//	sprintf(sql, deleteSQL, sid);
//
//	if (sqlite3_prepare_v2(scene_db, sql, strlen(sql), &stmt, NULL) != SQLITE_OK) {	//构建插入数据的sqlite3_stmt对象
//		URLAPI_DEBUG("delete failed: %s", sqlite3_errmsg(scene_db));
//		if(stmt)
//			sqlite3_finalize(stmt);
//		return (res = ERROR_WRITE_DB);
//	}
//	if (sqlite3_step(stmt) != SQLITE_DONE) {
//		sqlite3_finalize(stmt);
//		return (res = ERROR_WRITE_DB);
//	}
//	sqlite3_finalize(stmt);
//
//	return 0;
//}
/*
 * function: t_scene_getlist()
 * description:
 * input:
 * output:
 * return:	0-ok, <0-failed
 * others:
 * */
int t_scene_getlist(scene_base_list_st *scene_base_list, sqlite3 *db)
{
	int res;

	char *errmsg;
	char **result;
	int j,i;
	int row, col, index;

	const char *querySql = "SELECT *FROM t_scene order by sid";

    if( sqlite3_get_table(db, querySql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
		URLAPI_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }

    URLAPI_DEBUG("row=%d,col=%d\n",row,col);

//    //generate scene_base
//    if(row ==0){
//    	return (res = ERROR_NO_LIST);
//    }
    scene_base_list->scene_base = (scene_base_stPtr)malloc(row*sizeof(scene_base_st)); //need to free in outside
    if(scene_base_list->scene_base == NULL){
    	 URLAPI_DEBUG("list is NULL\n");
    	sqlite3_free_table(result);
    	return (res = ERROR_OTHER);
    }

	index = 0;
	for(i=0; i<row; i++)
	{
		for(j=0; j<col; j++)
		{
			URLAPI_DEBUG("%s\n", result[index]);
			index++;
		}
		URLAPI_DEBUG("\n");
	}

	index = col;
	scene_base_list->list_total = row;
    for(j=0; j<row; j++){
    	scene_base_list->scene_base[j].sid = atoi(result[index]);
    	strcpy(scene_base_list->scene_base[j].scnname, result[index+1]);
    	scene_base_list->scene_base[j].scnindex = atoi(result[index+2]);
    	strcpy(scene_base_list->scene_base[j].scnaction, result[index+3]);
        index += col;
    }
	//free
	sqlite3_free_table(result);
	return 0;
}
/*
 * function: t_getact()
 * description: get the urlstring of http action in table
 * input:
 * output:
 * return:	0-ok, <0-failed
 * others:
 * */
//int t_getact(sqlite3 *db, const char *table_name, const char *id_name, int id, const char *url_name, url_string_st ***urlstring)
//{
//	int res;
//
//	char *errmsg;
//	char **result;
//	int j,i;
//	int row, col, index;
//
//	const char *querySql = "SELECT %s FROM %s WHERE %s=%d";
//	char sql[256];
//
//	sprintf(sql, querySql, url_name, table_name, id_name, id);
//    if( sqlite3_get_table(db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
//		URLAPI_DEBUG("read db failed:%s\n",errmsg);
//		return (res = ERROR_READ_DB);
//    }
//    URLAPI_DEBUG("row=%d,col=%d\n",row,col);
//
//    if(row ==0){
//    	sqlite3_free_table(result);
//    	return (res = ERROR_MDFY_NO_ID_DB);
//    }
//
//    //generate string
//    **urlstring = (url_string_st *)malloc(row*sizeof(url_string_st)); //need to free in outside
//    if(urlstring == NULL){
//    	URLAPI_DEBUG("malloc error\n");
//    	sqlite3_free_table(result);
//    	return (res = ERROR_OTHER);
//    }
//
//	index = col;
//    for(j=0; j<row; j++){
//
////    	urlstring[j]->urltotal = row;
////    	strcpy(urlstring[j]->urlstring, result[index]);
////    	URLAPI_DEBUG("%d\n", urlstring[j]->urltotal);
////    	URLAPI_DEBUG("%s\n", urlstring[j]->urlstring);
//    	(**urlstring[j]).urltotal = row;
//    	strcpy((**urlstring[j]).urlstring, result[index]);
//    	URLAPI_DEBUG("%d\n", (**urlstring[j]).urltotal);
//    	URLAPI_DEBUG("%s\n", (**urlstring[j]).urlstring);
//    	index += col;
//    }
//
//	//free
//	sqlite3_free_table(result);
//	return 0;
//}
int t_getact(sqlite3 *db, const char *sql, url_string_st ***urlstring)
{
	int res;

	char *errmsg;
	char **result;
	int j;
	int row, col, index;

    if( sqlite3_get_table(db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
		URLAPI_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }

    if(row ==0){
    	sqlite3_free_table(result);
    	return (res = ERROR_MDFY_NO_ID_DB);
    }

    //generate string
    **urlstring = (url_string_st *)malloc(row*sizeof(url_string_st)); //need to free in outside
    if(urlstring == NULL){
    	URLAPI_DEBUG("malloc error\n");
    	sqlite3_free_table(result);
    	return (res = ERROR_OTHER);
    }

	index = col;
    for(j=0; j<row; j++){
    	((**urlstring)[j]).urltotal = row;
    	strcpy(((**urlstring)[j]).urlstring, result[index]);
    	index += col;
    }
	//free
	sqlite3_free_table(result);
	return 0;
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
	char sql[256];
	const char* insertSQL = "INSERT INTO t_scene VALUES (null,'%s', %d, '%s')";
	sprintf(sql, insertSQL, base->scnname, base->scnindex, base->scnaction);

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
	res = t_insert_retid(db, sql, &base->sid);
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
/*
 * function: modify_scene_db()
 * description: modify_scene_db
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int modify_scene_db(const scene_base_st* base, const scene_action_stPtr action)
{
	int res;

	const char* updateSQL = "UPDATE t_scene SET scnname= '%s', scnindex = %d, "
			"scnaction = '%s' WHERE sid = %d";
	const char* deleteSQL = "DELETE FROM t_scene_act WHERE sid = %d";

	char sql[256];

	//准备
	res = db_init();
	if(res<0){
		db_close();
		return res;
	}
	//create table
	res = t_scene_create();
	if(res<0){
		db_close();
		return res;
	}
	//create table
	res = t_scene_act_create();
	if(res<0){
		db_close();
		return res;
	}
	//update
	sprintf(sql, updateSQL, base->scnname, base->scnindex, base->scnaction, base->sid);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		db_close();
		return res;
	}
	//delete t_scene_act according sid
	sprintf(sql, deleteSQL, base->sid);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		db_close();
		return res;
	}
	//insert new
	res = t_scene_action_insert(action, base->sid, db);
	if(res<0){
		db_close();
		return res;
	}
	//关闭数据库
	db_close();
	return (res = 0);
}
/*
 * function: del_scene_db()
 * description: del_scene_db by sid
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int del_scene_db(int sid)
{
	int res;
	const char* deleteSQL = "DELETE FROM t_scene WHERE sid = %d";
	const char* deleteSQL2 = "DELETE FROM t_scene_act WHERE sid = %d";
	char sql[256];

	//准备
	res = db_init();
	if(res<0){
		db_close();
		return res;
	}
	//create table
	res = t_scene_create();
	if(res<0){
		db_close();
		return res;
	}
	//create table
	res = t_scene_act_create();
	if(res<0){
		db_close();
		return res;
	}
	//delete t_scene by sid
	sprintf(sql, deleteSQL, sid);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		db_close();
		return res;
	}
	//delete t_scene_act according sid
	sprintf(sql, deleteSQL2, sid);
	res = t_update_delete_and_change_check(db,sql);
	if(res<0){
		db_close();
		return res;
	}
	//关闭数据库
	db_close();
	return (res = 0);
}
/*
 * function: get_scene_db()
 * description: get_scene_db
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int get_scene_db(scene_base_list_st *scene_base_list)
{
	int res;

	//准备
	res = db_init();
	if(res<0){
		db_close();
		return res;
	}
	//create table
	res = t_scene_create();
	if(res<0){
		db_close();
		return res;
	}

	res = t_scene_getlist(scene_base_list, db);
	if(res<0){
		db_close();
		return res;
	}
	//关闭数据库
	db_close();
	return (res = 0);
}
/*
 * function: read_scene_act_db()
 * description: read_scene_act_db
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int read_scene_act_db(int sid, url_string_st **acturl)
{
	int res;
	const char *qsql ="SELECT urlstring FROM t_scene_act WHERE sid=%d";
	char sql[256];

	sprintf(sql, qsql, sid);
	//准备
	res = db_init();
	if(res<0){
		db_close();
		return res;
	}
	//create table
	res = t_scene_act_create();
	if(res<0){
		db_close();
		return res;
	}
	//read t_scene_act
	res = t_getact(db, sql, &acturl);
	if(res<0){
		db_close();
		return res;
	}
	//关闭数据库
	db_close();
	return (res = 0);
}
int modify_scene_index(const int *idall, const int *indexall, int cnt)
{
	int res;
	int count = cnt;

	//准备
	res = db_init();
	if(res<0){
		db_close();
		return res;
	}
	//create table
	res = t_scene_create();
	if(res<0){
		db_close();
		return res;
	}
	res = t_scene_index_modify(db, idall, indexall, count);
	if(res<0){
		db_close();
		return res;
	}
	return res;
}

