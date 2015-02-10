/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName: lockreg.c
  Author: fengqiuchao      
  Version : V1.0         
  Date: 2014/02/27
  Description: request and release a record lock
  History:
      <author>       <time>     <version >   <desc>
      fengqiuchao    2014/02/27     1.0     build this moudle  
***************************************************************************/

//#include "unp.h"
#include <fcntl.h>
#include "lockFile.h"


int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
	struct flock	lock;

	lock.l_type = type;		/* F_RDLCK, F_WRLCK, F_UNLCK */
	lock.l_start = offset;	/* byte offset, relative to l_whence */
	lock.l_whence = whence;	/* SEEK_SET, SEEK_CUR, SEEK_END */
	lock.l_len = len;		/* #bytes (0 means to EOF) */

	return(fcntl(fd, cmd, &lock));
}
