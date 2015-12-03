#广播模块，旧的广播模块，初始化模块
#登录，修改密码，修改别名
#云代理执行http模块，用户管理模块
#智能控制功能模块, 根据设备ID删除相应的定时、场景、联动规则的cgi；
#基础数据上传模块
#情景相关cgi
#定时相关cgi
#联动相关cgi
#按键和灯的功能模块, 通过io控制的方式允许zigbee设备入网
#获取软件版本，获取硬件版本，执行升级网关的cgi，网关升级程序, 停止所有报警器报警的cgi， 大洋所有报警器操作的cgi
#通知网关房间被删除，获取房间号的设备信息，改变设备的房间号，获取RF设备信息列表，修改RF设备名称，RF设备布撤防，开始/停止RF警号报警，RF设备启用/禁用配置，RF设备全部布防，全部撤防
#获取授权列表信息, 网关设置操作的cgi；
#授权计费功能模块
#允许所有入网，获取绑定列表，
#网络状态检测，绑定模块；
include ../Make.defines
PROGS =	bcast_daemon mcast_daemon init_smartgateway \
		clientLogin.cgi modifyPassword.cgi modifyAlias.cgi \
		cloudProxyHttpModule clientadmin_daemon \
		SmartControl DeleteSmartRuleByDeviceID.cgi \
		BaseDataUpload CallUploadDataProgram.cgi \
		AddScene.cgi DelScene.cgi EditScene.cgi GetSceneList.cgi DoScene.cgi ModifySceneIndex.cgi  \
		AddTimeAction.cgi EditTimeAction.cgi DelTimeAction.cgi GetTimeActionList.cgi EnableTimeAction.cgi \
		AddLinkage.cgi EditLinkage.cgi DelLinkage.cgi GetLinkageList.cgi EnableLinkage.cgi \
		ui_daemon SetAllPermitJoinOnByIoControl.cgi \
		GetSwVersion.cgi GetHwVersion.cgi DoUpdate.cgi DoUpdate StopAlarm.cgi AllIasWarningDeviceOperation.cgi \
		NotifyRoomIdHasBeenDeleted.cgi GetRFDevByRoomId.cgi ModifyRFDevRoomId.cgi GetRFDevList.cgi ChangeRFDevName.cgi ChangeRFDevArmState.cgi RFWarningDevOperation.cgi ChangeRFDevActivationState.cgi ChangeAllRFDevArmState.cgi \
		GetAuthState.cgi GatewaySettingOperation.cgi \
		Auth_Daemon \
#		SetAllPermitJoinOn.cgi GetAllBindList.cgi \
#		net_stat bind_daemon

USER_MANAGE_API = clientLogin.cgi modifyPassword.cgi modifyAlias.cgi 		
TIME_API = AddTimeAction.cgi EditTimeAction.cgi DelTimeAction.cgi GetTimeActionList.cgi EnableTimeAction.cgi
SCENE_API = AddScene.cgi DelScene.cgi EditScene.cgi GetSceneList.cgi DoScene.cgi ModifySceneIndex.cgi
LINKAGE_API = AddLinkage.cgi EditLinkage.cgi DelLinkage.cgi GetLinkageList.cgi EnableLinkage.cgi
APPLICATION_API = ${TIME_API} ${SCENE_API} ${LINKAGE_API}

all:	${PROGS}
usermanage:	${USER_MANAGE_API}
time:	${TIME_API}
scene:	${SCENE_API}
linkage:	${LINKAGE_API}
		cp ${LINKAGE_API} /home/mac/nfs_share/yanlyFolder/Linkage/
appapi:	${APPLICATION_API}


# Broadcast
bcast_daemon:	bcast_daemon.o cJSON.o lockreg.o 
		${CC} ${CFLAGS} -o $@ bcast_daemon.o cJSON.o lockreg.o ${LIBS} -lm

# Multicast
mcast_daemon:	mcast_daemon.o
		${CC} ${CFLAGS} -o $@ mcast_daemon.o ${LIBS}

# Init
init_smartgateway:	init_smartgateway.o lockreg.o
		${CC} ${CFLAGS} -o $@ init_smartgateway.o lockreg.o ${LIBS}

# Client login
clientLogin.cgi:	clientLogin.o clientAdmin.o cgic.o cJSON.o lockreg.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ clientLogin.o clientAdmin.o cgic.o cJSON.o lockreg.o glCalkProtocol.o ${LIBS} -lm

# Modify password
modifyPassword.cgi:	modifyPassword.o clientAdmin.o cgic.o cJSON.o lockreg.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ modifyPassword.o clientAdmin.o cgic.o cJSON.o lockreg.o glCalkProtocol.o ${LIBS} -lm

# Modify alias
modifyAlias.cgi:	modifyAlias.o clientAdmin.o cgic.o cJSON.o lockreg.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ modifyAlias.o clientAdmin.o cgic.o cJSON.o lockreg.o glCalkProtocol.o ${LIBS} -lm

#cloudProxyHttpModule
cloudProxyHttpModule:	cloudProxyHttpModule.o cJSON.o
		${CC} ${CFLAGS} -o $@ cloudProxyHttpModule.o cJSON.o ${LIBS} -lm -lcurl
		cp cloudProxyHttpModule /home/mac/nfs_share/yanlyFolder/special/
		
#clientadmin_daemon
clientadmin_daemon:	clientadmin_daemon.o cJSON.o lockreg.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ clientadmin_daemon.o cJSON.o lockreg.o glCalkProtocol.o ${LIBS} -lm

#net_stat
net_stat: net_stat.o
		${CC} -o $@ net_stat.o 

#bind_daemon
bind_daemon: bind_daemon.o cJSON.o 
		${CC} -o $@ bind_daemon.o cJSON.o -lpthread -lm

#ui_daemon
ui_daemon:	ui_daemon.o cJSON.o httpCurlMethod.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ ui_daemon.o cJSON.o httpCurlMethod.o glCalkProtocol.o ${LIBS} -lm -lcurl
#		cp ui_daemon /home/mac/nfs_share/yanlyFolder/Ui_daemon/
		
#siftCallback_daemon
#siftCallback_daemon:	siftCallback_daemon.o cJSON.o timedaction.o
#		${CC} ${CFLAGS} -o $@ siftCallback_daemon.o cJSON.o timedaction.o ${LIBS} -lm
		
#BaseDateUpload
BaseDataUpload:	baseDataUpload.o cJSON.o httpCurlMethod.o
		${CC} ${CFLAGS} -o $@ baseDataUpload.o cJSON.o httpCurlMethod.o ${LIBS} -lm -lcurl
#		cp BaseDataUpload /home/mac/nfs_share/yanlyFolder/Upload/
		
#SmartControl
SmartControl:	smartControl.o cJSON.o sqliteWrap.o sqliteOperator.o timingCommon.o httpSocketRaw.o glCalkProtocol.o timedaction.o linkageLoop.o invokeBaseDataUpldPrm.o httpCurlMethod.o httpGetMsgSmart.o
		${CC} ${CFLAGS} -o $@ smartControl.o cJSON.o sqliteWrap.o sqliteOperator.o timingCommon.o httpSocketRaw.o glCalkProtocol.o timedaction.o linkageLoop.o invokeBaseDataUpldPrm.o httpCurlMethod.o httpGetMsgSmart.o ${LIBS} -lm -lcurl -lsqlite3
#		cp SmartControl /home/mac/nfs_share/yanlyFolder/Upload/
#DoUpdate	
DoUpdate:	doUpdateRoutine.o cJSON.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ doUpdateRoutine.o cJSON.o glCalkProtocol.o ${LIBS} -lm
#		cp DoUpdate /home/mac/nfs_share/yanlyFolder/remoteupdate/
#Auth_Daemon
Auth_Daemon:	auth_daemon.o cJSON.o timedaction.o glCalkProtocol.o auth_push2cb.o auth_callback.o
		${CC} ${CFLAGS} -o $@ auth_daemon.o cJSON.o timedaction.o glCalkProtocol.o auth_push2cb.o auth_callback.o ${LIBS} -lm
#		cp Auth_Daemon /home/mac/nfs_share/yanlyFolder/rf/
#####################################################################################   //ma gong cgi
SetAllPermitJoinOn.cgi:	SetAllPermitJoinOn.o cJSON.o
		${CC} -I../lib -g -D_REENTRANT -Wall -o $@ SetAllPermitJoinOn.o cJSON.o ${LIBS} -lm
GetAllBindList.cgi:	GetAllBindList.o cJSON.o
		${CC} -I../lib -g -D_REENTRANT -Wall -o $@ GetAllBindList.o cJSON.o ${LIBS} -lm
#####################################################################################   //scene cgi
#AddScene
AddScene.cgi:	addScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o generUrl.o sqliteWrap.o timingCommon.o
		${CC} ${CFLAGS} -o $@ addScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o generUrl.o sqliteWrap.o timingCommon.o ${LIBS} -lm -lsqlite3
#EditScene
EditScene.cgi:	editScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o generUrl.o sqliteWrap.o timingCommon.o
		${CC} ${CFLAGS} -o $@ editScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o generUrl.o sqliteWrap.o timingCommon.o ${LIBS} -lm -lsqlite3
#DelScene
DelScene.cgi:	delScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o sqliteWrap.o timingCommon.o
		${CC} ${CFLAGS} -o $@ delScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o sqliteWrap.o timingCommon.o ${LIBS} -lm -lsqlite3
#GetSceneList.cgi
GetSceneList.cgi:	getSceneList.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o sqliteWrap.o timingCommon.o
		${CC} ${CFLAGS} -o $@ getSceneList.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o sqliteWrap.o timingCommon.o ${LIBS} -lm -lsqlite3
#DoScene.cgi
DoScene.cgi:	doScene.o cgic.o cJSON.o sqliteOperator.o httpCurlMethod.o glCalkProtocol.o sqliteWrap.o timingCommon.o apiComWithRFDaemon.o
		${CC} ${CFLAGS} -o $@ doScene.o cgic.o cJSON.o sqliteOperator.o httpCurlMethod.o  glCalkProtocol.o sqliteWrap.o timingCommon.o apiComWithRFDaemon.o ${LIBS} -lm -lsqlite3 -lcurl
#ModifySceneIndex.cgi
ModifySceneIndex.cgi:	modifySceneIndex.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o sqliteWrap.o timingCommon.o
		${CC} ${CFLAGS} -o $@ modifySceneIndex.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o sqliteWrap.o timingCommon.o ${LIBS} -lm -lsqlite3

##############################################################################################   //timeaction cgi
#AddTimeAction
AddTimeAction.cgi:	addTimeAction.o cgic.o cJSON.o glCalkProtocol.o generUrl.o sqliteWrap.o sqliteOperator.o timingCommon.o
		${CC} ${CFLAGS} -o $@ addTimeAction.o cgic.o cJSON.o glCalkProtocol.o generUrl.o sqliteWrap.o sqliteOperator.o timingCommon.o ${LIBS} -lm -lsqlite3
#EditTimeAction
EditTimeAction.cgi:	editTimeAction.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o generUrl.o sqliteWrap.o timingCommon.o
		${CC} ${CFLAGS} -o $@ editTimeAction.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o generUrl.o sqliteWrap.o timingCommon.o ${LIBS} -lm -lsqlite3
#DelTimeAction
DelTimeAction.cgi:	delTimeAction.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o sqliteWrap.o timingCommon.o
		${CC} ${CFLAGS} -o $@ delTimeAction.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o sqliteWrap.o timingCommon.o ${LIBS} -lm -lsqlite3
#GetTimeActionList
GetTimeActionList.cgi:	getTimeActionList.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o sqliteWrap.o timingCommon.o
		${CC} ${CFLAGS} -o $@ getTimeActionList.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o  sqliteWrap.o timingCommon.o ${LIBS} -lm -lsqlite3
#EnableTimeAction.cgi
EnableTimeAction.cgi:	enableTimeAction.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o sqliteWrap.o timingCommon.o
		${CC} ${CFLAGS} -o $@ enableTimeAction.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o sqliteWrap.o timingCommon.o ${LIBS} -lm -lsqlite3

##############################################################################################   //linkage cgi
#AddLinkage.cgi
AddLinkage.cgi:	addLinkage.o cgic.o cJSON.o glCalkProtocol.o generUrl.o sqliteWrap.o sqliteOperator.o timingCommon.o
		${CC} ${CFLAGS} -o $@ addLinkage.o cgic.o cJSON.o glCalkProtocol.o generUrl.o sqliteWrap.o sqliteOperator.o timingCommon.o ${LIBS} -lm -lsqlite3
#EditLinkage.cgi
EditLinkage.cgi: editLinkage.o cgic.o cJSON.o glCalkProtocol.o generUrl.o sqliteWrap.o sqliteOperator.o timingCommon.o
		${CC} ${CFLAGS} -o $@ editLinkage.o cgic.o cJSON.o glCalkProtocol.o generUrl.o sqliteWrap.o sqliteOperator.o timingCommon.o ${LIBS} -lm -lsqlite3
#DelLinkage.cgi
DelLinkage.cgi:	delLinkage.o cgic.o cJSON.o glCalkProtocol.o generUrl.o sqliteWrap.o sqliteOperator.o timingCommon.o
		${CC} ${CFLAGS} -o $@ delLinkage.o cgic.o cJSON.o glCalkProtocol.o generUrl.o sqliteWrap.o sqliteOperator.o timingCommon.o ${LIBS} -lm -lsqlite3
#GetLinkageList.cgi
GetLinkageList.cgi:	getLinkageList.o cgic.o cJSON.o glCalkProtocol.o sqliteWrap.o sqliteOperator.o timingCommon.o
		${CC} ${CFLAGS} -o $@ getLinkageList.o cgic.o cJSON.o glCalkProtocol.o sqliteWrap.o sqliteOperator.o timingCommon.o ${LIBS} -lm -lsqlite3

#EnableLinkage.cgi
EnableLinkage.cgi:	enableLinkage.o cgic.o cJSON.o glCalkProtocol.o sqliteWrap.o sqliteOperator.o timingCommon.o
		${CC} ${CFLAGS} -o $@ enableLinkage.o cgic.o cJSON.o glCalkProtocol.o sqliteWrap.o sqliteOperator.o timingCommon.o ${LIBS} -lm -lsqlite3

##############################################################################################   //invoke api upload daemon cgi
CallUploadDataProgram.cgi:	callUploadDataProgram.o cgic.o cJSON.o 
		${CC} ${CFLAGS} -o $@ callUploadDataProgram.o cgic.o cJSON.o ${LIBS} -lm
#		cp CallUploadDataProgram.cgi /home/mac/nfs_share/yanlyFolder/Upload/
StopAlarm.cgi:	stopAlarm.o cgic.o cJSON.o httpCurlMethod.o apiComWithRFDaemon.o
		${CC} ${CFLAGS} -o $@ stopAlarm.o cgic.o cJSON.o httpCurlMethod.o apiComWithRFDaemon.o ${LIBS} -lm -lcurl
#		cp StopAlarm.cgi /home/mac/nfs_share/yanlyFolder/Upload/
		
GetSwVersion.cgi:	getSwVersion.o cgic.o cJSON.o
		${CC} ${CFLAGS}	-o $@ getSwVersion.o cgic.o cJSON.o ${LIBS} -lm 
#		cp GetSwVersion.cgi /home/mac/nfs_share/yanlyFolder/remoteupdate/		
GetHwVersion.cgi:	getHwVersion.o cgic.o cJSON.o
		${CC} ${CFLAGS} -o $@ getHwVersion.o cgic.o cJSON.o ${LIBS} -lm 
#		cp GetHwVersion.cgi /home/mac/nfs_share/yanlyFolder/remoteupdate/		
DoUpdate.cgi:	doUpdate.o cgic.o cJSON.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ doUpdate.o cgic.o cJSON.o glCalkProtocol.o ${LIBS} -lm 
#		cp DoUpdate.cgi /home/mac/nfs_share/yanlyFolder/remoteupdate/	
AllIasWarningDeviceOperation.cgi:	AllIasWarningDeviceOperation.o cgic.o cJSON.o httpCurlMethod.o
		${CC} ${CFLAGS} -o $@ AllIasWarningDeviceOperation.o cgic.o cJSON.o httpCurlMethod.o ${LIBS} -lm -lcurl
#		cp AllIasWarningDeviceOperation.cgi /home/mac/nfs_share/yanlyFolder/Upload/	
##############################################################################################   //RF relevant
GetRFDevList.cgi:	getRFDevList.o cgic.o cJSON.o
		${CC} ${CFLAGS}	-o $@ getRFDevList.o cgic.o cJSON.o ${LIBS} -lm 
#		cp GetRFDevList.cgi /home/mac/nfs_share/yanlyFolder/rf/
ChangeRFDevName.cgi:	changeRFDevName.o cgic.o cJSON.o apiComWithRFDaemon.o
		${CC} ${CFLAGS}	-o $@ changeRFDevName.o cgic.o cJSON.o apiComWithRFDaemon.o ${LIBS} -lm 
#		cp ChangeRFDevName.cgi /home/mac/nfs_share/yanlyFolder/rf/
ChangeRFDevArmState.cgi:	changeRFDevArmState.o cgic.o cJSON.o apiComWithRFDaemon.o
		${CC} ${CFLAGS}	-o $@ changeRFDevArmState.o cgic.o cJSON.o apiComWithRFDaemon.o ${LIBS} -lm 
#		cp ChangeRFDevArmState.cgi /home/mac/nfs_share/yanlyFolder/rf/
RFWarningDevOperation.cgi:	RFWarningDevOperation.o cgic.o cJSON.o apiComWithRFDaemon.o
		${CC} ${CFLAGS}	-o $@ RFWarningDevOperation.o cgic.o cJSON.o apiComWithRFDaemon.o ${LIBS} -lm 
#		cp RFWarningDevOperation.cgi /home/mac/nfs_share/yanlyFolder/rf/
ChangeRFDevActivationState.cgi:	changeRFDevActivationState.o cgic.o cJSON.o apiComWithRFDaemon.o
		${CC} ${CFLAGS}	-o $@ changeRFDevActivationState.o cgic.o cJSON.o apiComWithRFDaemon.o ${LIBS} -lm 
#		cp ChangeRFDevActivationState.cgi /home/mac/nfs_share/yanlyFolder/rf/
ChangeAllRFDevArmState.cgi:	changeAllRFDevArmState.o cgic.o cJSON.o apiComWithRFDaemon.o
		${CC} ${CFLAGS}	-o $@ changeAllRFDevArmState.o cgic.o cJSON.o apiComWithRFDaemon.o ${LIBS} -lm 
#		cp ChangeAllRFDevArmState.cgi /home/mac/nfs_share/yanlyFolder/rf/
ModifyRFDevRoomId.cgi:	modifyRFDevRoomId.o cgic.o cJSON.o apiComWithRFDaemon.o
		${CC} ${CFLAGS}	-o $@ modifyRFDevRoomId.o cgic.o cJSON.o apiComWithRFDaemon.o ${LIBS} -lm 
		cp ModifyRFDevRoomId.cgi /home/mac/nfs_share/yanlyFolder/rf/
GetRFDevByRoomId.cgi:	getRFDevByRoomId.o cgic.o cJSON.o apiComWithRFDaemon.o
		${CC} ${CFLAGS}	-o $@ getRFDevByRoomId.o cgic.o cJSON.o apiComWithRFDaemon.o ${LIBS} -lm 
		cp GetRFDevByRoomId.cgi /home/mac/nfs_share/yanlyFolder/rf/
NotifyRoomIdHasBeenDeleted.cgi:	notifyRoomIdHasBeenDeleted.o cgic.o cJSON.o apiComWithRFDaemon.o
		${CC} ${CFLAGS}	-o $@ notifyRoomIdHasBeenDeleted.o cgic.o cJSON.o apiComWithRFDaemon.o ${LIBS} -lm 
		cp NotifyRoomIdHasBeenDeleted.cgi /home/mac/nfs_share/yanlyFolder/rf/
##############################################################################################   //business relevant
GetAuthState.cgi:	getAuthState.o cgic.o cJSON.o
		${CC} ${CFLAGS}	-o $@ getAuthState.o cgic.o cJSON.o ${LIBS} -lm 
#		cp GetAuthState.cgi /home/mac/nfs_share/yanlyFolder/rf/
SetAllPermitJoinOnByIoControl.cgi:	setAllPermitJoinOnByIoControl.o cgic.o
		${CC} ${CFLAGS}	-o $@ setAllPermitJoinOnByIoControl.o cgic.o ${LIBS}
#		cp SetAllPermitJoinOnByIoControl.cgi /home/mac/nfs_share/yanlyFolder/rf/
GatewaySettingOperation.cgi:	gatewaySettingOperation.o cgic.o cJSON.o glCalkProtocol.o myfile_operation.o httpCurlMethod.o
		${CC} ${CFLAGS}	-o $@ gatewaySettingOperation.o cgic.o cJSON.o glCalkProtocol.o myfile_operation.o httpCurlMethod.o ${LIBS} -lm -lcurl
#		cp GatewaySettingOperation.cgi /home/mac/nfs_share/yanlyFolder/rf/		
DeleteSmartRuleByDeviceID.cgi:	deleteSmartRuleByDeviceID.o cgic.o cJSON.o glCalkProtocol.o sqliteWrap.o sqliteOperator.o timingCommon.o
		${CC} ${CFLAGS} -o $@ deleteSmartRuleByDeviceID.o cgic.o cJSON.o glCalkProtocol.o sqliteWrap.o sqliteOperator.o timingCommon.o ${LIBS} -lm -lsqlite3
#		cp DeleteSmartRuleByDeviceID.cgi /home/mac/nfs_share/yanlyFolder/rf/
loopcgi:	loopcgi.o cgic.o
		${CC} ${CFLAGS} -o $@ loopcgi.o cgic.o ${LIBS}
#rf_recv_test:	rf_recv_test.o
#		${CC} ${CFLAGS} -o $@ rf_recv_test.o ${LIBS}
#		cp rf_recv_test /home/mac/nfs_share/yanlyFolder/rf/
#		cp rf_recv_test /media/sf_Share/bin/
########################################test################################################
#timerActionLoop:	timerActionLoop.o
#		${CC} ${CFLAGS} -o $@ timerActionLoop.o

#strsep_test
#strsep_test: strsep_test.o
#		${CC} ${CFLAGS} -o $@ strsep_test.o
		
#clean_strsep: 
#		rm -f strsep_test strsep_test.o

#libevent_test
#libevent_test: libevent_test.o
#		${CC} ${CFLAGS} -o $@ libevent_test.o -levent
ALL_DAEMON = Auth_Daemon DoUpdate SmartControl BaseDataUpload factory_reset.sh ui_daemon net_stat bind_daemon cloudProxyHttpModule clientadmin_daemon init_smartgateway bcast_daemon
DATE =`date '+%Y%m%d'`

#echo "${DATE]"
cp_application_api:
		cp *.cgi /home/mac/nfs_share/yanlyFolder/API/
		tar -czvf API_yanly${DATE}.tar.gz *.cgi
		cp API_yanly${DATE}.tar.gz /media/sf_Share/bin/
		cp *.cgi ../gateway/bin/api_files/
		mv API_yanly${DATE}.tar.gz ../gateway/bin/api_files/
cp_all_daemon:
		cp ${ALL_DAEMON} /home/mac/nfs_share/yanlyFolder/all_daemon/
		tar -czvf Daemon_yanly${DATE}.tar.gz ${ALL_DAEMON}
		cp Daemon_yanly${DATE}.tar.gz /media/sf_Share/bin/
		cp ${ALL_DAEMON} ../gateway/bin/
		mv Daemon_yanly${DATE}.tar.gz ../gateway/bin/
tar_all_elf:
		chmod 755 *
		cp ${ALL_DAEMON}  ../gateway/bin/
		cp *.cgi ../gateway/bin/api_files/
		tar -czvf API_yanly${DATE}.tar.gz *.cgi
		tar -czvf Daemon_yanly${DATE}.tar.gz ${ALL_DAEMON}
		tar -czvf ELF_yanly${DATE}.tar.gz API_yanly${DATE}.tar.gz Daemon_yanly${DATE}.tar.gz
		cp ELF_yanly${DATE}.tar.gz ../gateway/bin/
		mv ELF_yanly${DATE}.tar.gz /media/sf_Share/bin/
tar_source_code:
		tar -czvf unpv13e${DATE}.tar.gz ../../unpv13e/
		mv unpv13e${DATE}.tar.gz /media/sf_Share/bin/code/
cp_upload:
		cp BaseDataUpload SmartControl /home/mac/nfs_share/yanlyFolder/Upload/
clean:
		rm -f ${PROGS} ${CLEANFILES} *.o
