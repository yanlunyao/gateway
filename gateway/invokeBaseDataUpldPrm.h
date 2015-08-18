/*
 *	File name   : invokeBaseDataUpldPrm.h
 *  Created on  : May 15, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#ifndef INVOKEBASEDATAUPLDPRM_H_
#define INVOKEBASEDATAUPLDPRM_H_


void invoke_all_pthread();
//void invoke_by_datatype_pthread(int datatype, const char *para);
pid_t invoke_by_datatype_fork(int datatype, const char *para);

#endif /* INVOKEBASEDATAUPLDPRM_H_ */
