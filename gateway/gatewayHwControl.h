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

//gpio47 SET_GPIO  控制按键
#define SETIO_L			"echo 0 > /sys/class/gpio/gpio47/value"
#define SETIO_H			"echo 1 > /sys/class/gpio/gpio47/value"

//cloud led
#define SET_DARK_CLOUD 	"echo 0 > /sys/class/leds/cloud_led/brightness"
#define SET_LIGHT_CLOUD "echo 1 > /sys/class/leds/cloud_led/brightness"

#endif /* GATEWAY_GATEWAYHWCONTROL_H_ */
