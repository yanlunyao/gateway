include ../Make.defines

PROGS =	bcast_daemon mcast_daemon init_smartgateway \
		clientLogin.cgi modifyPassword.cgi modifyAlias.cgi \
		cloudProxyHttpModule clientadmin_daemon

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
clientLogin.cgi:	clientLogin.o clientAdmin.o cgic.o cJSON.o lockreg.o
		${CC} ${CFLAGS} -o $@ clientLogin.o clientAdmin.o cgic.o cJSON.o lockreg.o ${LIBS} -lm

# Modify password
modifyPassword.cgi:	modifyPassword.o clientAdmin.o cgic.o cJSON.o lockreg.o
		${CC} ${CFLAGS} -o $@ modifyPassword.o clientAdmin.o cgic.o cJSON.o lockreg.o ${LIBS} -lm

# Modify alias
modifyAlias.cgi:	modifyAlias.o clientAdmin.o cgic.o cJSON.o lockreg.o
		${CC} ${CFLAGS} -o $@ modifyAlias.o clientAdmin.o cgic.o cJSON.o lockreg.o ${LIBS} -lm

#cloudProxyHttpModule
cloudProxyHttpModule:	cloudProxyHttpModule.o cJSON.o
		${CC} ${CFLAGS} -o $@ cloudProxyHttpModule.o cJSON.o ${LIBS} -lm -lcurl

#clientadmin_daemon
clientadmin_daemon:	clientadmin_daemon.o cJSON.o lockreg.o
		${CC} ${CFLAGS} -o $@ clientadmin_daemon.o cJSON.o lockreg.o ${LIBS} -lm

clean:
		rm -f ${PROGS} ${CLEANFILES}
