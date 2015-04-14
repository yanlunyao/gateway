/*
 * sqliteOperator.h
 *
 *  Created on: Mar 31, 2015
 *      Author: yanly
 */

#ifndef SQLITEOPERATOR_H_
#define SQLITEOPERATOR_H_

#include "smartgateway.h"



//extern function
int add_scene_db(scene_base_st* base, const scene_action_stPtr action);
int modify_scene_db(const scene_base_st* base, const scene_action_stPtr action);
int del_scene_db(int sid);
int get_scene_db(scene_base_list_st *scene_base_list);
int read_scene_act_db(int sid, url_string_st **acturl);
int modify_scene_index(const int *idall, const int *indexall, int cnt);
#endif /* SQLITEOPERATOR_H_ */
