/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName:      ui_daemon.c
  Author:        马晓槟 
  Version :      1.1   
  Date:          2014-09-16
  Description:   实现按键和指示灯的功能。
  增加clr键功能，实现停止网关报警和大洋报警器报警的功能；
  CompileCmd:    arm-linux-gcc cJSON/cJSON.c ui_daemon.c -lpthread -lm -o ui_daemon
  History:        
      <author>     <time>    <version >    <desc>
       马晓槟    2014-09-12    V1.0
       马晓槟    2014-09-16    V1.1       补充注释
       yanly	2015-06-11    V2.0		 修改指示灯和按键功能
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "cJSON.h"
#include <curl/curl.h>
#include "httpCurlMethod.h"
#include "glCalkProtocol.h"
#include "gatewayHwControl.h"

#define BUILD_RELEASE_VERSION   1
#define APP_NAME			"ui_daemon"
#define APP_VERSION			"V2.0"

#define KEY_RST             '1'
#define KEY_CLR				'2'
#define KEY_SET             KEY_RST
#define KEY_STAT_RELEASE    0
#define KEY_STAT_PRESS      1
#define KEY_CLR_CODE		1
#define KEY_SET_CODE		352

#define FUNC_1_TIME         3   // s，重置用户信息长按时间
#define FUNC_2_TIME         10  // s，恢复出厂设置长按时间
#define MIN_FLASH_TIME      3   // s, 最小闪烁时间
#define WAIT_TIME           5   // s, 等待服务器响应的时间

#define BUF_SIZE            1024 

#if BUILD_RELEASE_VERSION
#define REQ_IP              "127.0.0.1"
#else
#define REQ_IP              "127.0.0.1"
//#define REQ_IP              "192.168.1.107"
#endif
#define REQ_PORT            5013

#define JSON_Status         "status"
#define JSON_Success        1

//send callback flag
#define NOT_NEED_SEND_CALLBACK		0
#define RESET_USER_INFO_CALLBACK	1
#define	RESET_FACTORY_CALLBACK		2

//const char SetAllPermitJoinOn[] = "GET /cgi-bin/rest/network/SetAllPermitJoinOn.cgi?"
//                                "second=60&callback=1234&encodemethod=NONE&sign=AAA HTTP/1.1\r\n"
//                                "Host: 127.0.0.1\r\n\r\n";
const char SetAllPermitJoinOn[] = "GET /cgi-bin/rest/network/SetAllPermitJoinOnByIoControl.cgi HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n";
const char RstAccount[] = "{\"action\":\"accountreset\"}";
const char RstFactory[] = "{\"action\":\"factoryreset\"}";
const char stopAlarm[] ="http://127.0.0.1/cgi-bin/rest/network/StopAlarm.cgi";

extern int errno;

//add volatie flag for variables  by yanly150625
volatile time_t start_tm;
double press_tm;
volatile bool f_block, f_ignore;
volatile bool is_ignore_when_press;
volatile int rst_stat = KEY_STAT_RELEASE;
int send_callback_flag =0;  //1-用户名别名密码恢复callback，2-恢复出厂化设置callback

pthread_t tid, re_tid;
void* thrd_func(void* arg);
void* retry_func(void* arg);
int rst_req(const char* req, int rlen);
char result_str[65535];
int main()
{
	printf("===============%s version[%s], COMPILE_TIME[%s: %s]\n", APP_NAME, APP_VERSION, __DATE__, __TIME__);
    int key;
    int key_state;
    int fd;
    int ret = 0;
    struct input_event e_buf;
    char resetCallback[200];

//    int repeat_param[2];

    CURLcode rc;
    int rt;
	rc = curl_global_init(CURL_GLOBAL_ALL);
	if (rc != CURLE_OK) {
		printf("curl_global_init error\n");
		return -2;
	}
//    fd = open("/dev/input/event1", O_RDONLY);
    fd = open("/dev/input/event0", O_RDONLY); //modify by yanly150611
    if (fd < 0) 
    {
        printf("Open gpio-keys failed.\n");
        return -1; 
    } 
    else
    {
        printf("Open gpio-keys success.\n");
    }

    // 参数设置失败，无法使用此特性
    /*
    repeat_param[0] = 500;    // ms重复按键第一次间隔
    repeat_param[1] = 66;     // ms重复按键后续间隔
    ret = ioctl(fd, EVIOCSREP, repeat_param); // 设置重复按键参数
    if(ret != 0)
    {
        printf("%s\n", strerror(errno));
        printf("Set repeat_param fail!\n");
    }
    else
    {
        printf("Set repeat_param ok.\n");
    }
    */

    /*
     * 当UI_Daemon启动后会检测文件标识（password.old、FactoryReset）是否存在
     * 若存在则自动尝试进行相应的操作，在操作完成之前不响应RST键的事件
     */
    f_ignore = true;
    if (pthread_create(&re_tid, NULL, retry_func, NULL) != 0) 
    {
        printf("Create thread error!\n");
        close(fd);
        return -1;
    }

    if (pthread_create(&tid, NULL, thrd_func, NULL) != 0) 
    {
        printf("Create thread error!\n");
        close(fd);
        return -1;
    }

    //==========================================================================

//    int socket_fd;
//    int len;		//modify by yanly150611
    struct sockaddr_in remote_addr;
//    char buf[BUF_SIZE] = {0}; //modify by yanly150611
//    char* recv_data = NULL; //modify by yanly150611
//    int recv_data_size = 0; //modify by yanly150611

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    remote_addr.sin_port = htons(80);

    //==========================================================================

    while (1)
    {
        ret = read(fd, &e_buf, sizeof(struct input_event));
        if (ret <= 0)
        {
            printf("Read fail!\n");
            return -1;
        }

        key = e_buf.code;
        printf("code is %d\n",key);
        key_state = e_buf.value;
        switch (key)
        {
//            case KEY_1: key = '1'; break;
//            case KEY_2: key = '2'; break;
            case KEY_CLR_CODE: key = KEY_CLR; break;  //modify by yanly150611
            case KEY_SET_CODE: key = KEY_SET; break;
            default: break;
        }

        if (key != 0)
        {
            printf("KEY_%c state = %d.\n", key, key_state);
        }

        //add by yanly150626
        //CLR键，实现停止报警功能
        if (key == KEY_CLR && key_state == KEY_STAT_RELEASE) {

        	printf("clear alarm pressed\n");
        	rt = http_curl_get(stopAlarm, result_str);
        	if(rt <0) {
        		printf("invoke api failed\n");
        		return -3;
        	}
        	printf("stop alarm ok\n");
        }

        if (key == KEY_RST)
        {
            if (key_state == KEY_STAT_PRESS)
            {
                is_ignore_when_press = f_ignore;
                if (f_ignore) 
                {
                    continue;
                }

                printf("start timing\n");
                rst_stat = KEY_STAT_PRESS;
                start_tm = time(NULL);
            }
            else if (key_state == KEY_STAT_RELEASE && !is_ignore_when_press)
            {
                int diff = 0;
                printf("stop timing\n");
                rst_stat = KEY_STAT_RELEASE;
                press_tm = difftime(time(NULL), (time_t)start_tm);
                printf("press time = %lf\n", press_tm);
                if (press_tm >= FUNC_1_TIME && press_tm < FUNC_2_TIME)
                {
                    while (f_block);
                    diff = press_tm - FUNC_1_TIME;
                    if (diff < MIN_FLASH_TIME)
                    {
                        sleep(MIN_FLASH_TIME - diff);
                    }

#if BUILD_RELEASE_VERSION
//                    if(send_callback_flag == NOT_NEED_SEND_CALLBACK) {
                    	send_callback_flag = RESET_USER_INFO_CALLBACK;
//                    }
//                    printf("reboot\n");
//                    system("reboot");
//                    sleep(3); //加个延时，为了在reboot前不继续运行到接下来的程序
#else
                    printf("reboot\n");
                    system("echo none > /sys/class/leds/12/trigger");
#endif
                }
                else if (press_tm >= FUNC_2_TIME)
                {
                    while (f_block);
                    diff = press_tm - FUNC_2_TIME;
                    if (diff < MIN_FLASH_TIME)
                    {
                        sleep(MIN_FLASH_TIME - diff);
                    }
#if BUILD_RELEASE_VERSION
//                    if(send_callback_flag != NOT_NEED_SEND_CALLBACK) {
                    	send_callback_flag = RESET_FACTORY_CALLBACK;
//                    }
//                    printf("reboot\n");
//                    system("reboot");
//                    sleep(3); //加个延时，为了在reboot前不继续运行到接下来的程序
#else
                    printf("reboot\n");
                    system("echo none > /sys/class/leds/12/trigger");
#endif
                }
                if(send_callback_flag != NOT_NEED_SEND_CALLBACK) {
                	//send callback
                	snprintf(resetCallback, sizeof(resetCallback),
                			"{\n	\"msgtype\":	0,\n	\"mainid\":	1,\n	\"subid\":	3,\n	\"status\": %d\n}", send_callback_flag);
                	push_to_CBDaemon(resetCallback, strlen(resetCallback)+1);
                	printf("%s\n", resetCallback);
                	send_callback_flag = NOT_NEED_SEND_CALLBACK;
                    printf("reboot\n");
                    system("sync");
                    sleep(1);
                    system("reboot");
                    sleep(3); //加个延时，为了在reboot前不继续运行到接下来的程序
                }
            }
        }
        //rst键，按一下实现入网功能
		if (key == KEY_RST && key_state == KEY_STAT_RELEASE)
		{
			printf("set SetAllPermitJoinOnByIoControl\n");
			system(SETIO_L);
			sleep(1);
			system(SETIO_H);
//
//
//			if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
//			{
//				close(socket_fd);
//				printf("socket create error\n");
//				continue;
//			}
//
//			if (connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) < 0)
//			{
//				close(socket_fd);
//				printf("socket connect error\n");
//				continue;
//			}
//			send(socket_fd, SetAllPermitJoinOn, strlen(SetAllPermitJoinOn), 0);
//			close(socket_fd);
		}
    }
    close(fd);
    curl_global_cleanup();
    return 0;
}

void* thrd_func(void* arg)
{
    int ret;
    while (1)
    {
        if (rst_stat == KEY_STAT_PRESS
                && (int)difftime(time(NULL), start_tm) == FUNC_1_TIME
                && !is_ignore_when_press)
        {
            f_block = true;
            printf("reset account\n");
//            system("echo timer > /sys/class/leds/12/trigger");
            system("echo timer > /sys/class/leds/sys_led/trigger");
            ret = rst_req(RstAccount, sizeof(RstAccount));
            if (ret == 0)
            {
                // 服务器重置用户信息成功，则直接删除alias.conf、password.conf
            	printf("server reset account success\n");
                system("rm -rf /gl/etc/alias.conf /gl/etc/password.*");
                printf("rm -rf /gl/etc/alias.conf /gl/etc/password.* ok\n");
            }
            else
            {
                // 服务器重置用户信息失败，则保留password.conf（将其重命名为password.old）
                // 因为网关还需使用旧密码登录服务器
                printf("reset account fail\n");
                system("rm -rf /gl/etc/alias.conf");
                system("[ ! -f /gl/etc/password.old ] && mv /gl/etc/password.conf /gl/etc/password.old");
            }
            printf("sync before\n");
            system("sync");
            printf("sync ok\n");
            f_block = false;
        }
        
        if (rst_stat == KEY_STAT_PRESS
                && (int)difftime(time(NULL), (time_t)start_tm) == FUNC_2_TIME
                && !is_ignore_when_press)
        {
            f_block = true;
            printf("reset factory setting\n");
//            system("echo 80 > /sys/class/leds/12/delay_on");
//            system("echo 80 > /sys/class/leds/12/delay_off");
            system("echo 80 > /sys/class/leds/sys_led/delay_on");
            system("echo 80 > /sys/class/leds/sys_led/delay_off");
            ret = rst_req(RstFactory, sizeof(RstFactory));
            if (ret == 0)// 服务器恢复出厂设置成功，网关也进行恢复操作
            {

//	1. 恢复初始用户名密码信息(程序进行到这里，这一步骤已在上一步完成)
//	2. zigbee恢复出厂设置
//	3. 清空大洋配置文件：rm -rf /mnt/jffs2/*，cp -rf /gl/backup/* /mnt/jffs2/
//	4. 删除视频设备配置信息数据库：/gl/etc/video/video_conf.db
//	5. 删除定时场景联动数据库：/gl/etc/database/application.db
//	6. 删除RF设备列表信息文件：/gl/etc/rf_dev.json
//	7. 删除绑定列表信息: /gl/etc/bindlist.json，/gl/etc/binddev.json
            	/*
            	 * 2. 恢复网关zigbee出厂设置：
            	 * （1）停止zigbee供电
            	 * （2）控制zigbee按键按下
            	 * （3）控制zigbee灯变暗
            	 * （4）恢复zigbee供电
            	 * （5）停止按zigbee按键
            	 * （6）延时1s
            	 * （7）停止zigbee供电
            	 * （8）控制zigbee灯变亮
            	 * （9）恢复zigbee供电
            	 * */
                printf("reset factory setting success\n");
//                system("rm -rf /gl/etc/FactorySetting");
                // for debug 
//                system("touch /gl/etc/FactoryReset-`date +%Y%m%d%H%M%S`");
                printf("execute factory reset shell\n");
                system("/gl/bin/factory_reset.sh");
//                system("rm -rf /mnt/jffs2/*");
//                system("cp -rf /gl/backup/* /mnt/jffs2/");

//                /* TODO:
//                 * 上面的操作对于ZigBee设备恢复出厂设置不够彻底。
//                 * 完成上面的操作后，所有ZigBee设备仍在同一个ZigBee网络内。
//                 * 当某个设备重新上电或被激活或与协调器通信后，该设备又会重新出现在设备列表中；
//                 * 设备的绑定信息被保存在输出簇的设备中，因此绑定控制仍然有效。
//                 */
            }
            else
            {
                // 服务器恢复出厂设置失败，则创建FactorySetting文件作为标识
                // 下次启动UI_Daemon时再尝试该操作
                printf("reset factory setting fail\n");
                system("touch /gl/etc/FactorySetting");
            }
            system("sync");
            printf("sync\n");
            f_block = false;
        }
        sleep(1);
    }
}

void* retry_func(void* arg)
{
    int ret_account = -1;
    int ret_factory = -1;
    
    if (access("/gl/etc/password.old", F_OK) == 0)
    {
        printf("retry to reset account\n");
        ret_account = rst_req(RstAccount, sizeof(RstAccount));
        if (ret_account == 0)
        {
            printf("reset account success\n");
            system("rm -rf /gl/etc/alias.conf /gl/etc/password.*");
        }
    }

    if (access("/gl/etc/FactorySetting", F_OK) == 0)
    {
        printf("retry to reset factory setting\n");
        ret_factory = rst_req(RstFactory, sizeof(RstFactory));
        if (ret_factory == 0)
        {
            printf("reset factory setting success\n");
            system("rm -rf /gl/etc/FactorySetting");
        }
    }

    if (ret_account == 0 || ret_factory == 0)
    {
        system("sync");
#if BUILD_RELEASE_VERSION
        system("reboot");
#else
        printf("reboot\n");
#endif
    }

    /*
     * 执行到此处说明所有Retry操作失败或者不需要进行Retry操作
     * 于是将f_ignore赋值为false，开始响应RST键的事件
     */
    f_ignore = false;
    return NULL; //add by yanly150611
}

int rst_req(const char* req, int rlen)
{

    int socket_fd;
    int len;
    int rs = 1;
    struct sockaddr_in remote_addr;
    char buf[BUF_SIZE] = {0};
    char* recv_data = NULL;
    int recv_data_size = 0;

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(REQ_IP);
    remote_addr.sin_port = htons(REQ_PORT);

    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        close(socket_fd);
        printf("socket create error\n");
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) < 0)
    {
        close(socket_fd);
        printf("socket connect error\n");
        return -1;
    }

    fcntl(socket_fd, F_SETFL, O_NONBLOCK);
    send(socket_fd, req, rlen, 0);

    sleep(WAIT_TIME);
    while (rs)
    {
        len = recv(socket_fd, buf, sizeof(buf), 0);
        if (len > 0)
        {
            recv_data = (char*)realloc(recv_data, sizeof(char) * (recv_data_size + len + 1));
            memcpy(recv_data + recv_data_size, buf, len);
            recv_data_size += len;
            *(recv_data + recv_data_size) = '\0';
        }
        else if (len < 0)
        {
            // 由于是非阻塞的模式，所以当errno为EAGAIN时，表示当前缓冲区已无数据可读
            // 在这里就当作是该次事件已处理
            if (errno == EAGAIN)
            {
                printf("stack buffer is empty\n");
            }
            else
            {
                printf("%s\n", strerror(errno));
            }
            break;
        }
        else if (len == 0)
        {
            // 对端的socket已正常关闭
            printf("remote socket already closed normally\n");
        }

        if (len == sizeof(buf))
        {
            // 如果读取的长度等于缓冲区的长度，则很可能数据未读完，需要再次读取
            rs = 1;
        }
        else
        {
            rs = 0;
        }
    }

    close(socket_fd);

    if (!recv_data)
    {
        return -1;
    }

    printf("recv_data:%s\n", recv_data);

    cJSON* json = cJSON_Parse(recv_data);
    if (!json)
    {
        printf("json parse error\n");
        return -1;
    }
    free(recv_data);

    cJSON* json_tmp = cJSON_GetObjectItem(json, JSON_Status);
    if (!json_tmp)
    {
        printf("invalid "JSON_Status"\n");
        return -1;
    }

    int ret = atoi(cJSON_PrintUnformatted(json_tmp));
    cJSON_Delete(json);

    if (ret == JSON_Success)
    {
        return 0;
    }
    else
    {
        printf(JSON_Status" = %d\n", ret);
        return -1;
    }
}

