include ../Make.defines

PROGS =	bcast_daemon mcast_daemon init_smartgateway \
		clientLogin.cgi modifyPassword.cgi modifyAlias.cgi \
		cloudProxyHttpModule clientadmin_daemon \
		siftCallback_daemon \
		net_stat bind_daemon \
		AddScene.cgi DelScene.cgi EditScene.cgi GetSceneList.cgi DoScene.cgi ModifySceneIndex.cgi 

all:	${PROGS}

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

#clientadmin_daemon
clientadmin_daemon:	clientadmin_daemon.o cJSON.o lockreg.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ clientadmin_daemon.o cJSON.o lockreg.o glCalkProtocol.o ${LIBS} -lm

#net_stat
net_stat: net_stat.o
		${CC} ${CFLAGS} -o $@ net_stat.o 

#bind_daemon
bind_daemon: bind_daemon.o cJSON.o 
		${CC} ${CFLAGS} -o $@ bind_daemon.o cJSON.o ${LIBS} -lm

#siftCallback_daemon
siftCallback_daemon:	siftCallback_daemon.o cJSON.o timedaction.o
		${CC} ${CFLAGS} -o $@ siftCallback_daemon.o cJSON.o timedaction.o ${LIBS} -lm


#AddScene
AddScene.cgi:	addScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o generUrl.o
		${CC} ${CFLAGS} -o $@ addScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o generUrl.o ${LIBS} -lm -lsqlite3
#EditScene
EditScene.cgi:	editScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o generUrl.o
		${CC} ${CFLAGS} -o $@ editScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o generUrl.o ${LIBS} -lm -lsqlite3
#EditScene.cgi:	editScene.o cgic.o cJSON.o sqliteOperator.o generUrl.o  
#		${CC} ${CFLAGS} -o $@ editScene.o cgic.o cJSON.o sqliteOperator.o generUrl.o ${LIBS} -lm -lsqlite3

#DelScene
DelScene.cgi:	delScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ delScene.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o ${LIBS} -lm -lsqlite3
#DelScene.cgi:	delScene.o cgic.o cJSON.o sqliteOperator.o 
#		${CC} ${CFLAGS} -o $@ delScene.o cgic.o cJSON.o sqliteOperator.o ${LIBS} -lm -lsqlite3		

GetSceneList.cgi:	getSceneList.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ getSceneList.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o ${LIBS} -lm -lsqlite3
#GetSceneList.cgi:	getSceneList.o cgic.o cJSON.o sqliteOperator.o 
#		${CC} ${CFLAGS} -o $@ getSceneList.o cgic.o cJSON.o sqliteOperator.o ${LIBS} -lm -lsqlite3		

DoScene.cgi:	doScene.o cgic.o cJSON.o sqliteOperator.o httpCurlMethod.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ doScene.o cgic.o cJSON.o sqliteOperator.o httpCurlMethod.o  glCalkProtocol.o ${LIBS} -lm -lsqlite3 -lcurl

ModifySceneIndex.cgi:	modifySceneIndex.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o
		${CC} ${CFLAGS} -o $@ modifySceneIndex.o cgic.o cJSON.o sqliteOperator.o glCalkProtocol.o ${LIBS} -lm -lsqlite3


#timerActionLoop:	timerActionLoop.o
#		${CC} ${CFLAGS} -o $@ timerActionLoop.o
########################################test################################################
#strsep_test
strsep_test: strsep_test.o
		${CC} ${CFLAGS} -o $@ strsep_test.o
		
clean_strsep: 
		rm -f strsep_test strsep_test.o

#libevent_test
libevent_test: libevent_test.o
		${CC} ${CFLAGS} -o $@ libevent_test.o -levent
		
mv_obj:
		mv ${PROGS} /home/mac/nfs_share/yanlyFolder/

clean:
		rm -f ${PROGS} ${CLEANFILES} *.o
		
		
