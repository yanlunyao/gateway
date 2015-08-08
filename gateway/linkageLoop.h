/*
 *	File name   : linkageLoop.h
 *  Created on  : Apr 28, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#ifndef LINKAGELOOP_H_
#define LINKAGELOOP_H_

#include "smartgateway.h"
//#include "list.h"

//typedef struct
//{
//	int lid;
//	char trgieee[IEEE_LEN+1];
//	char trgep[2+1];
//	char attribute[ATTRIBUTE_LEN+1]; //属性
//	char operator[REL_OPERATOR_LEN+1];		//运算符
//	int value;			//值
//	struct list_head list;
//}linkage_loop_st;

extern list_linkage_st linkage_loop_head;



void linkage_head_init();
void linkage_traverse_printf();
void list_linkage_add_member(linkage_loop_st member);
char list_linkage_edit_member(linkage_loop_st member);
char list_linkage_remove_member_byid(int id_value);
void release_list_linkage();
void list_linkage_compare_condition_trigger(char *ieee, char *ep, char *attr, int value, char *time);
#endif /* LINKAGELOOP_H_ */
