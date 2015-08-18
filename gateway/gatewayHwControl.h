/*
 *	File name   : gatewayHwControl.h
 *  Created on  : Aug 14, 2015
 *  Author      : yanly
 *  Description :
 *  Version     :
 *  History     : <author>		<time>		<version>		<desc>
 */
#ifndef GATEWAY_GATEWAYHWCONTROL_H_
#define GATEWAY_GATEWAYHWCONTROL_H_

//gpio47 SET_GPIO
#define SETIO_L			"echo 0 > /sys/class/gpio/gpio47/value"
#define SETIO_H			"echo 1 > /sys/class/gpio/gpio47/value"


#endif /* GATEWAY_GATEWAYHWCONTROL_H_ */
