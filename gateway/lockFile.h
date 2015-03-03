/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  File name: lockFile.h
  Description: lock file maros
  Author: fengqiuchao
  Version: 1.0      
  Date: 2014/03/03
  History:   
    1. Date:
       Author:
       Modification:
    2. ...
***************************************************************************/
#ifndef LOCKFILE_H__
#define LOCKFILE_H__

#include	<sys/types.h>  // pid_t
//record lock
int		lock_reg(int, int, int, off_t, int, off_t);
#define	read_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))
#define	readw_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define	write_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))
#define	writew_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define	un_lock(fd, offset, whence, len) \
	lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))
pid_t	lock_test(int, int, off_t, int, off_t);	
#define	is_read_lockable(fd, offset, whence, len) \
	(lock_test((fd), F_RDLCK, (offset), (whence), (len)) == 0)
#define	is_write_lockable(fd, offset, whence, len) \
	(lock_test((fd), F_WRLCK, (offset), (whence), (len)) == 0)

#endif //LOCKFILE_H__