/*
 * sqliteOperator.h
 *
 *  Created on: Mar 31, 2015
 *      Author: yanly
 */

#ifndef SQLITEOPERATOR_H_
#define SQLITEOPERATOR_H_

#define SQL_STRING_MAX_LEN			1024

#include "smartgateway.h"
#include "sqlite3.h"

int db_init();
void db_close();
int smartcontrol_table_init();

//extern function
sqlite3* get_application_db(void);
int t_getact_per(const char *sql, char *urlstring);
//scene
int add_scene_db(scene_base_st* base, const scene_action_stPtr action);
int modify_scene_db(const scene_base_st* base, const scene_action_stPtr action);
int del_scene_db(int sid);
int get_scene_db(scene_base_list_st *scene_base_list);
int read_scene_act_db(int sid, url_string_st **acturl);
int modify_scene_index(const int *idall, const int *indexall, int cnt);
int read_t_scene_base_byid(scene_base_st *base, int id);
//time
int add_timeaction_db(int *id_value, const time_action_st *time_act);
int edit_timeaction_db(const time_action_st *time_act);
int del_timeaction_db(int tid);
int get_timeaction_list_db(time_action_base_st **time_act, int *list_total);
int read_timeaction_url_db(int id, char *urlstring);
int do_time_action_db(int id, int enable);
int time_action_get_enable_list(time_list_st *list);
int time_action_get_time_list_st_byid(int id, time_list_st *list_member);
//linkage
int add_linkage_db(int *id_value, const linkage_st *linkage_act);
int edit_linkage_db(const linkage_st *linkage_act);
int del_linkage_db(int id_value);
int get_linkage_base_all_list_db(linkage_base_st **linkage_act, int *total_num);
int enable_linkage_db(char *sql);
int linkage_get_enable_list_db(list_linkage_st *head);
int get_linkage_list_member_byid(int id, linkage_loop_st *member);

//
int del_timeaction_by_isdeletedieee(int *id, const char *del_ieee);
int del_scene_by_isdeletedieee(int *id, const char *del_ieee);
int del_linkage_by_isdeletedieee(int *id, const char *del_ieee);

#endif /* SQLITEOPERATOR_H_ */
