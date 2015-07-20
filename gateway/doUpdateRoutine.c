/*
 *	File name   : doUpdateRoutine.c
 *  Created on  : Jul 17, 2015
 *  Author      : yanly
 *  Description : 执行网关升级的程序
 *  Version     : V01-00
 *  History     : <author>		<time>		<version>		<desc>
 */


//步骤
/*
b1. 点亮sys灯
b2. 创建标志文件》/gl/etc/update.flag
b3. 解压升级包》/gl/update/                升级包名字：update-<model>-<version>-<date>.tar.gz
b4. 执行解压包里的升级脚本update.sh
b5. 删除标记文件
b6. 删除升级临时文件
b7. 返回callback告诉客户端升级完成
b8. 熄灭sys灯
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "cJSON.h"
#include "glCalkProtocol.h"


#define APP_VERSION			"V01-00"

#define OLD_OPEN_SYS_LED			"echo default-on > /sys/class/leds/12/trigger"
#define OLD_CLOSE_SYS_LED			"echo none > /sys/class/leds/12/trigger"

#define NEW_OPEN_SYS_LED			"echo 1 > /sys/class/leds/user_led1/brightness"
#define NEW_CLOSE_SYS_LED			"echo 0 > /sys/class/leds/user_led1/brightness"
#define OPEN_SYS_LED				NEW_OPEN_SYS_LED
#define CLOSE_SYS_LED				NEW_CLOSE_SYS_LED
#define TOUCH_UPDATE_FLAG			"touch /gl/etc/update.flag"
#define EXTRACT_UPDATE_PACKAGE		"tar -xvzf /gl/update/update-*.tar.gz -C /gl/update/"
#define EXEC_UPDATE_SHELL			"/gl/update/update*/update.sh"
#define RM_UPDATE_FLAG				"rm -f /gl/etc/update.flag"
#define RM_UPDATE_PACKAGE			"rm -f /gl/update/update-*.tar.gz"
#define RM_UPDATE_TMPDIR			"rm -f -R /gl/update/update*/"

int main()
{
	int res=0;
	char reboot_flag=0;
    cJSON *root;
    char *json_out;

    printf("===============DoUpdate version:%s, COMPILE_TIME[%s: %s]\n", APP_VERSION, __DATE__, __TIME__);

    printf("%s\n", TOUCH_UPDATE_FLAG);
	res = system(TOUCH_UPDATE_FLAG);
	printf("res=%d\n",res);
	if(res !=0) {
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		goto over;
	}

	printf("%s\n", OPEN_SYS_LED);
	res = system(OPEN_SYS_LED);
	printf("res=%d\n",res);
	if(res !=0) {
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		goto over; //灯打开失败还是继续执行升级操作？
	}

	printf("%s\n", EXTRACT_UPDATE_PACKAGE);
	res = system(EXTRACT_UPDATE_PACKAGE);
	printf("res=%d\n",res);
	if(res !=0) {
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		goto over;
	}

	printf("%s\n", EXEC_UPDATE_SHELL);
	res = system(EXEC_UPDATE_SHELL);
	printf("res=%d\n",res);
	if(res !=0) {
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		goto over;
	}

	printf("%s\n", RM_UPDATE_FLAG);
	res = system(RM_UPDATE_FLAG);
	printf("res=%d\n",res);
	if(res !=0) {
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		goto over;
	}

	printf("%s\n", RM_UPDATE_PACKAGE);
	res = system(RM_UPDATE_PACKAGE);
	printf("res=%d\n",res);
	if(res !=0) {
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		goto over;
	}

	printf("%s\n", RM_UPDATE_TMPDIR);
	res = system(RM_UPDATE_TMPDIR);
	printf("res=%d\n",res);
	if(res !=0) {
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		goto over;
	}

over:
	if(res !=0)
		res = -1;
    root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "msgtype", 0);
	cJSON_AddNumberToObject(root, "mainid", 6);
	cJSON_AddNumberToObject(root, "subid", 2);
	cJSON_AddNumberToObject(root, "status", res);
	cJSON_AddNumberToObject(root, "need_reboot", reboot_flag);
	json_out=cJSON_Print(root);
	json_out[strlen(json_out)] = '\0';
    push_to_CBDaemon(json_out, strlen(json_out)+1);
	free(json_out);
	cJSON_Delete(root);

	system(CLOSE_SYS_LED);

	return 0;
}

