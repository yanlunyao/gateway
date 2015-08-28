/*
 *	File name   : smartControl.c
 *  Created on  : Apr 20, 2015
 *  Author      : yanly
 *  Description : 定时，联动功能主循环
 *
 *  新增，删除设备时调用模块BaseDataUpload；
 *  检测到报警时调用命令播放报警音乐；
 *  修改报警联动检测callback为msgtype=7，安防设备触发的callback
 *  Version     : V01-00
 *  History     : <author>		<time>		<version>		<desc>
 *  			  yanly		    150806		V01-01			增加RF设备相关逻辑：支持联动，RF设备列表更新时上传服务器
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
#include "baseDataUpload.h"
#include "httpGetMsgSmart.h"
#include <curl/curl.h>
#include "zigbeeDeviceInfo.h"

#define  PROJECT_VERSION			"SmartControl-V02-00"
#define  APP_NAME					"SmartControl"
#define  APP_VERSION				"V02-00"

#define  CB_HEART_BEAT_TIME			30   //30s
#define  TIMEOUT_UNIT				5	 //(5s/unit)
#define  INVALID_TIMEOUT			2 	 //10s=2*5

time_list_st list_time[TID_MAX]; //维护的时间全局列表
time_list_st time_member_null = {0};
static pthread_mutex_t	ta_list_lock = PTHREAD_MUTEX_INITIALIZER;
int fd_connect_callback; //连接5018的文件描述符
int warningduration = DEFAULT_GATEWAY_WARNING_DURATION; //网关报警时长

timed_action_notifier* notifier;
timed_action_t *stop_music_task;

//volatile char new_add_dev_flag =0;    //callback检测到新增设备的标记
//volatile char new_add_dev_timeout =0; //标记的超时时间
volatile char dev_enroll_flag =0;     //设备enroll的标记
volatile char dev_enroll_timeout =0; //标记的超时时间
volatile char dev_unenroll_flag =0;     //设备unenroll的标记
volatile char dev_unenroll_timeout =0; //标记的超时时间

//播放报警音乐的文件名字
const char *alarm_music_name[10] = {"burglar",
		"emergency",
		"fire",
		"gas",
		"stop",
		"water",
		"lowbattery",
		"devicetrouble",
		"doorbell",
		"dooropen"};
const unsigned char zigbee_wmode_map[] = {4, 0, 2, 1, 0xff, 7, 8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 6}; //0xff为无效，表示这个报警的w_mode不用声音报警
const unsigned char rf_wmode_map[] = {0xff, 1, 3, 2, 9, 0xff, 0xff, 0xff, 5, 0xff, 0xff, 0, 0xff, 6, 0xff};

//5018通信相关
void send_heartbeat(void* data);
static void read_callback_data_handle(int confd);
static void parse_json_callback(const char *text);

//设备被删除后触发的数据库操作
int del_dev_trigger_del_relevant_rule(const char *isdeleted_ieee);

//定时操作相关
void app5s_task(void *arg);
static void time_loop_init();
static void *time_loop_main(void *arg);
static char timing_rule_handle(struct tm *curnt, const time_list_st *time_element);
static char delay_rule_handle(time_t currt_stamp, const time_list_st *time_element);
static void list_time_remove_member_lock(int member);
static void list_time_reset_all_member();
static void list_time_add_member_lock(time_list_st member);
static char list_time_edit_member_lock(time_list_st member);
static void list_time_remove_member_byid_lock(int id_value);
static void list_time_printf_tid();

//多进程执行外部程序或命令
int system_to_do_mul_process(const char *string);

//网关报警声音操作相关
void setting_warningduration(int value);
void gateway_warning_operation(const char *cmdstring);

int main()
{
	pthread_t timeloop_thread_id;
	timed_action_t *time_action_cb;
	timed_action_t *ta_enroll_check;
	printf("===============%s version[%s], COMPILE_TIME[%s: %s]\n", APP_NAME, APP_VERSION, __DATE__, __TIME__);
//	printf("---------%s---------\n", PROJECT_VERSION);
	printf("------------------------------------------------\n");
	printf("------------------------------------------------\n");

	time_loop_init();
	linkage_head_init();
	int rc = curl_global_init(CURL_GLOBAL_ALL);
	if (rc != CURLE_OK) {
		GDGL_DEBUG("curl_global_init error\n");
		exit (1);
	}
	int tempvalue;
	//获取网关报警时长
	if(http_get_warningduration(&tempvalue) ==0) {
		setting_warningduration(tempvalue);
	}
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
	curl_global_cleanup();
}
static void time_loop_init()
{
	int res;
	res = db_init();
	if(res<0){
		db_close();
		exit(1);
	}
	res = smartcontrol_table_init();
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
//		printf("read data[size:%d]: %s\n", nread,read_data);
		//parse json
		parse_json_callback(read_data);
	}
}
static void parse_json_callback(const char *text)
{
	cJSON *root;
	cJSON *json_temp,*json_temp2;
	int msgtype, mainid, subid,status, id_value, enable,w_mode;
	zigbee_wmode zgwmode;
	rf_wmode rfwmode;
	char *dev_ieee, *dev_ep, *attrname, *warntime;
	int attr_value;
	int alarm1,alarm2;
	char audio_play_string[100]={0};

	//判断需不需要分包的标记
	char need_parse_again_flag=0;
	int next_size=0;

	time_list_st temp_member;
	linkage_loop_st link_member;

	// parse request string
	root = cJSON_Parse(text);
	if (!root) {
		GDGL_DEBUG("parse root error\n");
		return;
	}
	printf("[5018] size=%d\n",strlen(text));
	char *after1,*after2;
	after1 = cJSON_Print(root);
	after2 = cJSON_PrintUnformatted(root);
//	printf("cJSON_Print size=%d\n",strlen(after1));
//	printf("cJSON_PrintUnformatted size=%d\n",strlen(after2));
	if((strlen(text) == strlen(after1) )||(strlen(text) == strlen(after2)))
	{
		need_parse_again_flag =0;
	}
	else
	{  //分包
//		printf("cJSON_Print next value:%x\n",text[strlen(after1)]);
//		printf("cJSON_PrintUnformatted next value:%x\n",text[strlen(after2)]);
		if(text[strlen(after1)] == '{')
		{
			need_parse_again_flag =1;
			next_size = strlen(after1);
		}
		else if(text[strlen(after2)] == '{')
		{
			need_parse_again_flag =1;
			next_size = strlen(after2);
		}
		else
		{
			GDGL_DEBUG("subcontract invalid\n");
		}
	}
	free(after1);
	free(after2);

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
//	    	json_temp = cJSON_GetObjectItem(root, FIELD_STATUS); //解析status，默认所有广联msgtype都有status字段才可以这样做
//			if (!json_temp)
//			{
//				GDGL_DEBUG("json parse error\n");
//				cJSON_Delete(root);
//				return;
//			}
//			status = json_temp->valueint;
//			if(status <0) {
//    	    	GDGL_DEBUG("json status invaild\n");
//    	    	cJSON_Delete(root);
//    	    	return;
//			}
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
    		    	json_temp = cJSON_GetObjectItem(root, FIELD_STATUS); //解析status，默认所有广联定时都有status字段才可以这样做
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
    		    	json_temp = cJSON_GetObjectItem(root, FIELD_STATUS); //解析status，默认所有联动都有status字段才可以这样做
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
    	    	case MAINID_RF:
#ifdef USE_RF_FUNCTION
					json_temp = cJSON_GetObjectItem(root, FIELD_SUBID);
					if(!json_temp) {
						GDGL_DEBUG("json parse error\n");
						cJSON_Delete(root);
						return;
					}
					subid = json_temp->valueint;			//解析subid(rf)
					switch(subid) {
						case SUBID_RF_ALARM:
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
							json_temp = cJSON_GetObjectItem(root, "w_mode");
							if (!json_temp)
							{
								GDGL_DEBUG("json parse error\n");
								cJSON_Delete(root);
								return;
							}
							w_mode = atoi(json_temp->valuestring);
							json_temp = cJSON_GetObjectItem(root, "w_description");
							if (!json_temp)
							{
								GDGL_DEBUG("json parse error\n");
								cJSON_Delete(root);
								return;
							}
							attrname = json_temp->valuestring;
							printf("rf w_description=%s\n",attrname);

							if(w_mode <sizeof(rf_wmode_map)) {
								if(rf_wmode_map[w_mode] != 0xff) { //0xff无效
									//播放报警声音
									snprintf(audio_play_string, sizeof(audio_play_string), "mplayer -loop 0 /gl/res/%s", alarm_music_name[rf_wmode_map[w_mode]]);
									gateway_warning_operation(audio_play_string);

									json_temp = cJSON_GetObjectItem(root, "time");
									if (!json_temp)
									{
										GDGL_DEBUG("json parse error\n");
										cJSON_Delete(root);
										return;
									}
									warntime = json_temp->valuestring;
									printf("rf warntime=%s\n",warntime);
									rfwmode = w_mode;
									//检测报警联动条件
									if(rfwmode != rfDoorClose) {
										//printf("haah,dev_ieee=%s,dev_ep=%s\n", dev_ieee,dev_ep);
										list_linkage_compare_condition_trigger(dev_ieee, dev_ep, "alarm", ALARM_LINKAGE_DEFAULT_V, warntime);
										if((rfwmode == rfDoorOpen)||(rfwmode == rfBurglar))
											list_linkage_compare_condition_trigger(dev_ieee, dev_ep, "open", ALARM_LINKAGE_DEFAULT_V, NULL);
									}
								}
							}
						break;
						case SUBID_RF_OPEN_STATE_CHGE:
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
							json_temp = cJSON_GetObjectItem(root, "w_mode");
							if (!json_temp)
							{
								GDGL_DEBUG("json parse error\n");
								cJSON_Delete(root);
								return;
							}
							w_mode = atoi(json_temp->valuestring);
							json_temp = cJSON_GetObjectItem(root, "w_description");
							if (!json_temp)
							{
								GDGL_DEBUG("json parse error\n");
								cJSON_Delete(root);
								return;
							}
							attrname = json_temp->valuestring;

							rfwmode = w_mode;
							//所有大洋报警器播放门铃声
							if(rfwmode == rfDoorOpen) {
								char doorbellapi[300] = {0};
								snprintf(doorbellapi, sizeof(doorbellapi), "GET /cgi-bin/rest/network/AllIasWarningDeviceOperation.cgi?param1=%d&param2=0&param3=0&operatortype=4 "
										"HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", warningduration);
//								printf("%s\n", doorbellapi);
								http_get_method_by_socket(doorbellapi);
								//播放网关声音
								snprintf(audio_play_string, sizeof(audio_play_string), "mplayer -loop 0 /gl/res/%s", alarm_music_name[8]);
								gateway_warning_operation(audio_play_string);
							}

							//检测联动
							if((rfwmode == rfDoorOpen)||(rfwmode == rfBurglar))
								list_linkage_compare_condition_trigger(dev_ieee, dev_ep, "open", ALARM_LINKAGE_DEFAULT_V, NULL);
						break;
						case SUBID_RF_ACTIVATE_STATE_CHGE:
						break;
						case SUBID_RF_LIST_CHGE:
							invoke_by_datatype_fork(GET_RF_LIST_NUM, NULL);
						break;
						default: break;
					}
    	    	break;
#endif
    	    	case MAIND_GATEWAY_SETTING:
					json_temp = cJSON_GetObjectItem(root, FIELD_SUBID);
					if(!json_temp) {
						GDGL_DEBUG("json parse error\n");
						cJSON_Delete(root);
						return;
					}
					subid = json_temp->valueint;			//解析subid
    	    		switch(subid) {
    	    			case SETTING_RESPOND:
    						json_temp = cJSON_GetObjectItem(root, "warningduration");
    						if(!json_temp) {
    							GDGL_DEBUG("json parse error\n");
    							cJSON_Delete(root);
    							return;
    						}
    						attr_value = json_temp->valueint;
    						setting_warningduration(attr_value);
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
			list_linkage_compare_condition_trigger(dev_ieee, dev_ep, attrname, attr_value, NULL);
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
			json_temp = cJSON_GetObjectItem(root, "w_mode");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			w_mode = atoi(json_temp->valuestring);
			json_temp = cJSON_GetObjectItem(root, "w_description");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			attrname = json_temp->valuestring;
			printf("w_description=%s\n",attrname);
			if(w_mode <sizeof(zigbee_wmode_map)) {
				if(zigbee_wmode_map[w_mode] != 0xff) { //0xff无效

					zgwmode = w_mode;
					//所有大洋报警器播放门铃声
					if(zgwmode == zgDoorbell) {
						char doorbellapi[300] = {0};
						snprintf(doorbellapi, sizeof(doorbellapi), "GET /cgi-bin/rest/network/AllIasWarningDeviceOperation.cgi?param1=%d&param2=0&param3=0&operatortype=4 "
								"HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", warningduration);
						http_get_method_by_socket(doorbellapi);
					}
					//播放网关声音
					snprintf(audio_play_string, sizeof(audio_play_string), "mplayer -loop 0 /gl/res/%s", alarm_music_name[zigbee_wmode_map[w_mode]]);
					gateway_warning_operation(audio_play_string);
					json_temp = cJSON_GetObjectItem(root, "time");
					if (!json_temp)
					{
						GDGL_DEBUG("json parse error\n");
						cJSON_Delete(root);
						return;
					}
#ifdef USE_RF_FUNCTION
					//所有RF报警器报警
					if((zgwmode == zgBurglar)||(zgwmode == zgFire)||(zgwmode == zgEmergency)) { //Burglar, Fire, Emergency允许播放
						char rfwarningapi[300] = {0};
						snprintf(rfwarningapi, sizeof(rfwarningapi), "GET /cgi-bin/rest/network/RFWarningDevOperation.cgi?rfid=0&operatortype=4&param1=0 "
								"HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n");
						http_get_method_by_socket(rfwarningapi);
					}
#endif
					warntime = json_temp->valuestring;
					printf("zigbee warntime=%s\n",warntime);
					//检测报警联动条件
					if((zgwmode == zgBurglar)||(zgwmode == zgFire)||(zgwmode == zgEmergency)) //Burglar, Fire, Emergency允许联动
						list_linkage_compare_condition_trigger(dev_ieee, dev_ep, "alarm", ALARM_LINKAGE_DEFAULT_V, warntime);
				}
			}
    	break;
    	case DEV_BYPASS_CHANGE_MSGTYPE:
			json_temp = cJSON_GetObjectItem(root, "zone_ieee");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			dev_ieee = json_temp->valuestring;
			//printf("ieee=%s\n", dev_ieee);
			json_temp = cJSON_GetObjectItem(root, "w_mode");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			w_mode = atoi(json_temp->valuestring);
			zgwmode = w_mode;
			//printf("zgwmode=%d\n", zgwmode);
			if(zgwmode == zgBypass) {
				//检测设备是否是门磁
				char dev_mode_id[10]={0};
				if(http_get_dev_modeid(dev_mode_id, dev_ieee) ==0) {
					printf("dev_mode_id=%s\n", dev_mode_id);
					if(memcmp(dev_mode_id, DOOR_MODEID, 5) ==0) {
						//是门磁，所有大洋报警器播放门铃声
						char doorbellapi[300] = {0};
						snprintf(doorbellapi, sizeof(doorbellapi), "GET /cgi-bin/rest/network/AllIasWarningDeviceOperation.cgi?param1=%d&param2=0&param3=0&operatortype=4 "
									"HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", warningduration);
						//printf("%s\n", doorbellapi);
						http_get_method_by_socket(doorbellapi);
						//播放网关声音
						snprintf(audio_play_string, sizeof(audio_play_string), "mplayer -loop 0 /gl/res/%s", alarm_music_name[8]);
						gateway_warning_operation(audio_play_string);
					}
				}
			}
    	break;
    	case IAS_DEV_CHANGE_MSGTYPE:
			json_temp = cJSON_GetObjectItem(root, "callbackType");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			status = json_temp->valueint;
			if(status !=2)
			{
				GDGL_DEBUG("callbackType invalid\n");
				cJSON_Delete(root);
				return;
			}
			json_temp = cJSON_GetObjectItem(root, "IEEE");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			dev_ieee = json_temp->valuestring;
//			printf("ieee=%s\n",dev_ieee);
			json_temp = cJSON_GetObjectItem(root, "EP");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			dev_ep = json_temp->valuestring;
//			printf("ieee=%s\n",dev_ep);
			json_temp = cJSON_GetObjectItem(root, "value");
			if (!json_temp)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			json_temp2 = cJSON_GetObjectItem(json_temp, "alerm1");
			if (!json_temp2)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			alarm1 = json_temp2->valueint;
			json_temp2 = cJSON_GetObjectItem(json_temp, "alerm2");
			if (!json_temp2)
			{
				GDGL_DEBUG("json parse error\n");
				cJSON_Delete(root);
				return;
			}
			alarm2 = json_temp2->valueint;
//			printf("alarm1=%d,alarm2=%d\n",alarm1,alarm2);
			//检测联动条件
			if((alarm1 ==1)||(alarm2 ==1))
			{
				list_linkage_compare_condition_trigger(dev_ieee, dev_ep, "open", ALARM_LINKAGE_DEFAULT_V, NULL);
			}
//			else
//			{
//				list_linkage_compare_condition_trigger(dev_ieee, dev_ep, "normal", ALARM_LINKAGE_DEFAULT_V, NULL);
//			}
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
//如果有另一个包，触发多一次
	if(need_parse_again_flag ==1)
	{
		if(next_size >0){
			printf("[5018] data parse again\n");
			parse_json_callback(&text[next_size]);
		}
		else{
			GDGL_DEBUG("next_size invalid\n");
		}
	}
}
/*
 * this fuction remove the member of the list_time[member]
 * */
static void list_time_remove_member_lock(int member)
{
	if(member >= TID_MAX)
		return;
	if(list_time[member].tid !=-1) {
		printf("[tk] remove id=%d\n", list_time[member].tid);
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
			printf("[tk] remove id=%d\n", list_time[i].tid);
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
			printf("[tk] add id=%d\n", list_time[i].tid);
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
			printf("[tk] edit id=%d\n", list_time[i].tid);
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
void stop_music_callback(void *arg)
{
	system("killall mplayer");
	printf("[music] killall mplayer\n");

	if(stop_music_task) {
		timed_action_unschedule(notifier, stop_music_task);
		free(stop_music_task);
		stop_music_task = NULL;
		if(stop_music_task!=NULL)
			GDGL_DEBUG("free,but stop_music_task really not NULl\n");
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
	int res; //add yanly150703

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
			printf("[devBeDeleted] -tid=%d\n",will_del_id[i]);
			if((send_cb_len = cJsonTimeAction_callback(send_cb_string, SUBID_DEL_TA, 1, will_del_id[i], NULL)) >=0) {
				push_to_CBDaemon(send_cb_string, send_cb_len);
				usleep(300000); //300ms //发送太快调试工具接收不到
				//GDGL_DEBUG("send tima success\n");
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
			//GDGL_DEBUG("will changed sid:%d\n",will_del_id[i]);
			//printf("[devBeDeleted] -sid=%d\n",will_del_id[i]);
			res = read_t_scene_base_byid(&base_buf, will_del_id[i]);
			if(res <0) {
				if(res == ERROR_MDFY_NO_ID_DB) //如果没有这条id规则了，代表这条规则已被删除，发送删除的callback
				{
					printf("[devBeDeleted] -sid=%d\n",will_del_id[i]);
					if((send_cb_len = cJsonDelDoScene_callback(send_cb_string, will_del_id[i], SUBID_DEL_SCENE, 1)) >=0) {
						push_to_CBDaemon(send_cb_string, send_cb_len);
						usleep(300000); //300ms //发送太快调试工具接收不到
						//GDGL_DEBUG("send scene success\n");
					}
				}
				continue;
			}
			printf("[devBeDeleted] edit sid=%d\n",will_del_id[i]);
			if((send_cb_len = cJsonScene_callback(send_cb_string, &base_buf, NULL, NULL, SUBID_EDIT_SCENE, 1)) >=0) {
				push_to_CBDaemon(send_cb_string, send_cb_len);
				usleep(300000); //300ms //发送太快调试工具接收不到
				//GDGL_DEBUG("send scene success\n");
			}
		}
	}
	memset(will_del_id, 0, DEL_ID_MAX);
	//判断是否在联动规则里
	if(del_linkage_by_isdeletedieee(will_del_id, isdeleted_ieee) >0) {
		for(i=0; i<DEL_ID_MAX; i++) {
			if(will_del_id[i] == 0)
				break;
			//GDGL_DEBUG("will del lid:%d\n",will_del_id[i]);
			printf("[devBeDeleted] -lid=%d\n",will_del_id[i]);
			if((send_cb_len = cJsonLinkage_callback(send_cb_string, SUBID_DEL_LINK, 1, will_del_id[i], NULL)) >=0) {
				push_to_CBDaemon(send_cb_string, send_cb_len);
				usleep(300000); //300ms //发送太快调试工具接收不到
				//GDGL_DEBUG("send linkage success\n");
			}
		}
	}
	//GDGL_DEBUG("over\n");
	exit(0);
	return 0;
}
int system_to_do_mul_process(const char *string)
{
	int res;
	pid_t	pid;
	if ((pid = fork()) > 0) {
		waitpid(-1,NULL,0);
		return(pid);				//parent process
	}
	if ((pid = fork()) > 0) {
		exit(0);					//child process
		return (pid);
	}
	res = system(string);
	if(res <0) {
		GDGL_DEBUG("system call failed,res is: %d\n", res);
	}
	exit(0);
	return 0;
}
void setting_warningduration(int value)
{
	warningduration =value;
	printf("warningduration=%d\n", warningduration);
}
void gateway_warning_operation(const char *cmdstring)
{
	if(warningduration <=0) {
		return;
	}

	system("killall mplayer");
	printf("[music] killall mplayer\n");
	if(stop_music_task) {
		timed_action_unschedule(notifier, stop_music_task);
		free(stop_music_task);
		stop_music_task = NULL;
		if(stop_music_task!=NULL)
			GDGL_DEBUG("free,but stop_music_task really not NULl\n");
	}
	system_to_do_mul_process(cmdstring);
	printf("[music] %s\n", cmdstring);
	stop_music_task = timed_action_schedule(notifier, warningduration, 0, &stop_music_callback, NULL);
}


