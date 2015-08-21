/*
 *	File name   : auth_daemon.c
 *  Created on  : Aug 10, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <unistd.h>
#define __USE_XOPEN
#include <time.h>
#include "unpthread.h"
#include "smartgateway.h"
#include "cJSON.h"
//#include "callbackProtocolField.h"
#include "glCalkProtocol.h"
#include "timedaction.h"
#include "auth_daemon.h"
#include "auth_callback.h"
#include "auth_push2cb.h"

static pthread_mutex_t	auth_lock = PTHREAD_MUTEX_INITIALIZER;

auth_st auth_limit;
//volatile int expire_timestamp;			//授权时间到期的时间戳
//volatile int nolonger_remind_time;		//到期后不再提醒的时间戳 = expire_timestamp + remind_after

//时间任务相关
static void judge_auth_state_expire_timer();

//文件操作相关
static void read_auth_json_file();
int parse_text_2_auth_st(const char *stext);

//socket操作相关
static void auth_fd_doit(int sockfd);

static void printf_auth_st(auth_st tmep);

int check_auth_st(auth_st *temp_auth);

//文件锁
struct flock* file_lock(short type, short whence)
{
    static struct flock ret;
    ret.l_type = type ;
    ret.l_start = 0;
    ret.l_whence = whence;
    ret.l_len = 0;
    ret.l_pid = getpid();
    return &ret;
}

int main()
{
	printf("===============%s version[%s], COMPILE_TIME[%s: %s]\n", APP_NAME, APP_VERSION, __DATE__, __TIME__);
//	struct tm tmp_tm_time;
//	int timestamp;
	int auth_fd;
	static socklen_t auth_addr_len;

	auth_multi_timer_init();

	//读授权文件
	read_auth_json_file();

	//判断授权有效性,设置对应的timer
	judge_auth_state_expire_timer();

	//监听5022端口，更新到期时间，重新设置延时触发
	auth_fd = Tcp_listen(NULL, FEATURE_GDGL_AUTH_LISTEN_PORT_STR, &auth_addr_len);
	socklen_t		clilen;
	struct sockaddr_in	cliaddr;
	int	connfd;
	while(1) {

		connfd = Accept(auth_fd, (struct sockaddr*) &cliaddr, &clilen);

		printf("[Accept]addr=%s,sock=%d\n",inet_ntoa(cliaddr.sin_addr), connfd);
		if( memcmp(inet_ntoa(cliaddr.sin_addr), "127.0.0.1", 9) ==0 ) {
			printf("addr is localhost\n");
		}

		auth_fd_doit(connfd);
		Close(connfd);
	}

	return 0;
}
//获取auth.json文件内容，写到结构体：auth_st auth_limit;
static void read_auth_json_file()
{
	cJSON* json_all;
    FILE* fp;
    int fd;
    int file_size;
    char file_buf[1024];
    char* tmp;
    char need_new_auth_file=0;
    int res_parse =0;


    fp = fopen(FEATURE_GDGL_AUTH_PATH, "a+");
    fd = fileno(fp);
    fcntl(fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    if (file_size > 0)
    {
        fread(file_buf, sizeof(char), file_size, fp);
        file_buf[file_size] = '\0';
        printf("auth.json=%s\n",file_buf);
        res_parse = parse_text_2_auth_st(file_buf);
        if(res_parse !=0) {
        	need_new_auth_file =1;
        }
        else {
            fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
            fclose(fp);
        }
    }
    else
    {
    	need_new_auth_file =1;
    }

    if(need_new_auth_file ==1) {
    	printf("******new default auth.json file*******\n******\n*****\n");
        json_all = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_all, "state", AUTH_STATE_DEFAULT);
        cJSON_AddStringToObject(json_all, "expire_time", AUTH_EXPIRETIME_DEFAULT);
        cJSON_AddNumberToObject(json_all, "available", AUTH_AVAILABLE_DEFAULT);
        cJSON_AddNumberToObject(json_all, "remind_before_expire", AUTH_BEFORE_REMIND_DEFAULT);
        cJSON_AddNumberToObject(json_all, "remind_after_expire", AUTH_AFTER_REMIND_DEFAULT);
        tmp = cJSON_Print(json_all);
    	if(truncate(FEATURE_GDGL_AUTH_PATH, 0)<0) {  //清空文件内容
    		GDGL_DEBUG("truncate failed\n");
    	}
        fwrite(tmp, sizeof(char), strlen(tmp), fp);
        snprintf(file_buf, sizeof(file_buf), "%s", tmp);
        free(tmp);
        fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
        fclose(fp);
        parse_text_2_auth_st(file_buf);
    }
}
/*
 * <0 is parse error, =0 is success
 * */
int parse_text_2_auth_st(const char *stext)
{
	cJSON* json_all;
	cJSON* json_temp;
	char *expire_time;
	auth_st get_file_auth;
	struct tm tmp_tm_time;
	int timestamp;

	json_all = cJSON_Parse(stext);
	if(!json_all) {
		GDGL_DEBUG("cJSON_Parse failed\n");
		return -1;
	}
    json_temp = cJSON_GetObjectItem(json_all, "state");
	if(!json_temp) {
		GDGL_DEBUG("cJSON_GetObjectItem failed\n");
		cJSON_Delete(json_all);
		return -1;
	}
    get_file_auth.state = json_temp->valueint;
    json_temp = cJSON_GetObjectItem(json_all, "expire_time");
	if(!json_temp) {
		GDGL_DEBUG("cJSON_GetObjectItem failed\n");
		cJSON_Delete(json_all);
		return -1;
	}
    expire_time = json_temp->valuestring;
    snprintf(get_file_auth.expire_time, strlen(expire_time)+1, "%s", expire_time);
    get_file_auth.expire_time[strlen(expire_time)+1] = '\0';
    json_temp = cJSON_GetObjectItem(json_all, "available");
	if(!json_temp) {
		GDGL_DEBUG("cJSON_GetObjectItem failed\n");
		cJSON_Delete(json_all);
		return -1;
	}
    get_file_auth.available = json_temp->valueint;
    json_temp = cJSON_GetObjectItem(json_all, "remind_before_expire");
	if(!json_temp) {
		GDGL_DEBUG("cJSON_GetObjectItem failed\n");
		cJSON_Delete(json_all);
		return -1;
	}
    get_file_auth.remind_before = json_temp->valueint;
    json_temp = cJSON_GetObjectItem(json_all, "remind_after_expire");
	if(!json_temp) {
		GDGL_DEBUG("cJSON_GetObjectItem failed\n");
		cJSON_Delete(json_all);
		return -1;
	}
    get_file_auth.remind_after = json_temp->valueint;
    cJSON_Delete(json_all);

	strptime(get_file_auth.expire_time, "%Y-%m-%d %H:%M:%S", &tmp_tm_time);
	timestamp = mktime(&tmp_tm_time);
	get_file_auth.expire_timestamp = timestamp;
	get_file_auth.nolonger_remind_time = timestamp + get_file_auth.remind_after*24*3600;

//	printf_auth_st(get_file_auth);
	//判断参数值是否合理，不合理修改相关参数，重写文件
	if(check_auth_st(&get_file_auth) !=0) {
	    printf("check_auth_file value unreasonable, reset!\n");
//	    printf_auth_st(get_file_auth);
		update_auth_json_file_by_auth_st(&get_file_auth);
	}
    setting_auth_limit_st(get_file_auth);
    return 0;
}

//监听连接的socket，把内容覆盖到结构体auth_st auth_limit，并重新设置callback函数expire_time_arrival_callback
static void auth_fd_doit(int sockfd)
{
	int nfd;
	ssize_t		nread;
	char		request[MAX_REQUEST];
	int res;
	cJSON* root;
	char send_cb_string[GL_CALLBACK_MAX_SIZE];
	int send_cb_len;
	char *tmp;
//	auth_st get_socket_auth;
//	char *time1;

    while (1) {
		if ( (nfd = readable_timeo(sockfd, AUTH_READ_TIMEOUT)) < 0) {
			GDGL_DEBUG("readable_timeo error\n");
		    return;
		}
		else if (nfd == 0) { //timeout , return & close connection
			GDGL_DEBUG("eadable_timeo timeout\n");
		    return;
		}

	    if ( (nread = read(sockfd, request, MAXLINE-1)) < 0 ) {
		    GDGL_DEBUG("read error\n");
		    return;
        }
	    else if ( nread == 0) {
	    	GDGL_DEBUG("connection closed by other end\n");
		    return;		/* connection closed by other end */
	    }
	    request[nread] = 0; // add null
	    printf("read=%s\n", request);

	    update_auth_json_file_by_string(request);	//更新文件
	    res = parse_text_2_auth_st(request);
	    if(res !=0) {
	    	GDGL_DEBUG("read data invalid\n");
	    	return;
	    }
	    root = cJSON_Parse(request);
		cJSON_AddNumberToObject(root, MSGTYPE_STRING, GL_MSGTYPE_VALUE);
		cJSON_AddNumberToObject(root, GL_MAINID, GL_MAINID_AUTH);
		cJSON_AddNumberToObject(root, GL_SUBID, 3);
	    tmp = cJSON_Print(root);
	    snprintf(send_cb_string, sizeof(send_cb_string), "%s", tmp);
	    free(tmp);
		send_cb_len = strlen(send_cb_string)+1;
		push_to_CBDaemon(send_cb_string, send_cb_len); //发送授权更新消息

	    judge_auth_state_expire_timer();
	    //over
	    return;
    }
}
//判断授权状态，设置相应的延时callback
static void judge_auth_state_expire_timer()
{
	int now_time = time(NULL);
	printf("now time=%d\n", now_time);
	time_t interval1 = auth_limit.expire_timestamp-now_time;
//	time_t interval2 = auth_limit.nolonger_remind_time -now_time;
	//时间异常待处理...

	if(expire_task!=NULL) {
		GDGL_DEBUG("timed_action_unschedule(expire_task)\n");
		timed_action_unschedule(auth_notifier, expire_task);
		free(expire_task);
		expire_task = NULL;
		if(expire_task!=NULL)
			printf("free,but expire_task really not NULl\n");
	}
	if(remiding_task!=NULL) {
		GDGL_DEBUG("timed_action_unschedule(remiding_task)\n");
		timed_action_unschedule(auth_notifier, remiding_task);
		free(remiding_task);
		remiding_task = NULL;
		if(remiding_task!=NULL)
			printf("free,but remiding_task really not NULl\n");

	}
	printf_auth_st(auth_limit);
	switch(auth_limit.state) {

		case INSIDER_TEST_STATE:
		break;

		case NORMAL_STATE:
//			if(interval1 <0) { //当前时间>过期时间，但是state==正常，重新设置state并写auth.json
//				printf("now time > expire time, but state is normal state\n");
//				setting_auth_limit_state(ARREARAGE_STATE);
//				printf("set state=%d\n", auth_limit.state);
//			}
			printf("timed_action_schedule, delaytime=%ld\n",interval1);
			expire_task = timed_action_schedule(auth_notifier, interval1, 0, &expire_time_arrival_callback, NULL);
			if(expire_task==NULL) {
				GDGL_DEBUG("timed_action_schedule failed\n");
				exit(1);
			}
		break;

		case NONACTIVATED_STATE:
			send_service_stop_callback();
		break;

		case ARREARAGE_STATE:
			if(auth_limit.available == AVAILABLE_INVALID) {
				send_service_stop_callback();
			}
			else { //欠费还允许使用的时间期限内
				printf("timed_action_schedule_periodic, delaytime=%d\n",REMIND_PERIOD_TIME);
				send_reminding_callback(auth_limit.expire_time);
				remiding_task = timed_action_schedule_periodic(auth_notifier, REMIND_PERIOD_TIME, 0, &remiding_time_arrival_callback, NULL);
				if(remiding_task==NULL) {
					GDGL_DEBUG("timed_action_schedule failed\n");
					exit(1);
				}
			}
		break;

		case LOGOFF_STATE:
			send_service_stop_callback();
		break;

		default:break;
	}
}
//检查auth_st值有没有异常，正常返回0，异常有修改返回1
int check_auth_st(auth_st *temp_auth)
{
	//temp_auth可以被修改
	int status=0;
	time_t now_time = time(NULL);

	if(temp_auth-> expire_timestamp-now_time <0) { //当前时间>过期时间，但是state==正常，重新设置state
		if(temp_auth->state == NORMAL_STATE) {
			temp_auth->state = ARREARAGE_STATE;
			status = 1;
			printf("now time > expire time, state==normal state, reset state value\n");
		}
	}
	if(temp_auth-> nolonger_remind_time-now_time <0) { //当前时间>欠费仍可使用的时间，但是avialable==有效，重新设置avialable
		if(temp_auth->available == AVAILABLE_VALID) {
			if(temp_auth->state == ARREARAGE_STATE) {
				temp_auth->available = AVAILABLE_INVALID;
				status = 1;
				printf("now time > nolonger_remind_time, available==VALID, reset available value\n");
			}

		}
	}
	return status;
}
void update_auth_json_file_by_auth_st(const auth_st *temp_auth)
{
	cJSON* json_all;
    FILE* fp;
    int fd;
    char *out;

	if(truncate(FEATURE_GDGL_AUTH_PATH, 0)<0) {  //清空文件内容
		GDGL_DEBUG("truncate failed\n");
	}

    fp = fopen(FEATURE_GDGL_AUTH_PATH, "a+");
    fd = fileno(fp);
    fcntl(fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));
    fseek(fp, 0L, SEEK_END);
    rewind(fp); //将文件内部的指针重新指向一个流的开头

	json_all = cJSON_CreateObject();
	cJSON_AddNumberToObject(json_all, "state", temp_auth->state);
	cJSON_AddStringToObject(json_all, "expire_time", temp_auth->expire_time);
	cJSON_AddNumberToObject(json_all, "available", temp_auth->available);
	cJSON_AddNumberToObject(json_all, "remind_before_expire", temp_auth->remind_before);
	cJSON_AddNumberToObject(json_all, "remind_after_expire", temp_auth->remind_after);
	out = cJSON_Print(json_all);

	fwrite(out, sizeof(char), strlen(out), fp);
	free(out);
	cJSON_Delete(json_all);

    fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
    fclose(fp);
    printf("update_auth_json_file\n");
//    printf_auth_st(*temp_auth);
}
void update_auth_json_file_by_string(const char *wtext)
{
    FILE* fp;
    int fd;

	if(truncate(FEATURE_GDGL_AUTH_PATH, 0)<0) {  //清空文件内容
		GDGL_DEBUG("truncate failed\n");
	}
    fp = fopen(FEATURE_GDGL_AUTH_PATH, "a+");
    fd = fileno(fp);
    fcntl(fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));
    fseek(fp, 0L, SEEK_END);
    rewind(fp); //将文件内部的指针重新指向一个流的开头
	fwrite(wtext, sizeof(char), strlen(wtext), fp);
    fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
    fclose(fp);
}
static void printf_auth_st(const auth_st tmep)
{
    printf("state=%d\n",tmep.state);
    printf("expire_time=%s\n",tmep.expire_time);
    printf("available=%d\n",tmep.available);
    printf("remind_before=%d\n",tmep.remind_before);
    printf("remind_after=%d\n",tmep.remind_after);
    printf("expire_timestamp=%ld\n",tmep.expire_timestamp);
    printf("nolonger_remind_time=%ld\n",tmep.nolonger_remind_time);
}
void set_expire_timestamp(time_t temp)
{
	Pthread_mutex_lock(&auth_lock);
	auth_limit.expire_timestamp = temp;
	Pthread_mutex_unlock(&auth_lock);
}
void set_nolonger_remind_time(time_t temp)
{
	Pthread_mutex_lock(&auth_lock);
	auth_limit.nolonger_remind_time = temp;
	Pthread_mutex_unlock(&auth_lock);
}
void setting_auth_limit_st(const auth_st temp)
{
	Pthread_mutex_lock(&auth_lock);
	auth_limit = temp;
	Pthread_mutex_unlock(&auth_lock);
}
void setting_auth_limit_state(int i)
{
	Pthread_mutex_lock(&auth_lock);
	auth_limit.state = i;
	Pthread_mutex_unlock(&auth_lock);
}
void setting_auth_limit_available(int i)
{
	Pthread_mutex_lock(&auth_lock);
	auth_limit.available = i;
	Pthread_mutex_unlock(&auth_lock);
}
