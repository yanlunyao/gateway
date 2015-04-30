/*
 *	File name   : timing_common.c
 *  Created on  : Apr 20, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "smartgateway.h"
#include "timingCommon.h"

/*
 * function: gl_time_format_convert_to_system_tm()
 * description:
 * input:	gl time format example: 20150420123030
 * output:
 * return:	struct tm
 * */
void gl_time_format_convert_to_system_tm(const char *gl_time_format, struct tm* store_time_tm)
{
	char year[5];//+''
	char month[3];
	char day[3];
	char hour[3];
	char min[3];
	char sec[3];

	strncpy(year, gl_time_format, 4);
	strncpy(month, gl_time_format+4, 2);
	strncpy(day, gl_time_format+6, 2);
	strncpy(hour, gl_time_format+8, 2);
	strncpy(min, gl_time_format+10, 2);
	strncpy(sec, gl_time_format+12, 2);

	year[4] = '\0';
	month[2] = '\0';
	day[2] = '\0';
	hour[2] = '\0';
	min[2] = '\0';
	sec[2] = '\0';

	store_time_tm->tm_year = atoi(year)-1900;
	store_time_tm->tm_mon = atoi(month)-1;
	store_time_tm->tm_mday = atoi(day);
	store_time_tm->tm_hour = atoi(hour);
	store_time_tm->tm_min = atoi(min);
	store_time_tm->tm_sec = atoi(sec);

	return ;
}
void system_tm_convert_to_gl_time_format(const struct tm* time_tm, char *store_gl_time)
{
	snprintf(store_gl_time, OUT_TIME_FORMAT_LEN+1, "%04d%02d%02d%02d%02d%02d", time_tm->tm_year+1900,time_tm->tm_mon+1,
			time_tm->tm_mday,time_tm->tm_hour,time_tm->tm_min,time_tm->tm_sec);
	return;
}
//延时模式的执行时间计算: gl_time will change
void delay_enrolltime_to_excutetime(char *gl_time, char *delay_sec)
{
	struct tm temp_tm;
	struct tm exetime_tm;
	time_t temp_stamp;

	gl_time_format_convert_to_system_tm(gl_time, &temp_tm);//gl time format convert to tm

	temp_stamp = mktime(&temp_tm);							//tm convert to timestamp
	temp_stamp = temp_stamp + atoi(delay_sec);				//timestamp+second of delay time

	localtime_r(&temp_stamp, &exetime_tm);					//timestamp convert to tm

	system_tm_convert_to_gl_time_format(&exetime_tm, gl_time);//tm convert to gl time format

}
//获取当前时间转换成gl的时间格式
void get_current_time_gl_format(char *current_gl_time)
{
	time_t calendar;
	struct tm time_tm;

	calendar = time(NULL);
	localtime_r(&calendar, &time_tm);
	system_tm_convert_to_gl_time_format(&time_tm, current_gl_time);
}
int compare_time_tm(const struct tm *first, const struct tm *second)
{
	return 0;
}

////test
//int  main(){
//	char temp[14];
//	char old_time[14] = "20141212123030";
//	struct tm old_tm;
//
//	time_t calendar=0;
//	struct tm *time_data;
//	calendar = time(NULL);  //get current calendar time
//	time_data = localtime(&calendar);
//	system_tm_convert_to_gl_time_format(time_data, temp);
//	printf("current time=%s\n", temp);
//
//	gl_time_format_convert_to_system_tm(old_time, &old_tm);
//	system_tm_convert_to_gl_time_format(&old_tm, temp);
//	printf("old time=%s\n", temp);
//
//	return 0;
//}
//int  main()
//{
//	char temp[14];
//	struct tm current_tm;
//
//	time_t calendar=0;
//	while(1){
//	calendar = time(NULL);  //get current calendar time
//	localtime_r(&calendar, &current_tm);
//	system_tm_convert_to_gl_time_format(&current_tm, temp);
//	printf("current time=%s\n", temp);
//	sleep(1);
//	}
//
//	return 0;
//}


