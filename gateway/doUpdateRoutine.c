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
#include <unistd.h>
#include <fcntl.h>
#include "cJSON.h"
#include "glCalkProtocol.h"
#include "smartgateway.h"

#define APP_VERSION			"V01-00"

#define OLD_OPEN_SYS_LED			"echo default-on > /sys/class/leds/12/trigger"
#define OLD_CLOSE_SYS_LED			"echo none > /sys/class/leds/12/trigger"

#define NEW_OPEN_SYS_LED			"echo 1 > /sys/class/leds/user_led1/brightness"
#define NEW_CLOSE_SYS_LED			"echo 0 > /sys/class/leds/user_led1/brightness"
#define OPEN_SYS_LED				NEW_OPEN_SYS_LED
#define CLOSE_SYS_LED				NEW_CLOSE_SYS_LED
#define TOUCH_UPDATE_FLAG			"touch /gl/etc/update.flag"
//#define EXTRACT_UPDATE_PACKAGE		"tar -xvzf /gl/update/update-*.tar.gz -C /gl/update/"
#define EXTRACT_UPDATE_PACKAGE		"tar -xvzf `ls -t /gl/update/*.tar.gz | head -n 1` -C /gl/update/"     //解压最新的压缩包
//#define EXEC_UPDATE_SHELL			"/gl/update/update*/update.sh"
#define EXEC_UPDATE_SHELL			"`ls -td /gl/update/*/ |head -n 1`update.sh"     //执行最新的更新脚本
#define RM_UPDATE_FLAG				"rm -f /gl/etc/update.flag"
#define RM_UPDATE_PACKAGE			"rm -f /gl/update/update-*.tar.gz"
#define RM_UPDATE_TMPDIR			"rm -f -R /gl/update/update*/"
#define RM_TEMP_FILES				"rm -f -R /gl/etc/update.flag /gl/update/*"

//#define FIND_REBOOT_FLAG_FILE		"ls /gl/update/update-*/need_to_reboot.flag"
#define FIND_REBOOT_FLAG_FILE		"ls `ls -td /gl/update/*/ |head -n 1`need_to_reboot.flag"    //获取最新的更新目录是否有reboot标记文件
#define FIND_PACKAGE_NAME_CMD		"ls -t /gl/update/*.tar.gz | head -n 1"  //按时间排序，返回最新的压缩包的名称


//error flag
#define TOUCH_UPDATE_FLAG_FAILED			-1
#define OPEN_SYS_LED_FAILED					-2
#define EXTRACT_UPDATE_PACKAGE_FAILED		-3
#define EXEC_UPDATE_SHELL_FAILED			-4

struct flock* file_lock(short type, short whence);
int get_cur_sw_version_from_package(char *name);
int main()
{
	int res=0;
	char reboot_flag=0;
	char cur_sw_version[1024]={0};
	char *new_cur_sw_version;
    cJSON *root;
    char *json_out;

	cJSON* json_all;
	cJSON* json_tmp;
    FILE* fp;
    int fd;
    int file_size;
    char file_buf[1024];
    char *new_file_buf;

    printf("===============DoUpdate version:%s, COMPILE_TIME[%s: %s]\n", APP_VERSION, __DATE__, __TIME__);

    printf("%s\n", TOUCH_UPDATE_FLAG);
	res = system(TOUCH_UPDATE_FLAG);
	printf("res=%d\n",res);
	if(res !=0) {
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		res = TOUCH_UPDATE_FLAG_FAILED;
		goto over;
	}

	printf("%s\n", OPEN_SYS_LED);
	res = system(OPEN_SYS_LED);
	printf("res=%d\n",res);
	if(res !=0) {
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		res = OPEN_SYS_LED_FAILED;
		goto over; //灯打开失败还是继续执行升级操作？
	}

	printf("%s\n", EXTRACT_UPDATE_PACKAGE);
	res = system(EXTRACT_UPDATE_PACKAGE);
	printf("res=%d\n",res);
	if(res !=0) {
		res = EXTRACT_UPDATE_PACKAGE_FAILED;
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		goto over;
	}

	printf("%s\n", EXEC_UPDATE_SHELL);
	res = system(EXEC_UPDATE_SHELL);
	printf("res=%d\n",res);
	if(res !=0) {
		res = EXEC_UPDATE_SHELL_FAILED;
//		printf("[failed], errno[%d], error msg[%s]\n", errno, strerror(errno));
		goto over;
	}

	//读取是否有need_to_reboot文件
	printf("%s\n", FIND_REBOOT_FLAG_FILE);
	if(system(FIND_REBOOT_FLAG_FILE)==0)
	{
		reboot_flag = 1;
	}
	else
		reboot_flag = 0;
	printf("reboot_flag=%d\n", reboot_flag);

	//截取文件
	///////////////////
	get_cur_sw_version_from_package(cur_sw_version);
	///////////////////
	printf("%s\n", RM_TEMP_FILES);
	system(RM_TEMP_FILES);

over:
	if(res !=0)
	{
//		res = -1;
		reboot_flag = 0;//如果更新失败，reboot_flag置0，不需要重启。
	}
	//更新版本信息
	//////////////////////////
    fp = fopen(FEATURE_GDGL_SW_VERSION_PATH, "a+");
    if (fp == NULL)
    {
        fclose(fp);
    }
    else
    {
        fd = fileno(fp);
        fcntl(fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));
        fseek(fp, 0L, SEEK_END);
        file_size = ftell(fp);
        rewind(fp);
        if (file_size > 0)
        {
             fread(file_buf, sizeof(char), file_size, fp);
             json_all = cJSON_Parse(file_buf);
             if(json_all !=NULL) // 解析成功
             {
            	 if(res !=0)  //如果更新失败，读取本地版本信息
            	 {
                     json_tmp = cJSON_GetObjectItem(json_all, "cur_sw_version");
                     if (json_tmp!=NULL)
                     {
                    	 snprintf(cur_sw_version, sizeof(cur_sw_version), json_tmp->valuestring);
                     }
            	 }
            	 else
            	 {
            		new_cur_sw_version = cur_sw_version;
					cJSON_ReplaceItemInObject(json_all, "cur_sw_version", cJSON_CreateString(new_cur_sw_version));
					new_file_buf = cJSON_Print(json_all);
					new_file_buf[strlen(new_file_buf)] = '\0';
					rewind(fp);
					if(truncate(FEATURE_GDGL_SW_VERSION_PATH, 0)<0) {  //清空文件内容
						printf("truncate failed\n");
					}
					printf("new_sw_version_json=%s\n", new_file_buf);
					fwrite(new_file_buf, sizeof(char), strlen(new_file_buf), fp);
					free(new_file_buf);
            	 }
             }
             fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
             fclose(fp);
             cJSON_Delete(json_all);
        }
    }
    //////////////////////////
    root=cJSON_CreateObject();
	cJSON_AddNumberToObject(root, "msgtype", 0);
	cJSON_AddNumberToObject(root, "mainid", 6);
	cJSON_AddNumberToObject(root, "subid", 2);
	cJSON_AddNumberToObject(root, "status", res);
	cJSON_AddNumberToObject(root, "need_reboot", reboot_flag);
	cJSON_AddStringToObject(root, "cur_sw_version", cur_sw_version);
	json_out=cJSON_Print(root);
	json_out[strlen(json_out)] = '\0';
    push_to_CBDaemon(json_out, strlen(json_out)+1);
	free(json_out);
	cJSON_Delete(root);
	system(CLOSE_SYS_LED);
	printf("%s\n",CLOSE_SYS_LED);

	if(reboot_flag ==1) {
        system("sync");
        printf("sync\n");
		printf("reboot after sleep 3 second\n");
		sleep(3);
		printf("sleep over\n");
		system("/sbin/reboot");
		sleep(3);
	}
	printf("over\n");
	return 0;
}
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

int get_cur_sw_version_from_package(char *name)
{
	printf("%s\n",FIND_PACKAGE_NAME_CMD);
	FILE *stream = popen(FIND_PACKAGE_NAME_CMD,"r");
	char cmd_respond[1024]={0};
	char *ss;
	char *version;
	int recv_len;
	recv_len = fread(cmd_respond,sizeof(char),sizeof(cmd_respond),stream);
	cmd_respond[recv_len] = '\0';
	ss = cmd_respond;
	printf("package name[%s]\n",ss);
	strsep(&ss, "-");
	strsep(&ss, "-");
	version = strsep(&ss, "-");
	memcpy(name, version, strlen(version));
	printf("package version[%s]\n",name);
	return 0;
}

