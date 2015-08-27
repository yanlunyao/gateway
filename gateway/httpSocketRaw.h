/*
 *	File name   : httpSocketRaw.h
 *  Created on  : Apr 22, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#ifndef HTTPSOCKETRAW_H_
#define HTTPSOCKETRAW_H_

pid_t execute_url_action(int table_flag, int id_value);
pid_t execute_ipccapture_url(int table_flag, int id_value, char *time);
int http_get_method_by_socket(const char *urlstring);
#endif /* HTTPSOCKETRAW_H_ */
