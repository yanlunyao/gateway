/*
 *	File name   : timingCommon.h
 *  Created on  : Apr 20, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#ifndef TIMINGCOMMON_H_
#define TIMINGCOMMON_H_

void gl_time_format_convert_to_system_tm(const char *gl_time_format, struct tm* store_time_tm);
void system_tm_convert_to_gl_time_format(const struct tm* time_tm, char *store_gl_time);


#endif /* TIMINGCOMMON_H_ */
