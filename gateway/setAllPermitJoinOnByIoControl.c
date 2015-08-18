/*
 *	File name   : setAllPermitJoinOnByIoControl.c
 *  Created on  : Aug 14, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */

/*
 *	File name   : stopAlarm.c
 *  Created on  : Jul 1, 2015
 *  Author      : yanly
 *  Description : 停止大洋zigbee报警器报警和网关报警：获取大洋ep信息，得到报警器ieee发送停止报警api；调用killall mplayer停止网关报警；
 *  Version     : V01-00
 *  History     : <author>		<time>		<version>		<desc>
 *  				yanly		150806		V01-01		    增加停止所有RF警号报警
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cgic.h"
#include "gatewayHwControl.h"

int cgiMain()
{
	cgiHeaderContentType("application/json"); //MIME

	//用按键控制gpio来允许入网操作：
	system(SETIO_L);
	sleep(1);
	system(SETIO_H);

	fprintf(cgiOut,"%s\n", "{\n	\"status\":	0\n}");

	return 0;
}
