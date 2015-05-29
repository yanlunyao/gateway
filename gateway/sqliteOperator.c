/*
 * sqliteOperator.c
 *
 *  Created on: Mar 31, 2015
 *      Author: yanly
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "sqliteWrap.h"
#include "sqliteOperator.h"
#include "timingCommon.h"

sqlite3 * db=NULL;

int t_time_action_create(sqlite3 *target_db);
int t_scene_create(sqlite3 *target_db);
int t_scene_act_create(sqlite3 *target_db);
int t_linkage_create(sqlite3 *target_db);
//////////////////////////////////////////////////////////////////////////////////////////
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
void db_close()
{
	if(db !=NULL)
		sqlite3_close(db);
}
sqlite3* get_application_db(void)
{
	return db;
}
int smartcontrol_table_init()
{
	int res=0;
	res = t_scene_create(db);
	if(res<0){
		return res;
	}
	res = t_scene_act_create(db);
	if(res<0){;
		return res;
	}
	res = t_time_action_create(db);
	if(res<0){;
		return res;
	}
	res = t_linkage_create(db);
	if(res<0){;
		return res;
	}
	return res;
}
/////////////////////////////////////////////////////////////////////////////////////creat table
int t_time_action_create(sqlite3 *target_db)
{
	int result;
    int ret = 0;
    char* errmsg=NULL;
    //actname, actpara, actmode, para1, para2, para3, enable, urlstring, actobj
    const char *sql="create table if not exists t_time_action(tid integer primary key autoincrement, \
    		actname nvarchar(60),actpara varchar(40),actmode int(1),para1 varchar(14),\
    		para2 int(1),para3 char(7),enable int(1),urlstring varchar,actobj char(16), actiontype char(1))";

    //表不存在就建表
    result = sqlite3_exec(target_db, sql, NULL, NULL, &errmsg);
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
int t_scene_create(sqlite3 *target_db)
{
	int result;
    int ret = 0;
    char* errmsg=NULL;

    const char *sql="create table if not exists t_scene(sid integer primary key autoincrement,scnname nvarchar(60),\
    		scnindex int(1),scnaction varchar)";

    //表不存在就建表
    result = sqlite3_exec(target_db,sql, NULL, NULL, &errmsg);
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
int t_scene_act_create(sqlite3 *target_db)
{
	int result;
    int ret = 0;
    char* errmsg=NULL;

    const char *sql="create table if not exists t_scene_act(sid integer(1),urlstring varchar,actobj char(16),actpara varchar(40))";

    //表不存在就建表
    result = sqlite3_exec(target_db, sql, NULL, NULL, &errmsg);
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
int t_linkage_create(sqlite3 *target_db)
{
	int result;
    int ret = 0;
    char* errmsg=NULL;

    const char *sql="create table if not exists t_linkage(lid integer primary key autoincrement,"
    		"lnkname nvarchar(60),trgieee char(16),trgep char(2),trgcnd varchar(30),lnkact varchar(40),enable char(1),"
    		"attribute varchar(30),operator char(2),value int(1),"
    		"actobj char(16),urlstring varchar,actiontype varchar(1))";

    //表不存在就建表
    result = sqlite3_exec(target_db,sql, NULL, NULL, &errmsg);
	if(result != SQLITE_OK )
	{
		URLAPI_DEBUG("create table failed:%s\n",errmsg);
		ret = ERROR_ACCESS_DB;
	}
    sqlite3_free(errmsg);
    return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////
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
	const char* insertSQL = "INSERT INTO t_scene_act VALUES (?, ?, ?, ?)"; //sid integer(1),urlstring varchar(200),actobj char(16),actpara varchar(40)
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
		sqlite3_bind_text(stmt, 4, action[i].actpara, strlen(action[i].actpara), SQLITE_TRANSIENT);
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
int read_t_scene_base_byid(scene_base_st *base, int id)
{
	int res;

	char *errmsg;
	char **result;
	int j;
	int row, col, index;
	char sql[SQL_STRING_MAX_LEN];

	const char *querySql = "SELECT *FROM t_scene WHERE sid=%d";
	sprintf(sql,querySql,id);
    if( sqlite3_get_table(db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
		GDGL_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }
    if(row ==0){
    	sqlite3_free_table(result);
    	return (res = ERROR_MDFY_NO_ID_DB);
    }
	index = col;
    for(j=0; j<row; j++){
    	base->sid = atoi(result[index]);
    	strcpy(base->scnname, result[index+1]);
    	base->scnindex = atoi(result[index+2]);
    	strcpy(base->scnaction, result[index+3]);
        index += col;
    }
	//free
	sqlite3_free_table(result);
	return 0;
}
/*
 * function: t_getact()
 * description: read all url string by dynamic allocation in database
 * input:
 * output:
 * return:	0-ok, <0-failed
 * others:
 * */
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
 * function: t_getact_per()
 * description: read a url string in database
 * input:
 * output:
 * return:	0-ok, <0-failed
 * others:
 * */
int t_getact_per(const char *sql, char *urlstring)
{
	int res;

	char *errmsg;
	char **result;
	int row, col, index;
	int repeat_cnt = 0;

    if( (res = sqlite3_get_table(db, sql, &result, &row, &col, &errmsg))!= SQLITE_OK){  //need to free result
    	while(res == SQLITE_BUSY) {
    		repeat_cnt++;
    		if(repeat_cnt >DB_BUSY_REPEAT_CNT)
    			break;
    		usleep(DB_BUSY_WAIT_TIME*1000); //5ms
    		printf("#\n");
    		res = sqlite3_get_table(db, sql, &result, &row, &col, &errmsg);
    	}
    	if(res !=SQLITE_OK) {
			GDGL_DEBUG("read db failed:%s\n",errmsg);
			return (res = ERROR_READ_DB);
    	}
    }
    if(row ==0){
    	sqlite3_free_table(result);
    	return (res = ERROR_MDFY_NO_ID_DB);
    }
    //generate string
	index = col;
//	GDGL_DEBUG("haha is :%s\n",result[index]);
    strcpy(urlstring, result[index]);
	//free
	sqlite3_free_table(result);
	return 0;
}
//int t_getact_per(const char *sql, char *urlstring)
//{
//	int res;
//
//	sqlite3_stmt *stmt=NULL;
//
//	if ((res = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, NULL)) != SQLITE_OK) {	//构建插入数据的sqlite3_stmt对象
//		GDGL_DEBUG("sqlite3_prepare_v2 failed:%d\n",res);
//		if(stmt)
//			sqlite3_finalize(stmt);
//		return (res = ERROR_ACCESS_DB);
//	}
//
//	do {
//		int r = sqlite3_step(stmt);
//		if(r == SQLITE_ROW) {
//			const char *v= (const char *)sqlite3_column_text(stmt, 0);
//			GDGL_DEBUG("the text string is:%s\n",v);
//		}
//		else if(r == SQLITE_DONE) {
//			break;
//		}
//		else {
//			GDGL_DEBUG("read failed:%d\n",r);
//			break;
//		}
//	}while(1);
//
//	sqlite3_finalize(stmt);
//
//	return 0;
//}
/*
 * function: t_timeaction_get_row_by_id()
 * description:
 * input:
 * output:
 * return:	0-ok, <0-failed
 * others:
 * */
int t_timeaction_get_row_by_id(sqlite3 *my_db, time_action_st *time_act, int id)
{
	int res;

	char *errmsg;
	char **result;
//	int j;
	int row, col, index;
	char sql[SQL_STRING_MAX_LEN];

	const char *querySql = "SELECT *FROM t_time_action WHERE tid=%d";
	sprintf(sql, querySql, id);
//	printf("sql=%s\n",sql);

    if( sqlite3_get_table(my_db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
		URLAPI_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }

    URLAPI_DEBUG("row=%d,col=%d\n",row,col);
    //generate scene_base
    if(row ==0){
    	sqlite3_free_table(result);
    	return (res = ERROR_MDFY_NO_ID_DB);
    }

	index = col;
//    for(j=0; j<row; j++){
    	time_act->ta_base.tid = atoi(result[index]);
    	strcpy(time_act->ta_base.actname, result[index+1]);
    	strcpy(time_act->ta_base.actpara, result[index+2]);
    	time_act->ta_base.actmode = atoi(result[index+3]);
    	strcpy(time_act->ta_base.para1, result[index+4]);
    	time_act->ta_base.para2 = atoi(result[index+5]);
    	strcpy(time_act->ta_base.para3, result[index+6]);
    	time_act->ta_base.enable = atoi(result[index+7]);
    	strcpy(time_act->urlobject.urlstring, result[index+8]);
    	strcpy(time_act->urlobject.actobj, result[index+9]);
        index += col;
//    }
	//free
	sqlite3_free_table(result);
	return 0;
}
int time_action_get_enable_list(time_list_st *list) //
{
	int res;

	char *errmsg;
	char **result;
	int j;
	int row, col, index;

	const char *querySql = "SELECT tid,actmode,para1,para2,para3 FROM t_time_action WHERE enable=1";

    if( sqlite3_get_table(db, querySql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
    	GDGL_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }

    if(row ==0){
    	sqlite3_free_table(result);
    	return (res = 0);	//列表为空
    }

    if(row >= TID_MAX){	//允许同时启用的定时规则范围外
    	sqlite3_free_table(result);
    	GDGL_DEBUG("tid exceed max num\n");
    	return res = -1; //
    }
	index = col;
    for(j=0; j<row; j++){
    	list[j].tid = atoi(result[index]);
    	list[j].mode = (char)atoi(result[index+1]);
    	strcpy(list[j].excute_time, result[index+2]);
    	(list[j].excute_time)[OUT_TIME_FORMAT_LEN] = '\0';
    	list[j].repeat = (char)atoi(result[index+3]);
    	strcpy(list[j].process_time, result[index+4]);
    	(list[j].process_time)[DELAY_OR_REPEAT_TIME_FLAG_LEN] = '\0';
        index += col;
    }
	//free
	sqlite3_free_table(result);
	return 1;
}
int time_action_get_time_list_st_byid(int id, time_list_st *list_member) //
{
	int res;

	char *errmsg;
	char **result;
//	int j;
	int row, col, index;
	char sql[SQL_STRING_MAX_LEN];

	const char *querySql = "SELECT tid, actmode, para1, para2, para3 FROM t_time_action WHERE tid=%d";
	sprintf(sql, querySql, id);

    if( sqlite3_get_table(db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
    	GDGL_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }

    if(row ==0){
    	sqlite3_free_table(result);
    	return (res = -1);	//列表为空
    }

	index = col;
//    for(j=0; j<row; j++){
		list_member->tid = atoi(result[index]);
		list_member->mode = (char)atoi(result[index+1]);
    	strcpy(list_member->excute_time, result[index+2]);
    	(list_member->excute_time)[OUT_TIME_FORMAT_LEN] = '\0';
    	list_member->repeat = (char)atoi(result[index+3]);
    	strcpy(list_member->process_time, result[index+4]);
    	(list_member->process_time)[DELAY_OR_REPEAT_TIME_FLAG_LEN] = '\0';
        index += col;
//    }
	//free
	sqlite3_free_table(result);
	return 0;
}
int t_timeaction_get_alllist(sqlite3 *db, time_action_base_st ***time_act, int *list_total)
{
	int res;

	char *errmsg;
	char **result;
	int j;
	int row, col, index;
	const char *querySql = "SELECT tid,actname,actpara,actmode,para1,para2,para3,enable FROM t_time_action order by tid";

    if( sqlite3_get_table(db, querySql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
		URLAPI_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }

    URLAPI_DEBUG("row=%d,col=%d\n",row,col);
    //generate scene_base
    **time_act = (time_action_base_st *)malloc(row*sizeof(time_action_base_st)); //need to free in outside
	if(**time_act == NULL){
		URLAPI_DEBUG("malloc error\n");
		sqlite3_free_table(result);
		return (res = ERROR_OTHER);
	}
    *list_total = row;		//赋list总数
 	index = col;
	for(j=0; j<row; j++){
		((**time_act)[j]).tid = atoi(result[index]);
		strcpy(((**time_act)[j]).actname, result[index+1]);
		strcpy(((**time_act)[j]).actpara, result[index+2]);
		((**time_act)[j]).actmode = atoi(result[index+3]);
		strcpy(((**time_act)[j]).para1, result[index+4]);
		((**time_act)[j]).para2 = atoi(result[index+5]);
		strcpy(((**time_act)[j]).para3, result[index+6]);
		((**time_act)[j]).enable = atoi(result[index+7]);
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
	char sql[SQL_STRING_MAX_LEN];
	const char* insertSQL = "INSERT INTO t_scene VALUES (null,'%s', %d, '%s')";
	sprintf(sql, insertSQL, base->scnname, base->scnindex, base->scnaction);

	res = t_scene_create(db);
	if(res<0){
		return res;
	}
	res = t_scene_act_create(db);
	if(res<0){;
		return res;
	}
	//写
	res = t_insert_retid(db, sql, &base->sid);
	if(res<0){
		return res;
	}
	res = t_scene_action_insert(action, base->sid, db);
	if(res<0){
		return res;
	}
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

	char sql[SQL_STRING_MAX_LEN];

	//update
	sprintf(sql, updateSQL, base->scnname, base->scnindex, base->scnaction, base->sid);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		return res;
	}
	//delete t_scene_act according sid
	sprintf(sql, deleteSQL, base->sid);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		return res;
	}
	//insert new
	res = t_scene_action_insert(action, base->sid, db);
	if(res<0){
		return res;
	}
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

	//delete t_scene by sid
	sprintf(sql, deleteSQL, sid);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		return res;
	}
	//delete t_scene_act according sid
	sprintf(sql, deleteSQL2, sid);
	res = t_update_delete_and_change_check(db,sql);
	if(res<0){
		return res;
	}
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
	res = t_scene_create(db);
	if(res<0){
		return res;
	}
	res = t_scene_act_create(db);
	if(res<0){;
		return res;
	}
	res = t_scene_getlist(scene_base_list, db);
	if(res<0){
		return res;
	}
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

	//read t_scene_act
	res = t_getact(db, sql, &acturl);
	if(res<0){
		return res;
	}
	return (res = 0);
}
int modify_scene_index(const int *idall, const int *indexall, int cnt)
{
	int res;
	int count = cnt;

	res = t_scene_index_modify(db, idall, indexall, count);
	if(res<0){
		return res;
	}
	return res;
}
/************************************************************************************************/  //table: t_time_action
int add_timeaction_db(int *id_value, const time_action_st *time_act)
{
	int res;
	char sql[SQL_STRING_MAX_LEN];

	const char* insertSQL = "INSERT INTO t_time_action VALUES (NULL,'%s', '%s', %d, '%s', %d, '%s', %d, '%s', '%s', %d)";
	sprintf(sql, insertSQL, time_act->ta_base.actname, time_act->ta_base.actpara, time_act->ta_base.actmode,
			time_act->ta_base.para1, time_act->ta_base.para2, time_act->ta_base.para3, time_act->ta_base.enable,
			time_act->urlobject.urlstring, time_act->urlobject.actobj, time_act->urlobject.actiontype);

	res = t_time_action_create(db);
	if(res<0){
		return res;
	}
	//写
	res = t_insert_retid(db, sql, id_value);
	if(res<0){
		return res;
	}

	return (res = 0);
}
/*
 * function: edit_timeaction_db()
 * description: edit_timeaction_db
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int edit_timeaction_db(const time_action_st *time_act)
{
	int res;
	char sql[SQL_STRING_MAX_LEN];
	const char* updateSQL = "UPDATE t_time_action SET actname='%s', actpara='%s', actmode=%d, para1='%s', para2=%d, para3='%s', enable=%d, "
			"urlstring='%s', actobj='%s', actiontype=%d WHERE tid =%d";

	//update
	sprintf(sql, updateSQL, time_act->ta_base.actname, time_act->ta_base.actpara, time_act->ta_base.actmode,
			time_act->ta_base.para1, time_act->ta_base.para2, time_act->ta_base.para3, time_act->ta_base.enable,
			time_act->urlobject.urlstring, time_act->urlobject.actobj, time_act->urlobject.actiontype, time_act->ta_base.tid);

//	printf("sql=%s\n",sql);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		return res;
	}

	return (res = 0);
}
/*
 * function: del_timeaction_db()
 * description: del_timeaction_db by sid
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int del_timeaction_db(int tid)
{
	int res;
	const char* deleteSQL = "DELETE FROM t_time_action WHERE tid = %d";
	char sql[256];

	//delete t_scene by sid
	sprintf(sql, deleteSQL, tid);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		return res;
	}

	return (res = 0);
}
/*
 * function: get_timeaction_list_db()
 * description: get_timeaction_list_db
 * input:
 * output:
 * return: 0-ok, <0-failed
 * others:
 * */
int get_timeaction_list_db(time_action_base_st **time_act, int *list_total)
{
	int res;

	res = t_time_action_create(db);
	if(res<0){
		return res;
	}
	res = t_timeaction_get_alllist(db, &time_act, list_total);
	if(res<0){
		return res;
	}

	return (res = 0);
}
int do_time_action_db(int id, int enable)
{
	time_action_st ta;
	time_t calendar=0;
	struct tm time_data;
	char new_time[OUT_TIME_FORMAT_LEN];

	int res;
	int actmode;
	const char* updateSQL = "UPDATE t_time_action SET enable=%d WHERE tid =%d";
	const char* update_delay_sql = "UPDATE t_time_action SET para1='%s', enable=%d WHERE tid = %d";
	char sql[SQL_STRING_MAX_LEN];

	if(enable == START_TIMEACTION) {
		//获取该id的row数据
		res = t_timeaction_get_row_by_id(db, &ta, id);
		if(res<0){
//			db_close();
			return res;
		}
		//如果为启用延时, 必须重新登记该动作的时间,并更新数据库
		actmode = ta.ta_base.actmode;
		if((actmode == TIME_ACTION_DELAY_MODE)) {
			//获取系统时间，转换成我们的时间格式
			calendar = time(NULL);  //get current calendar time
			localtime_r(&calendar, &time_data);
			system_tm_convert_to_gl_time_format(&time_data, new_time);
			sprintf(sql, update_delay_sql, new_time, enable, id);
		}
		else
			sprintf(sql, updateSQL, enable, id);
	}
	else
		sprintf(sql, updateSQL, enable, id);

//	printf("sql: %s\n",sql);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		return res;
	}

	return (res = 0);
}
//int do_time_action_db(int id, int enable, time_action_st *ta)
//{
//	int res;
//	const char* updateSQL = "UPDATE t_time_action SET enable=%d WHERE tid =%d";
//	const char* update_delay_sql = "UPDATE t_time_action SET para1='%s', enable=%d WHERE tid = %d";
//	char sql[SQL_STRING_MAX_LEN];
//	//准备
//	res = db_init();
//	if(res<0){
//		db_close();
//		return res;
//	}
//	//获取该id的row数据
//	res = t_timeaction_get_row_by_id(db, ta, id);
//	if(res<0){
//		db_close();
//		return res;
//	}
//	//如果设置的规则启用状态与当前一样，不生效,返回错误
//	if(ta->ta_base.enable == enable){
//		db_close();
//		res = ERROR_PARAMETER_INVALID;
//		return res;
//	}
//	//如果为启用延时, 必须重新登记该动作的时间,并更新数据库
//	int actmode = ta->ta_base.actmode;
//	int enable_flag = ta->ta_base.enable;
//	if((actmode == TIME_ACTION_DELAY_MODE) && (enable_flag == START_TIMEACTION)) {
//		//获取系统时间，转换成我们的时间格式
//		time_t calendar=0;
//		struct tm *time_data;
//		char new_time[OUT_TIME_FORMAT_LEN];
//		calendar = time(NULL);  //get current calendar time
//		time_data = localtime(&calendar);
//		system_tm_convert_to_gl_time_format(time_data, new_time);
//		sprintf(sql, update_delay_sql, new_time, enable, id);
//	}
//	else {
//		sprintf(sql, updateSQL, enable, id);
//	}
//	printf("sql: %s\n",sql);
//	res = t_update_delete_and_change_check(db, sql);
//	if(res<0){
//		db_close();
//		return res;
//	}
//	//关闭数据库
//	db_close();
//	return (res = 0);
//}
/**********************************************************************************************/  //table:t_linkage
/*
 * function: add_linkage_db()
 * */
int add_linkage_db(int *id_value, const linkage_st *linkage_act)
{
	int res;
	char sql[SQL_STRING_MAX_LEN];

	const char* insertSQL = "INSERT INTO t_linkage VALUES (NULL,'%s','%s','%s','%s','%s',%d,'%s','%s',%d,'%s','%s',%d)";

	sprintf(sql, insertSQL, linkage_act->lnk_base.lnkname, linkage_act->lnk_base.trgieee, linkage_act->lnk_base.trgep,
			linkage_act->lnk_base.trgcnd, linkage_act->lnk_base.lnkact, linkage_act->lnk_base.enable,
			linkage_act->lnk_condition.attribute, linkage_act->lnk_condition.operator, linkage_act->lnk_condition.value,
			linkage_act->urlobject.actobj, linkage_act->urlobject.urlstring, linkage_act->urlobject.actiontype);

	res = t_linkage_create(db);
	if(res<0){
		return res;
	}
	res = t_insert_retid(db, sql, id_value);
	if(res<0){
		return res;
	}

	if(linkage_act->urlobject.actiontype == IPC_CAPTURE_ACT_TYPE) {
		char new_url[URL_STRING_LEN+1]={0};
		const char* updateSQL = "UPDATE t_linkage SET urlstring='%s' WHERE lid=%d";
		snprintf(new_url, sizeof(new_url), "%s&flag=%d&ruleid=%d",
				linkage_act->urlobject.urlstring, LNK_IPC_CAPTURE_RULE_FLAG, *id_value);
		sprintf(sql, updateSQL, new_url, *id_value);

//		printf("sql:%s\n",sql);
		res = t_update_delete_and_change_check(db, sql);
		if(res<0){
			return res;
		}
	}
	return (res = 0);
}
int edit_linkage_db(const linkage_st *linkage_act)
{
	int res;
	char sql[SQL_STRING_MAX_LEN];
	char new_url[URL_STRING_LEN+1]={0};
	const char* updateSQL = "UPDATE t_linkage SET lnkname='%s',trgieee='%s',trgep='%s',trgcnd='%s',lnkact='%s',"
			"enable=%d,attribute='%s',operator='%s',value=%d,actobj='%s',urlstring='%s',actiontype=%d WHERE lid=%d";

	if(linkage_act->urlobject.actiontype == IPC_CAPTURE_ACT_TYPE) {
		snprintf(new_url, sizeof(new_url), "%s&flag=%d&ruleid=%d",
				linkage_act->urlobject.urlstring, LNK_IPC_CAPTURE_RULE_FLAG, linkage_act->lnk_base.lid);
	}
	else {
		snprintf(new_url, sizeof(new_url), "%s", linkage_act->urlobject.urlstring);
	}
	//update
	sprintf(sql, updateSQL, linkage_act->lnk_base.lnkname, linkage_act->lnk_base.trgieee, linkage_act->lnk_base.trgep,
			linkage_act->lnk_base.trgcnd, linkage_act->lnk_base.lnkact, linkage_act->lnk_base.enable,
			linkage_act->lnk_condition.attribute, linkage_act->lnk_condition.operator, linkage_act->lnk_condition.value,
			linkage_act->urlobject.actobj, new_url, linkage_act->urlobject.actiontype, linkage_act->lnk_base.lid);
//	printf("sql:%s\n",sql);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		return res;
	}

	return (res = 0);
}
int del_linkage_db(int id_value)
{
	int res;
	const char* deleteSQL = "DELETE FROM t_linkage WHERE lid = %d";
	char sql[SQL_STRING_MAX_LEN];

	//delete t_linkage by lid
	sprintf(sql, deleteSQL, id_value);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		return res;
	}
	return (res = 0);
}
int get_linkage_base_all_list_db(linkage_base_st **linkage_act, int *total_num)
{
	int res;
	char sql[] = "SELECT lid,lnkname,trgieee,trgep,trgcnd,lnkact,enable FROM t_linkage order by lid";

	char *errmsg;
	char **result;
	int j;
	int row, col, index;

	res = t_linkage_create(db);
	if(res<0){
		return res;
	}

    if( sqlite3_get_table(db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){
		URLAPI_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }
    URLAPI_DEBUG("row=%d,col=%d\n",row,col);
    //generate scene_base
    *linkage_act = (linkage_base_st *)malloc(row*sizeof(linkage_base_st)); //need to free in outside
	if(*linkage_act == NULL){
		URLAPI_DEBUG("malloc error\n");
		sqlite3_free_table(result);
		return (res = ERROR_OTHER);
	}

	*total_num = row;		//赋list总数
 	index = col;
	for(j=0; j<row; j++){
		((*linkage_act)[j]).lid = atoi(result[index]);
		strcpy(((*linkage_act)[j]).lnkname, result[index+1]);
		strcpy(((*linkage_act)[j]).trgieee, result[index+2]);
		strcpy(((*linkage_act)[j]).trgep, result[index+3]);
		strcpy(((*linkage_act)[j]).trgcnd, result[index+4]);
		strcpy(((*linkage_act)[j]).lnkact, result[index+5]);
		((*linkage_act)[j]).enable = atoi(result[index+6]);
		index += col;
	}

	//free
	sqlite3_free_table(result);
	return 0;
}
int enable_linkage_db(char *sql)
{
	int res;

	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		return res;
	}

	return (res = 0);
}
int linkage_get_enable_list_db(list_linkage_st *head)
{
	int res;
	char *errmsg;
	char **result;
	int j;
	int row, col, index;

	list_linkage_st *pos;

	char sql[] = "SELECT lid,trgieee,trgep,attribute,operator,value FROM t_linkage WHERE enable=1";

    if( sqlite3_get_table(db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
    	GDGL_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }

    if(row ==0){
    	sqlite3_free_table(result);
    	return (res = 0);	//列表为空
    }

	index = col;
    for(j=0; j<row; j++){
    	pos = (list_linkage_st *)malloc(sizeof(list_linkage_st));
    	if(pos ==NULL) {
    		GDGL_DEBUG("malloc failed\n");
    		exit(1);
    	}
    	pos->linkage_member.effect_status =0;  //add yanly150525
    	pos->linkage_member.lid = atoi(result[index]);
    	snprintf(pos->linkage_member.trgieee, IEEE_LEN+1, result[index+1]);
    	snprintf(pos->linkage_member.trgep, 2+1, result[index+2]);
    	snprintf(pos->linkage_member.attribute, ATTRIBUTE_LEN+1, result[index+3]);
    	snprintf(pos->linkage_member.operator, REL_OPERATOR_LEN+1, result[index+4]);
    	pos->linkage_member.value = atoi(result[index+5]);
    	//将节点链接到链表的末尾
    	list_add_tail(&(pos->list), &(head->list));
        index += col;
    }
	//free
	sqlite3_free_table(result);
	return 1;
}
int get_linkage_list_member_byid(int id, linkage_loop_st *member) //
{
	int res;

	char *errmsg;
	char **result;

	int row, col, index;
	char sql[SQL_STRING_MAX_LEN];

	const char *querySql = "SELECT lid,trgieee,trgep,attribute,operator,value FROM t_linkage WHERE lid=%d";
	sprintf(sql, querySql, id);

    if( sqlite3_get_table(db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
    	GDGL_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }
    if(row ==0){
    	sqlite3_free_table(result);
    	return (res = -1);	//列表为空
    }
	index = col;
//    for(j=0; j<row; j++){
		member->effect_status =0;  //add yanly150525
		member->lid = atoi(result[index]);
		snprintf(member->trgieee, IEEE_LEN+1, result[index+1]);
		snprintf(member->trgep, 2+1, result[index+2]);
		snprintf(member->attribute, ATTRIBUTE_LEN+1, result[index+3]);
		snprintf(member->operator, REL_OPERATOR_LEN+1, result[index+4]);
		member->value = atoi(result[index+5]);
        index += col;
//    }
	//free
	sqlite3_free_table(result);
	return 0;
}
/**********************************************************************************************/
//删除设备需要的sql操作
/*
 * return : 0~not found, >0~found, <0~access database failed
 * */
int del_timeaction_by_isdeletedieee(int *id, const char *del_ieee)
{
	int res;
	char *errmsg;
	char **result;
	int row, col, index,j;
	char sql[SQL_STRING_MAX_LEN];
	const char *querySql = "SELECT DISTINCT tid FROM t_time_action WHERE actobj='%s'";
	const char *delSql = "DELETE FROM t_time_action WHERE actobj='%s'";

	//read id
	sprintf(sql, querySql, del_ieee);
    if( sqlite3_get_table(db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
    	GDGL_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }
    if(row ==0){
    	sqlite3_free_table(result);
    	return (res = 0);
    }
	index = col;
    for(j=0; j<row; j++){
		id[j] = atoi(result[index]);
        index += col;
    }
    id[row] = 0;
	sqlite3_free_table(result);
	//delete
	sprintf(sql, delSql, del_ieee);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		return res;
	}
	return (res = 1);
}
int del_scene_by_isdeletedieee(int *id, const char *del_ieee)
{
	int res;
	char *errmsg;
	char **result;
	int row, col, index,j;
	int id_total;
	char scnaction[SCENEACTION_MAX_LEN]={0};
	char last_scnaction[SCENEACTION_MAX_LEN]={0};
	char sql[SQL_STRING_MAX_LEN];
	const char *querySql = "SELECT DISTINCT sid FROM t_scene_act WHERE actobj='%s'";
	const char *queryActpara = "SELECT actpara FROM t_scene_act WHERE sid=%d";
	const char *delSql = "DELETE FROM t_scene_act WHERE actobj='%s'";
	const char *updateSql = "UPDATE t_scene SET scnaction='%s' WHERE sid = %d";

	//read id
	sprintf(sql, querySql, del_ieee);
    if( sqlite3_get_table(db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
    	GDGL_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }
    if(row ==0){ sqlite3_free_table(result); return (res = 0);}
	index = col;
    for(j=0; j<row; j++){ id[j] = atoi(result[index]); index += col;}
    id[row] = 0; id_total = row;
	sqlite3_free_table(result);

	//delete
	sprintf(sql, delSql, del_ieee);
//	printf("del sql is %s\n",sql);
	res = t_update_delete_and_change_check(db, sql);
	if(res <0)
		return res;

	//read actpara of t_scene_atc
	int i;
	for(i=0; i < id_total; i++) {
		sprintf(sql, queryActpara, id[i]);
//		printf("query sql is %s\n",sql);
	    if( sqlite3_get_table(db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
	    	GDGL_DEBUG("read db failed:%s\n",errmsg);
			return (res = ERROR_READ_DB);
	    }
	    if(row ==0){
//	    	GDGL_DEBUG("row is zero,id=%d\n",id[i]);
	    	sprintf(sql, updateSql, scnaction, id[i]);
	    	res = t_update_delete_and_change_check(db, sql);
	    	if(res< 0) {
	    		GDGL_DEBUG("update failed\n");
	    	}
	    	sqlite3_free_table(result);
	    	continue;
	    }
		index = col;
	    for(j=0; j<row; j++){
	    	if(j ==0) {
	    		snprintf(scnaction, SCENEACTION_MAX_LEN, "%s", result[index]);
	    	}
	    	else{
	    		snprintf(scnaction, SCENEACTION_MAX_LEN, "%s@%s", last_scnaction, result[index]);
	    	}
	    	strcpy(last_scnaction, scnaction);
	    	index += col;
	    }
	    //update t_scene
//	    printf("scnaction is %s\n",scnaction);
	    sprintf(sql, updateSql, scnaction, id[i]);
	    res = t_update_delete_and_change_check(db, sql);
	    if(res<0){
	    	GDGL_DEBUG("update failed\n");
	    }
	    sqlite3_free_table(result);
	}
	return (res = 1);
}
int del_linkage_by_isdeletedieee(int *id, const char *del_ieee)
{
	int res;
	char *errmsg;
	char **result;
	int row, col, index,j;
	char sql[SQL_STRING_MAX_LEN];
	const char *querySql = "SELECT DISTINCT lid FROM t_linkage WHERE (actobj='%s' OR trgieee='%s')";
	const char *delSql = "DELETE FROM t_linkage WHERE (actobj='%s' OR trgieee='%s')";

	//read id
	sprintf(sql, querySql, del_ieee, del_ieee);
    if( sqlite3_get_table(db, sql, &result, &row, &col, &errmsg)!= SQLITE_OK){  //need to free result
    	GDGL_DEBUG("read db failed:%s\n",errmsg);
		return (res = ERROR_READ_DB);
    }
    if(row ==0){
    	sqlite3_free_table(result);
    	return (res = 0);
    }
	index = col;
    for(j=0; j<row; j++){
		id[j] = atoi(result[index]);
        index += col;
    }
    id[row] = 0;
	sqlite3_free_table(result);
	//delete
	sprintf(sql, delSql, del_ieee, del_ieee);
	res = t_update_delete_and_change_check(db, sql);
	if(res<0){
		return res;
	}
	return (res = 1);
}
/**********************************************************************************************/


