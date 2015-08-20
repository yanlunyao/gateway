/*
 *	File name   : setAllPermitJoinOnByIoControl.c
 *  Created on  : Aug 14, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
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
