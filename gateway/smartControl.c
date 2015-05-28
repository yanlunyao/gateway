/*
 *	File name   : smartControl.c
 *  Created on  : Apr 20, 2015
 *  Author      : yanly
 *  Description : 定时，联动功能主循环
 *  Version     : V01-00
 *  History     : <author>		<time>		<version>		<desc>
 */

#include "unpthread.h"
#include "timingCommon.h"
#include "smartgateway.h"
#include "sqliteOperator.h"
#include "httpSocketRaw.h"
#include "cJSON.h"
#include "callbackProtocolField.h"
#include "glCalkProtocol.h"
#include "timedaction.h"
#include "linkageLoop.h"
#include "invokeBaseDataUpldPrm.h"

#define  PROJECT_VERSION			"SmartControl-V01-00"

#define  CB_HEART_BEAT_TIME			30   //30s
#define  TIMEOUT_UNIT				5	 //(5s/unit)
#define  INVALID_TIMEOUT			2 	 //10s=2*5

time_list_st list_time[TID_MAX]; //维护的时间全局列表
time_list_st time_member_null = {0};
static pthread_mutex_t	ta_list_lock = PTHREAD_MUTEX_INITIALIZER;
int fd_connect_callback; //连接5018的文件描述符
timed_action_notifier* notifier;

//volatile char new_add_dev_flag =0;    //callback检测到新增设备的标记
//volatile char new_add_dev_timeout =0; //标记的超时时间
volatile char dev_enroll_flag =0;     //设备enroll的标记
volatile char dev_enroll_timeout =0; //标记的超时时间
volatile char dev_unenroll_flag =0;     //设备unenroll的标记
volatile char dev_unenroll_timeout =0; //标记的超时时间

//
void app5s_task(void *arg);
static void time_loop_init();
static void *time_loop_main(void *arg);

static char timing_rule_handle(struct tm *curnt, const time_list_st *time_element);
static char delay_rule_handle(time_t currt_stamp, const time_list_st *time_element);

void send_heartbeat(void* data);
static void read_callback_data_handle(int confd);

static void parse_json_callback(const char *text);
int del_dev_trigger_del_relevant_rule(const char *isdeleted_ieee);

static void list_time_remove_member_lock(int member);
static void list_time_reset_all_member();
static void list_time_add_member_lock(time_list_st member);
static char list_time_edit_member_lock(time_list_st member);
static void list_time_remove_member_byid_lock(int id_value);
static void list_time_printf_tid();
//

int main()
{
	pthread_t timeloop_thread_id;
	timed_action_t *time_action_cb;
	timed_action_t *ta_enroll_check;
	printf("---------%s---------\n", PROJECT_VERSION);
	printf("------------------------------------------------\n");
	printf("------------------------------------------------\n");
	time_loop_init();
	linkage_head_init();
	ta_enroll_check = timed_action_schedule_periodic(notifier, TIMEOUT_UNIT, 0, &app5s_task, NULL); //5s task

	//读数据库启用的定时规则信息，保存到列表
	if(time_action_get_enable_list(list_time) <0) {
		//读表出错，程序退出
		GDGL_DEBUG("time action enable list read error!\n");
		exit(1);
	}
	//读联动列表信息，保存到双向链表
	if(linkage_get_enable_list_db(&linkage_loop_head) <0) {
		//读表出错，程序退出
		GDGL_DEBUG("linkage enable list read error!\n");
		exit(1);
	}
	linkage_traverse_printf();
	list_time_printf_tid();
	//新开线程，循坏检测定时列表。触发定时操作后，列表需更新
	Pthread_create(&timeloop_thread_id, NULL, time_loop_main, NULL);

	//连接端口5018，实时检测相关callback信息，维护列表
	while(1){
		fd_connect_callback = Tcp_connect(NULL, FEATURE_GDGL_CB_DAEMON_CALLBACK_PORT_STR);
		//send heartbeat
		time_action_cb = timed_action_schedule_periodic(notifier, CB_HEART_BEAT_TIME, 0, &send_heartbeat, &fd_connect_callback);
	    if(time_action_cb ==NULL){
	    	GDGL_DEBUG("time task send heartbeat init error\n");
	    	exit(1);
	    }
		//read data
		read_callback_data_handle(fd_connect_callback);
		timed_action_unschedule(notifier, time_action_cb);//stop time task
	}
}
static void time_loop_init()
{
	int res;
	res = db_init();
	if(res<0){
		db_close();
		exit(1);
	}
	list_time_reset_all_member();//初始化列表

	notifier = timed_action_mainloop_threaded(); //初始化多定时任务
	if(notifier == NULL)
	{
		GDGL_DEBUG("multi timeaction task init error \n");
		exit(1);
	}
}
static void *time_loop_main(void *arg)
{
	int i;
	int temp_id;
	int send_cb_len;
	char send_cb_string[256];
	char time_rule_over_flag =0;//标记为1说明要删除在此表里的id规则

	time_t timestamp_current;
	struct tm tm_current;
	timestamp_current = time(NULL); //get current time
	localtime_r(&timestamp_current, &tm_current);

	while(1) {

		for(i=0; i<TID_MAX; i++) {

			if(list_time[i].tid ==-1)
				continue;

			switch(list_time[i].mode) {  //是否时间到
				case TIME_ACTION_DELAY_MODE:
					time_rule_over_flag = delay_rule_handle(timestamp_current, &list_time[i]);
					break;
				case TIME_ACTION_TIMING_MODE:
					time_rule_over_flag = timing_rule_handle(&tm_current, &list_time[i]);
					break;
				default :
					break;
			}
			if(time_rule_over_flag) {
				temp_id = list_time[i].tid;
				list_time_remove_member_lock(i); //add mulex lock
				//disable the tid in the database and send calkback
				do_time_action_db(temp_id, STOP_TIMEACTION);
				if((send_cb_len = cJsonEnableTimeAction_callback(send_cb_string, ENABLE_TA_SUBID, 1, temp_id, STOP_TIMEACTION)) >=0) {
					push_to_CBDaemon(send_cb_string, send_cb_len);
				}
				time_rule_over_flag = 0;
			}
		}
		sleep(1);
		timestamp_current = time(NULL); //get current time
		localtime_r(&timestamp_current, &tm_current);
		//挂起1s?
	}
	return NULL;
}
static char timing_rule_handle(struct tm *curnt, const time_list_st *time_element)
{
	char flag_rule_over =0;
	char repeat_flag;
	char process_time[DELAY_OR_REPEAT_TIME_FLAG_LEN];

	struct tm excute_time;
	time_t excute_time_sec;
	time_t curnt_sec;

	repeat_flag = time_element->repeat;
	strncpy(process_time, time_element->process_time, DELAY_OR_REPEAT_TIME_FLAG_LEN);

	curnt_sec = mktime(curnt);
	gl_time_format_convert_to_system_tm(time_element->excute_time, &excute_time);

	excute_time_sec = mktime(&excute_time);

	char gl_current[14];
	system_tm_convert_to_gl_time_format(curnt, gl_current);
//	printf("excute time=%14.14s, current time=%s\n", time_element->excute_time, gl_current);
//	printf("excute sec=%d, current sec=%d\n", excute_time_sec, curnt_sec);
	if(excute_time_sec < curnt_sec) {

		if(repeat_flag == TIME_ACTION_REPEAT_DISABLE) {
			flag_rule_over =1;
			return flag_rule_over;
		}
	}
	else if(excute_time_sec == curnt_sec) {

		execute_url_action(TIME_ACTION_TABLE_FLAG, time_element->tid);//执行定时的操作
		if(repeat_flag == TIME_ACTION_REPEAT_DISABLE)
			flag_rule_over =1;
		else
			flag_rule_over = 0;
		return flag_rule_over;
	}
	//判断重复时间
	if(repeat_flag == TIME_ACTION_REPEAT_ENABLE) {
//		printf("the day of week of today is %d\n", curnt->tm_wday);
		if(process_time[curnt->tm_wday] == '1') { //今天是否在允许重复的标记里 //process_time= "xxxxxxx", '1'表示允许重复
//			printf("today enable repeat\n");
			if((excute_time.tm_hour == curnt->tm_hour) &&
			(excute_time.tm_min == curnt->tm_min)&&
			(excute_time.tm_sec == curnt->tm_sec)){
				//执行定时的操作
				execute_url_action(TIME_ACTION_TABLE_FLAG, time_element->tid);
			}

		}
	}
	flag_rule_over = 0;
	return flag_rule_over;
}
static char delay_rule_handle(time_t currt_stamp, const time_list_st *time_element)
{
	char flag =0;
	time_t temp_timestamp;
//	time_t temp_current;
	struct tm excute_time;
//	char current_gl_format[14];
//	char out_time_gl_format[15];

//    snprintf(out_time_gl_format, 15, time_element->excute_time);
//    out_time_gl_format[14] = '\0';

	gl_time_format_convert_to_system_tm(time_element->excute_time, &excute_time);
//	system_tm_convert_to_gl_time_format(curnt, current_gl_format);
	//算出执行时间  ---------------------------
	temp_timestamp = mktime(&excute_time);
	temp_timestamp = temp_timestamp + atoi(time_element->process_time);	//登记的时间戳+延时的秒数
//	localtime_r(&temp_stamp, &exetime_tm);					//timestamp convert to tm
	//对比时间是否相等
//	temp_current = mktime(curnt);
//	printf("out time=%d, current time=%d\n", temp_timestamp, currt_stamp);
//	printf("out time=%s, current time=%s\n", out_time_gl_format, current_gl_format);
	if(temp_timestamp <= currt_stamp) {

		if( (temp_timestamp+1) >= currt_stamp) {
			//执行定时的操作
			execute_url_action(TIME_ACTION_TABLE_FLAG, time_element->tid);
		}
		flag =1;
	}
	else {
		flag =0;
	}
	return flag;
}
static void read_callback_data_handle(int confd)
{
	char read_data[MAXLINE];
	ssize_t		nread;

	while(1){

		if ( (nread = read(confd, read_data, MAXLINE-1)) < 0 ) {
			GDGL_DEBUG("read error\n");
			return;
		}
		else if ( nread == 0) {
			GDGL_DEBUG("connection closed by other end\n");
			return;		/* connection closed by other end */
		}
		read_data[nread] = '\0'; // add null
		//GDGL_DEBUG("read data: %s\n", read_data);
		//parse json
		parse_json_callback(read_data);
	}
}
static void parse_json_callback(const char *text)
{
	cJSON *root;
	cJSON *json_temp,*json_temp2;
	int msgtype, mainid, subid,status, id_value, enable;
	char *dev_ieee, *dev_ep, *attrname;
	int attr_value;

	time_list_st temp_member;
	linkage_loop_st link_member;

	// parse request string
	root = cJSON_Parse(text);
	if (!root) {
		GDGL_DEBUG("parse root error\n");
		return;
	}
	json_temp = cJSON_GetObjectItem(root, FIELD_MSGTYPE);
    if (!json_temp)
    {
    	GDGL_DEBUG("json parse error\n");
    	cJSON_Delete(root);
    	return;
    }
    msgtype = json_temp->valueint;
    switch (msgtype) {
    	case GL_MSGTYPE:										//gl的msgtype
	    	json_temp = cJSON_GetObjectItem(root, FIELD_STATUS); //解析status，默认所有广联msgtype都有status字段才可以这样做
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			status = json_temp->valueint;
			if(status <0) {
    	    	GDGL_DEBUG("json status invaild\n");
    	    	cJSON_Delete(root);
    	    	return;
			}
    		json_temp = cJSON_GetObjectItem(root, FIELD_MAINID);
    	    if (!json_temp)
    	    {
    	    	GDGL_DEBUG("json parse error\n");
    	    	cJSON_Delete(root);
    	    	return;
    	    }
    	    mainid = json_temp->valueint;					//解析mainid
    	    switch (mainid) {
    	    	case MAINID_TIMEACTION:
    	    		json_temp = cJSON_GetObjectItem(root, FIELD_TID);
					if (!json_temp)
					{
						GDGL_DEBUG("json parse error\n");
						cJSON_Delete(root);
						return;
					}
        	    	id_value = json_temp->valueint;			//解析tid（time action）
    	    		json_temp = cJSON_GetObjectItem(root, FIELD_SUBID);
    	    	    if (!json_temp)
    	    	    {
    	    	    	GDGL_DEBUG("json parse error\n");
    	    	    	cJSON_Delete(root);
    	    	    	return;
    	    	    }
    	    	    subid = json_temp->valueint;			//解析subid（time action）
    	    	    switch(subid) {
    	    	    	case SUBID_ADD_TA:
    	       	    		json_temp = cJSON_GetObjectItem(root, FIELD_ENABLE);
							if (!json_temp)
							{
								GDGL_DEBUG("json parse error\n");
								cJSON_Delete(root);
								return;
							}
							enable = json_temp->valueint;
							if(enable == START_TIMEACTION) {
								//read member parameter, and add to list_time
								if(time_action_get_time_list_st_byid(id_value, &temp_member) >=0) {
									list_time_add_member_lock(temp_member);
								}
							}
    	    	    	break;
    	    	    	case SUBID_EDIT_TA:
    	       	    		json_temp = cJSON_GetObjectItem(root, FIELD_ENABLE);
							if (!json_temp)
							{
								GDGL_DEBUG("json parse error\n");
								cJSON_Delete(root);
								return;
							}
							enable = json_temp->valueint;
							if(enable == START_TIMEACTION) {
								if(time_action_get_time_list_st_byid(id_value, &temp_member) >=0) {
									if(list_time_edit_member_lock(temp_member) ==0) {//no in the list_time
										list_time_add_member_lock(temp_member);     //new add
									}
								}
							}
							else if(enable == STOP_TIMEACTION) {
								list_time_remove_member_byid_lock(id_value);
							}
							else{}
    	    	    	break ;
    	    	    	case SUBID_DEL_TA:
    	    	    		list_time_remove_member_byid_lock(id_value);
    	    	    		//list_time_printf_tid();
    	    	    	break;
    	    	    	case SUBID_ENABLE_TA:
    	       	    		json_temp = cJSON_GetObjectItem(root, FIELD_ENABLE);
							if (!json_temp)
							{
								GDGL_DEBUG("json parse error\n");
								cJSON_Delete(root);
								return;
							}
							enable = json_temp->valueint;
							if(enable == START_TIMEACTION) {
								if(time_action_get_time_list_st_byid(id_value, &temp_member) >=0) {
									if(list_time_edit_member_lock(temp_member) == 0) {//no in the list_time
										list_time_add_member_lock(temp_member);     //new add
									}
								}
							}
							else if(enable == STOP_TIMEACTION) {
								list_time_remove_member_byid_lock(id_value);
							}
							else{}
    	    	    	break;
    	    	    	default:
    	    	    	break;
    	    	    }
    	    	break;

    	    	case MAINID_LINKAGE:
					json_temp = cJSON_GetObjectItem(root, FIELD_LID);
					if(!json_temp) {
						GDGL_DEBUG("json parse error\n");
						cJSON_Delete(root);
						return;
					}
					id_value = json_temp->valueint;			//解析lid(linkage)
					json_temp = cJSON_GetObjectItem(root, FIELD_SUBID);
					if(!json_temp) {
						GDGL_DEBUG("json parse error\n");
						cJSON_Delete(root);
						return;
					}
					subid = json_temp->valueint;			//解析subid(linkage)
					switch(subid) {
						case SUBID_ADD_LINK:
	   	       	    		json_temp = cJSON_GetObjectItem(root, FIELD_ENABLE);
							if (!json_temp)
							{
								GDGL_DEBUG("json parse error\n");
								cJSON_Delete(root);
								return;
							}
							enable = json_temp->valueint;
							if(enable == ENABLE) {
								if(get_linkage_list_member_byid(id_value, &link_member) >=0) {
									list_linkage_add_member(link_member);
								}
							}
						break;
						case SUBID_EDIT_LINK:
	   	       	    		json_temp = cJSON_GetObjectItem(root, FIELD_ENABLE);
							if (!json_temp)
							{
								GDGL_DEBUG("json parse error\n");
								cJSON_Delete(root);
								return;
							}
							enable = json_temp->valueint;
							if(enable == ENABLE) {
								if(get_linkage_list_member_byid(id_value, &link_member) >=0) {
									if(list_linkage_edit_member(link_member) == 0) {
										list_linkage_add_member(link_member);
									}
								}
							}
							else if(enable == DISABLE) {
								list_linkage_remove_member_byid(id_value);
							}
							else{}
						break;
						case SUBID_DEL_LINK:
							list_linkage_remove_member_byid(id_value);
							//list_time_printf_tid();
						break;
						case SUBID_ENABLE_LINK:
    	       	    		json_temp = cJSON_GetObjectItem(root, FIELD_ENABLE);
							if (!json_temp)
							{
								GDGL_DEBUG("json parse error\n");
								cJSON_Delete(root);
								return;
							}
							enable = json_temp->valueint;
							if(enable == ENABLE) {
								if(get_linkage_list_member_byid(id_value, &link_member) >=0) {
									if(list_linkage_edit_member(link_member) == 0) {
										list_linkage_add_member(link_member);
									}
								}
							}
							else if(enable == DISABLE) {
								list_linkage_remove_member_byid(id_value);
							}
							else{}
						break;
						default:break;
					}
    	    	break;
    	    	default:break;
    	    }
    	break;
    	case DEV_ATTRIBUTE_MSGTYPE:
			json_temp = cJSON_GetObjectItem(root, "deviceIeee");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			dev_ieee = json_temp->valuestring;
			json_temp = cJSON_GetObjectItem(root, "deviceEp");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			dev_ep = json_temp->valuestring;
			json_temp = cJSON_GetObjectItem(root, "attributename");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			attrname = json_temp->valuestring;
			json_temp = cJSON_GetObjectItem(root, "value");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			attr_value = atoi(json_temp->valuestring);
			list_linkage_compare_condition_trigger(dev_ieee, dev_ep, attrname, attr_value);
    	break;
    	case DEV_ALARM_MSGTYPE:
			json_temp = cJSON_GetObjectItem(root, "zone_ieee");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			dev_ieee = json_temp->valuestring;
			json_temp = cJSON_GetObjectItem(root, "zone_ep");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			dev_ep = json_temp->valuestring;
			json_temp = cJSON_GetObjectItem(root, "w_description");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			attrname = json_temp->valuestring;
			attr_value = ALARM_LINKAGE_DEFAULT_V;
			list_linkage_compare_condition_trigger(dev_ieee, dev_ep, attrname, attr_value);
    	break;
    	case DEL_DEVICE_MSGTYPE:
			json_temp = cJSON_GetObjectItem(root, "IEEE");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			dev_ieee = json_temp->valuestring;
    		del_dev_trigger_del_relevant_rule(dev_ieee);
		break;
    	case NEW_ADDED_DEVICE_MSGTYPE:
			json_temp = cJSON_GetObjectItem(root, "device");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			json_temp2 = cJSON_GetObjectItem(json_temp, "ieee");
			if (!json_temp2)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			dev_ieee = json_temp2->valuestring;
			printf("ieee:%s\n",dev_ieee);
//			new_add_dev_timeout = INVALID_TIMEOUT;
//			new_add_dev_flag = 1;
			invoke_by_datatype_fork(GET_IEEEENDPOINT_NUM, dev_ieee);
    	break;
    	case DEV_ENROLL_MSGTYPE:
    		dev_enroll_timeout = INVALID_TIMEOUT;  //可能出现新增设备没有立即enroll的情况
    		dev_enroll_flag =1;
    	break;
    	case DEV_UNENROLL_MSGTYPE:
    		dev_unenroll_timeout = INVALID_TIMEOUT;
    		dev_unenroll_flag =1;
    	break;
    	default:break;
    }
	cJSON_Delete(root);
}
/*
 * this fuction remove the member of the list_time[member]
 * */
static void list_time_remove_member_lock(int member)
{
	if(member >= TID_MAX)
		return;
	if(list_time[member].tid !=-1) {
		GDGL_DEBUG("remove id %d from list_time\n", list_time[member].tid);
		Pthread_mutex_lock(&ta_list_lock);
		list_time[member].tid = -1;
		Pthread_mutex_unlock(&ta_list_lock);
	}
}
/*
 * this fuction remove the list_time member by tid
 * */
static void list_time_remove_member_byid_lock(int id_value)
{
	int i;
	for(i=0; i<TID_MAX; i++) {
		if(list_time[i].tid == id_value) {
			GDGL_DEBUG("remove id %d from lsit_time\n", list_time[i].tid);
			Pthread_mutex_lock(&ta_list_lock);
			list_time[i].tid = -1;
			Pthread_mutex_unlock(&ta_list_lock);
		}
	}
}
static void list_time_add_member_lock(time_list_st member)
{
	int i;
	for(i=0; i< TID_MAX; i++) {
		if(list_time[i].tid ==-1) {
			Pthread_mutex_lock(&ta_list_lock);
			list_time[i] = member;
			Pthread_mutex_unlock(&ta_list_lock);
			GDGL_DEBUG("add id %d in list_time\n", list_time[i].tid);
			return;
		}
	}
}
/*
 * return value: 1=edit success, 0=eidt failed
 * */
static char list_time_edit_member_lock(time_list_st member)
{
	int i;
	for(i=0; i< TID_MAX; i++) {
		if(list_time[i].tid == member.tid) {
			Pthread_mutex_lock(&ta_list_lock);
			list_time[i] = member;
			Pthread_mutex_unlock(&ta_list_lock);
			GDGL_DEBUG("edit id %d in list_time\n", list_time[i].tid);
			return 1;
		}
	}
	return 0;
}
static void list_time_reset_all_member()
{
	int i;
	for(i=0; i< TID_MAX; i++) {
		list_time[i].tid = -1;		//初始化列表
	}
}
static void list_time_printf_tid()
{
	int i;
	printf("====timing list traverse====\n");
	for(i=0; i< TID_MAX; i++) {
		if(list_time[i].tid <0)
			continue;
		printf("list_time_id[%d]=%d\n", i, list_time[i].tid);
		printf("list_mode[%d]=%d\n", i, list_time[i].mode);
		printf("list_exetime[%d]=%s\n", i, list_time[i].excute_time);
		printf("list_repeat[%d]=%d\n", i, list_time[i].repeat);
		printf("list_processtime[%d]=%s\n", i, list_time[i].process_time);
	}
	printf("~~~~~~~~~~~~~~~~~~~~~~~~\n");
}
void app5s_task(void *arg)
{
//	if(new_add_dev_flag ==1) {
//		if(new_add_dev_timeout >0) {
//			new_add_dev_timeout--;
//		}
//		else {
//			new_add_dev_flag =0;
//		}
//	}
	if(dev_enroll_flag ==1) {
		if(dev_enroll_timeout >0) {
			dev_enroll_timeout--;
		}
		else {
			dev_enroll_flag =0;
			invoke_by_datatype_fork(GET_CIELIST_NUM, NULL);
		}
	}

	if(dev_unenroll_flag ==1) {
		if(dev_unenroll_timeout >0) {
			dev_unenroll_timeout--;
		}
		else {
			dev_unenroll_flag =0;
			invoke_by_datatype_fork(GET_CIELIST_NUM, NULL);
		}
	}
}
void send_heartbeat(void* data)
{
	int fd = *(int *)data;
	char hearbeat[] = "{\"msgtype\": 1,\"heartbeat\": \"0200070007\"}";

    if ( writen(fd, hearbeat, strlen(hearbeat)) != strlen(hearbeat)) {
    	GDGL_DEBUG("write error\n");
		close(fd);
		return;
    }
}
#define DEL_ID_MAX	100 //删除设备同时允许影响最多的id数目
int del_dev_trigger_del_relevant_rule(const char *isdeleted_ieee)
{
	int will_del_id[DEL_ID_MAX] = {0};
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;
	int i;

	pid_t	pid;
	if ((pid = fork()) > 0) {
		waitpid(-1,NULL,0);
		return(pid);		//parent process
	}
	if ((pid = fork()) > 0) {
		exit(0);			//child process
		return (pid);
	}
							//grandson process
	//判断是否在定时规则里
	if(del_timeaction_by_isdeletedieee(will_del_id, isdeleted_ieee) >0) {
		for(i=0; i<DEL_ID_MAX; i++) {
			if(will_del_id[i] == 0)
				break;
			GDGL_DEBUG("will del id:%d\n",will_del_id[i]);
			if((send_cb_len = cJsonTimeAction_callback(send_cb_string, SUBID_DEL_TA, 1, will_del_id[i], NULL)) >=0) {
				push_to_CBDaemon(send_cb_string, send_cb_len);
				usleep(300000); //300ms //发送太快调试工具接收不到
				GDGL_DEBUG("send tima success\n");
			}
		}
	}
	memset(will_del_id, 0, DEL_ID_MAX);
	//判断是否在场景规则里
	if(del_scene_by_isdeletedieee(will_del_id, isdeleted_ieee) >0) {
		scene_base_st base_buf;
		for(i=0; i<DEL_ID_MAX; i++) {
			if(will_del_id[i] == 0)
				break;
			GDGL_DEBUG("will del id:%d\n",will_del_id[i]);
			if(read_t_scene_base_byid(&base_buf, will_del_id[i]) <0) {
				continue;
			}
			if((send_cb_len = cJsonScene_callback(send_cb_string, &base_buf, NULL, NULL, SUBID_EDIT_SCENE, 1)) >=0) {
				push_to_CBDaemon(send_cb_string, send_cb_len);
				usleep(300000); //300ms //发送太快调试工具接收不到
				GDGL_DEBUG("send scene success\n");
			}
		}
	}
	memset(will_del_id, 0, DEL_ID_MAX);
	//判断是否在联动规则里
	if(del_linkage_by_isdeletedieee(will_del_id, isdeleted_ieee) >0) {
		for(i=0; i<DEL_ID_MAX; i++) {
			if(will_del_id[i] == 0)
				break;
			GDGL_DEBUG("will del id:%d\n",will_del_id[i]);
			if((send_cb_len = cJsonLinkage_callback(send_cb_string, SUBID_DEL_LINK, 1, will_del_id[i], NULL)) >=0) {
				push_to_CBDaemon(send_cb_string, send_cb_len);
				usleep(300000); //300ms //发送太快调试工具接收不到
				GDGL_DEBUG("send linkage success\n");
			}
		}
	}
	GDGL_DEBUG("over\n");
	exit(0);
	return 0;
}

