/*
 *	File name   : linkageLoop.c
 *  Created on  : Apr 28, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "list.h"
#include "linkageLoop.h"
#include "sqliteOperator.h"
#include "httpSocketRaw.h"
#include "timedaction.h"

#define LINK_EFFECT_TIME		3	//3s

list_linkage_st linkage_loop_head;
//list_linkage_st *linkage_loop;
//struct list_head *pos, *next;

extern timed_action_notifier* notifier;


void link_effect_status_update(void *arg);
//初始化链表头
void linkage_head_init()
{
	INIT_LIST_HEAD(&linkage_loop_head.list);
}
//遍历链表打印链表信息
void linkage_traverse_printf()
{
	struct list_head *pos;
	list_linkage_st *linkage_pos;

	printf("====linkage list traverse====\n");
	list_for_each(pos, &(linkage_loop_head.list)) {
		linkage_pos = list_entry(pos, list_linkage_st, list);
		printf("lid:%d\n", linkage_pos->linkage_member.lid);
		printf("ieee:%s\n", linkage_pos->linkage_member.trgieee);
		printf("ep:%s\n", linkage_pos->linkage_member.trgep);
		printf("attribute:%s\n", linkage_pos->linkage_member.attribute);
		printf("operator:%s\n", linkage_pos->linkage_member.operator);
		printf("value:%d\n", linkage_pos->linkage_member.value);
		printf("effect status:%d\n", linkage_pos->linkage_member.effect_status);
		printf("actiontype:%d\n", linkage_pos->linkage_member.actiontype);
		printf("~~~~~~~~~~~~~~~~~~~~~~~~\n");
	}
}
//添加新节点到链尾
void list_linkage_add_member(linkage_loop_st member)
{
	list_linkage_st *pos;

	pos = (list_linkage_st *)malloc(sizeof(list_linkage_st));
	if(pos == NULL) {
		GDGL_DEBUG("malloc failed\n");
		exit(1);
	}

	pos->linkage_member = member;
	//将节点链接到链表的末尾
	list_add_tail(&(pos->list), &(linkage_loop_head.list));
	printf("[lnk] add id=%d\n", member.lid);
//	linkage_traverse_printf();
}
/*
 * description:根据id编辑链表成员信息
 * return value: 1=edit success, 0=eidt failed
 * */
char list_linkage_edit_member(linkage_loop_st member)
{
	char res=0;
	struct list_head *pos, *next;
	list_linkage_st *member_pos;

	list_for_each_safe(pos, next, &(linkage_loop_head.list)) {
		member_pos = list_entry(pos, list_linkage_st, list);
		if(member_pos->linkage_member.lid == member.lid) {
			member_pos->linkage_member = member;
			res = 1;
			printf("[lnk] edit id=%d\n", member.lid);
			break;
		}
	}
//	linkage_traverse_printf();
	return res;
}
/*
 * description: 根据lid删除对应节点
 * return: >0删除成功，=0删除失败，找不到对应节点id。
 * */
char list_linkage_remove_member_byid(int id_value)
{
	char res=0;
	struct list_head *pos, *next;
	list_linkage_st *member;

	list_for_each_safe(pos, next, &(linkage_loop_head.list)) {
		member = list_entry(pos, list_linkage_st, list);
		if(member->linkage_member.lid == id_value) {			//找到这个id节点
			list_del_init(pos);
			free(member);
			res = 1;
			printf("[lnk] remove id=%d\n", id_value);
			break;
		}
	}
//	linkage_traverse_printf();
	return res;
}
//释放链表
void release_list_linkage()
{
	struct list_head *pos, *next;
	list_linkage_st *member_pos;

	list_for_each_safe(pos, next, &(linkage_loop_head.list)) {
		member_pos = list_entry(pos, list_linkage_st, list);
		list_del_init(pos);
		free(member_pos);
	}
}
//根据运算符字符计算运算符整型标记
static int flag_of_operator(char *opt)
{
	const char opt_string[5][2] = {"eq", "bt", "lt", "be", "le"};
	int res;
	int i;
	for(i=0; i<5; i++) {
		res = memcmp(opt, &(opt_string[i][0]), 2);
		if(res == 0)
			return i;
	}
	return -1;
}

void list_linkage_compare_condition_trigger(char *ieee, char *ep, char *attr, int value, char *time_para)
{
	struct list_head *pos, *next;
	list_linkage_st *member;
	timed_action_t *ta_link_status_1s;
	char trg_enable =0;

	list_for_each_safe(pos, next, &(linkage_loop_head.list)) {
		member = list_entry(pos, list_linkage_st, list);
		if(member->linkage_member.effect_status ==1) {
			continue;
		}
//		if(		(memcmp(ieee, member->linkage_member.trgieee, IEEE_LEN)==0)&&
//				(memcmp(ep, member->linkage_member.trgep, 2)==0)&&
//				(memcmp(attr, member->linkage_member.attribute, strlen(attr))==0)
//		)
		if(		(memcmp(ieee, member->linkage_member.trgieee, strlen(ieee))==0)&&
				(memcmp(ep, member->linkage_member.trgep, 2)==0)&&
				(memcmp(attr, member->linkage_member.attribute, strlen(attr))==0)
		)
		{
			switch(flag_of_operator(member->linkage_member.operator)) {
				case 0:
					if(value == member->linkage_member.value)
						trg_enable =1;
				break;
				case 1:
					if(value > member->linkage_member.value)
						trg_enable =1;
				break;
				case 2:
					if(value < member->linkage_member.value)
						trg_enable =1;
				break;
				case 3:
					if(value >= member->linkage_member.value)
						trg_enable =1;
				break;
				case 4:
					if(value <= member->linkage_member.value)
						trg_enable =1;
				break;
				default:break;
			}
			if(trg_enable) {

				trg_enable =0;
				member->linkage_member.effect_status =1;
				//执行定时任务：联动操作触发后，3秒内不能再次触发；
				ta_link_status_1s = timed_action_schedule(notifier, LINK_EFFECT_TIME, 0, &link_effect_status_update, &(member->linkage_member));
			    if(ta_link_status_1s ==NULL){
			    	GDGL_DEBUG("time task init error\n");
			    	exit(1);
			    }
				printf("[lnk] trigger, lid=%d, ieee=%s, attribute=%s, opt=%s, value=%d\n",member->linkage_member.lid,ieee,attr,member->linkage_member.operator,value);
				printf("[lnk] actiontype=%d\n", member->linkage_member.actiontype);
				if((member->linkage_member.actiontype == IPC_CAPTURE_ACT_TYPE)||
						(member->linkage_member.actiontype == IPC_RECORD_ACT_TYPE))	//如果是截图或者录像的操作，重新组合url串
				{
					execute_ipccapture_url(LINkAGE_TABLE_FLAG, member->linkage_member.lid, time_para);
				}
				else
				{
					execute_url_action(LINkAGE_TABLE_FLAG, member->linkage_member.lid);
				}
			}
		}
	}
}

/*
 * 联动列表允许触发的状态更新, effect_status=1表示联动现在不能触发，effect_status=0表示联动现在可以触发。
 */
void link_effect_status_update(void *arg)
{
	linkage_loop_st *pos = (linkage_loop_st *)arg;
	if(pos->effect_status ==1) {
		pos->effect_status =0;
		printf("[lnk] lid[%d] effect status expired\n", pos->lid);
	}
}

