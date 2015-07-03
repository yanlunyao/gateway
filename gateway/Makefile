include ../Make.defines

PROGS =	bcast_daemon mcast_daemon init_smartgateway \
		clientLogin.cgi modifyPassword.cgi modifyAlias.cgi \
		cloudProxyHttpModule clientadmin_daemon \
		SmartControl \
		BaseDataUpload CallUploadDataProgram.cgi \
		AddScene.cgi DelScene.cgi EditScene.cgi GetSceneList.cgi DoScene.cgi ModifySceneIndex.cgi  \
		AddTimeAction.cgi EditTimeAction.cgi DelTimeAction.cgi GetTimeActionList.cgi EnableTimeAction.cgi \
		AddLinkage.cgi EditLinkage.cgi DelLinkage.cgi GetLinkageList.cgi EnableLinkage.cgi \
		ui_daemon \
		#siftCallback_daemon \
		net_stat bind_daemon 

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
		${CC} ${CFLAGS} -o $@ net_stat.o 

#bind_daemon
bind_daemon: bind_daemon.o cJSON.o 
		${CC} ${CFLAGS} -o $@ bind_daemon.o cJSON.o ${LIBS} -lm

#ui_daemon
ui_daemon:	ui_daemon.o cJSON.o httpCurlMethod.o
		${CC} ${CFLAGS} -o $@ ui_daemon.o cJSON.o httpCurlMethod.o ${LIBS} -lm -lcurl
		cp ui_daemon /home/mac/nfs_share/yanlyFolder/Ui_daemon/
		
#siftCallback_daemon
#siftCallback_daemon:	siftCallback_daemon.o cJSON.o timedaction.o
#		${CC} ${CFLAGS} -o $@ siftCallback_daemon.o cJSON.o timedaction.o ${LIBS} -lm
		
#BaseDateUpload
BaseDataUpload:	baseDataUpload.o cJSON.o httpCurlMethod.o
		${CC} ${CFLAGS} -o $@ baseDataUpload.o cJSON.o httpCurlMethod.o ${LIBS} -lm -lcurl
#		cp BaseDataUpload /home/mac/nfs_share/yanlyFolder/Upload/
		
#SmartControl
SmartControl:	smartControl.o cJSON.o sqliteWrap.o sqliteOperator.o timingCommon.o httpSocketRaw.o glCalkProtocol.o timedaction.o linkageLoop.o invokeBaseDataUpldPrm.o
		${CC} ${CFLAGS} -o $@ smartControl.o cJSON.o sqliteWrap.o sqliteOperator.o timingCommon.o httpSocketRaw.o glCalkProtocol.o timedaction.o linkageLoop.o invokeBaseDataUpldPrm.o ${LIBS} -lm -lsqlite3
		cp SmartControl /home/mac/nfs_share/yanlyFolder/Upload/
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
DoScene.cgi:	doScene.o cgic.o cJSON.o sqliteOperator.o httpCurlMethod.o glCalkProtocol.o sqliteWrap.o timingCommon.o
		${CC} ${CFLAGS} -o $@ doScene.o cgic.o cJSON.o sqliteOperator.o httpCurlMethod.o  glCalkProtocol.o sqliteWrap.o timingCommon.o ${LIBS} -lm -lsqlite3 -lcurl
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
StopAlarm.cgi:	stopAlarm.o cgic.o cJSON.o httpCurlMethod.o
		${CC} ${CFLAGS} -o $@ stopAlarm.o cgic.o cJSON.o httpCurlMethod.o ${LIBS} -lm -lcurl
		cp StopAlarm.cgi /home/mac/nfs_share/yanlyFolder/Upload/
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
		
cp_application_api:
		cp ${APPLICATION_API} /home/mac/nfs_share/yanlyFolder/application/
cp_user_api:
		cp ${USER_MANAGE_API} /home/mac/nfs_share/yanlyFolder/usermanage/
cp_upload:
		cp BaseDataUpload SmartControl /home/mac/nfs_share/yanlyFolder/Upload/
cp_ui:	
		cp ui_daemon /home/mac/nfs_share/yanlyFolder/Ui_daemon/	
clean:
		rm -f ${PROGS} ${CLEANFILES} *.o
		
		
