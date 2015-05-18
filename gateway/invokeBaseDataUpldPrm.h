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

//datatype
#define GET_ENDPOINT_NUM					1
#define GET_ALLROOMINFO_NUM					2
#define GET_IPCLIST_NUM						3
#define GET_LOCALIASCIE_NUM					4
#define GET_CIELIST_NUM						5
#define GET_ALLBINDLIST_NUM					6
#define GET_IEEEENDPOINT_NUM				7
#define DATATYPE_MAX						GET_IEEEENDPOINT_NUM



void invoke_all_pthread();
//void invoke_by_datatype_pthread(int datatype, const char *para);
pid_t invoke_by_datatype_fork(int datatype, const char *para);

#endif /* INVOKEBASEDATAUPLDPRM_H_ */
