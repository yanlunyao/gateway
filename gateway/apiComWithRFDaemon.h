/*
 *	File name   : apiComWithRFDaemon.h
 *  Created on  : Aug 3, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

#ifndef GATEWAY_APICOMWITHRFDAEMON_H_
#define GATEWAY_APICOMWITHRFDAEMON_H_

//#define ENABLE_RF_DEBUG
#ifdef ENABLE_RF_DEBUG
#define RF_DEBUG(fmt, args...)	printf(fmt, ## args)
#else
#define RF_DEBUG(fmt, args...)
#endif

//protocol status
#define ENTRY_PARAMETER_INVALID -1
#define SOCKET_BUILD_FAILED		-2
#define	WRITE_FAILED			-3
#define SELECT_FAILED			WAIT_RECV_TIMEOUT
#define WAIT_RECV_TIMEOUT		-4
#define READ_FAILED				-5
#define DATA_FORMAT_INVALID		-6




int communicateWithRF(const char *send_text, int send_size, char *respond);

#endif /* GATEWAY_APICOMWITHRFDAEMON_H_ */
