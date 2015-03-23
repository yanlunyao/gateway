/***************************************************************************
  Copyright (C), 2009-2014 GuangdongGuanglian Electronic Technology Co.,Ltd.
  FileName:      bind_deamon.c
  Author:        马晓槟
  Version :      1.3
  Date:          2015-02-13
  Description:   监听5018端口，当收到msgtype=28（绑定相关）的callback消息类型时，根据需要更新
                 bindlist.json（对应json_all），同时把该设备的IEEE和第一个EP（保存这些信息用于
                 调用getBindList.cgi时使用，对于有多个EP的设备，只要调用其中一个EP，就能触发
                 HA_Daemon返回该设备所有EP的msgtype=28的callback消息）保存到binddev.json
                 （对应json_dev）中；
                 当收到msgtype=41（annouce）的callback消息类型时，如果对应设备的IEEE存在于
                 binddev.json中（说明该设备之前进行过绑定、解绑操作），则调用getBindList.cgi
                 （需要IEEE和EP）触发msgtype=28的callback消息；
                 当收到msgtype=14,15（且status=0）的callback消息类型时，如果对应设备的EP信息
                 已存在于binddev.json中则直接调用getBindList.cgi触发callback msgtype=28；如果
                 对应设备的EP信息在binddev.json中不存在，则开启新的线程读取对应设备的EP列表并
                 更新到binddev.json，然后再调用getBindList.cgi触发callback msgtype=28。
  CompileCmd:    arm-linux-gcc cJSON/cJSON.c bind_daemon.c -lm -lpthread -o bind_daemon
  History:        <author>   <time>    <version>    <desc>
        马晓槟   2014-10-09     1.1       修改端口为5018；增加错误重连的逻辑。
        马晓槟   2015-02-06     1.2       当曾经进行绑定、解绑操作的设备被激活时，调用getBindList.cgi
                                          触发HA_Daemon返回该设备所有EP的msgtype=28的callback消息。
        马晓槟   2015-02-13     1.3       添加对callback msgtype=14,15 的响应，使手动绑定也能触发绑定
                                          信息更新。
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include "cJSON.h"

#ifdef _DEBUG
#define DEST_IP         "192.168.1.237"
#else
#define DEST_IP         "127.0.0.1"
#endif

#define DEST_PORT       5018

#define BUF_SIZE        2048
#define REQ_SIZE        512
#define IEEE_LEN        16

#define HEART_BEAT      "0200070007"
#define HB_INTERVAL     30
#define RETRY_INTERVAL  3

#define BIND_LIST_FILE  "/gl/etc/bindlist.json"
#define BIND_DEV_FILE   "/gl/etc/binddev.json"

#define MSGTYPE_BIND        14
#define MSGTYPE_UNBIND      15
#define MSGTYPE_BIND_DATA   28
#define MSGTYPE_JOIN        29
#define MSGTYPE_DELDEV      34
#define MSGTYPE_ANNCE       41

#define OPT_SUCCESS         0

cJSON* json_all;
cJSON* json_dev;

int socket_cb;
struct sockaddr_in cb_addr;
int is_socket_valid;

pthread_t tid_hb = 0;
pthread_t tid_ep = 0;
void* thrd_hb(void* arg);
void* thrd_ep(void* arg);

struct flock* file_lock(short type, short whence);
int InitJsonAll();
int InitJsonDev();
int UpdateJsonAll(char* json_str, char* ieee, char* ep);
int UpdateJsonDev(char* ieee, char* ep);
char* GetEpByIEEE(char* ieee);
int DelDevFromJsonAll(char* ieee);
int SaveJsonAll();
int SaveJsonDev();
int GetBindList(char* ieee, char* ep);
int GetJsonStr(char* json_str, char* raw_str);

int main()
{
    int len;
    char buf[BUF_SIZE];
    int ret;

    if (InitJsonAll() < 0)
    {
        fprintf(stderr, "Init JsonAll failed!\n");
        return -1;
    }

    if (InitJsonDev() < 0)
    {
        fprintf(stderr, "Init JsonDev failed!\n");
        return -1;
    }

    memset(&cb_addr, 0, sizeof(cb_addr));
    cb_addr.sin_family = AF_INET;
    cb_addr.sin_addr.s_addr = inet_addr(DEST_IP);
    cb_addr.sin_port = htons(DEST_PORT);

    is_socket_valid = 0;
    
    // 创建一个线程用于定时发送心跳
    if (pthread_create(&tid_hb, NULL, thrd_hb, NULL) != 0)
    {
        pthread_kill(tid_hb, SIGKILL);
        perror("tid_hb create error: ");
        return -1;
    }

    while (1)
    {
        is_socket_valid = 0;

        if ((socket_cb = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            close(socket_cb);
            perror("socket_cb create error: ");
            sleep(RETRY_INTERVAL);
            continue;
        }

        if (connect(socket_cb, (struct sockaddr*)&cb_addr, sizeof(struct sockaddr)) < 0)
        {
            close(socket_cb);
            perror("socket_cb connect error: ");
            sleep(RETRY_INTERVAL);
            continue;
        }

        is_socket_valid = 1;

        while (1)
        {
            len = recv(socket_cb, buf, BUF_SIZE, 0);
            if (len <= 0)
            {
                fprintf(stderr, "receive callback data error\n");
                close(socket_cb);
                is_socket_valid = 0;
                sleep(RETRY_INTERVAL);
                break;
            }
            
            buf[len] = '\0';

            //printf("buf:\n%s\n", buf);

            cJSON* json_recv;
            cJSON* json_tmp;
            char* tmp_val;
            int msgtype;
            int status;
            char* ieee;
            char* ep;

            json_recv = cJSON_Parse(buf);
            if (!json_recv)
            {
                cJSON_Delete(json_recv);
                continue;
            }
            
            json_tmp = cJSON_GetObjectItem(json_recv, "msgtype");
            if (!json_tmp)
            {
                cJSON_Delete(json_recv);
                continue;
            }
            tmp_val = cJSON_PrintUnformatted(json_tmp);
            if (!tmp_val)
            {
                free(tmp_val);
                cJSON_Delete(json_recv);
                continue;
            }
            msgtype = atoi(tmp_val); 
            free(tmp_val);

            json_tmp = cJSON_GetObjectItem(json_recv, "IEEE");
            if (!json_tmp)
            {
                cJSON_Delete(json_recv);
                continue;
            }
            ieee = cJSON_PrintRawString(json_tmp);
            if (!ieee)
            {
                free(ieee);
                cJSON_Delete(json_recv);
                continue;
            }
            
            if (msgtype == MSGTYPE_BIND_DATA)
            {
                json_tmp = cJSON_GetObjectItem(json_recv, "EP");
                if (!json_tmp)
                {
                    free(ieee);
                    cJSON_Delete(json_recv);
                    continue;
                }
                ep = cJSON_PrintRawString(json_tmp);
                if (!ep)
                {
                    free(ieee);
                    free(ep);
                    cJSON_Delete(json_recv);
                    continue;
                }
            }

            switch (msgtype)
            {
                case MSGTYPE_BIND:
                case MSGTYPE_UNBIND:
                    printf("recv callback msgtype = %d\n", msgtype);
                    json_tmp = cJSON_GetObjectItem(json_recv, "status");
                    if (!json_tmp || json_tmp->type != cJSON_Number)
                    {
                        break;
                    }
                    tmp_val = cJSON_PrintUnformatted(json_tmp);
                    status = atoi(tmp_val);
                    if (status == OPT_SUCCESS)  // 只有当操作成功才更新绑定信息
                    {
                        char* tmp_ep = GetEpByIEEE(ieee);
                        if (tmp_ep)
                        {
                            // 如果对应设备的EP信息已存在于binddev.json中则直接
                            // 调用getBindList.cgi触发callback msgtype=28

                            GetBindList(ieee, tmp_ep);
                            free(tmp_ep);
                        }
                        else
                        {
                            // 如果对应设备的EP信息在binddev.json中不存在，则开
                            // 启新的线程读取对应设备的EP列表并更新到binddev.json
                            // 然后再调用getBindList.cgi触发callback msgtype=28

                            char arg[IEEE_LEN+1] = {0};
                            memcpy(arg, ieee, IEEE_LEN);
                            if (pthread_create(&tid_ep, NULL, thrd_ep, arg) != 0)
                            {
                                pthread_kill(tid_ep, SIGKILL);
                                perror("tid_ep create error: ");
                            }
                        }
                    }
                    free(tmp_val);
                    break;
                case MSGTYPE_BIND_DATA:
                    printf("recv callback msgtype = %d\n", msgtype);
                    // 如果绑定信息有变化，则更新到bindlist.json、binddev.json中
                    {
                        cJSON* json_add;
                        tmp_val = cJSON_PrintUnformatted(json_recv);
                        UpdateJsonAll(tmp_val, ieee, ep);
                        if (strcmp(ep, "02") != 0 && strcmp(ep, "03") != 0) // dev_list中只需保存设备的IEEE和第一个EP
                        {
                            UpdateJsonDev(ieee, ep);
                        }
                        free(tmp_val);
                        free(ep);
                    }
                    break;
                case MSGTYPE_ANNCE:
                    printf("recv callback msgtype = %d\n", msgtype);
                    // 每次dev_list中的设备激活都调用getBindList.cgi触发msgtype=28的callback消息
                    {
                        char* tmp_ep = GetEpByIEEE(ieee);
                        if (tmp_ep)
                        {
                            GetBindList(ieee, tmp_ep);
                            free(tmp_ep);
                        }
                    }
                    break;
                default: break;
            }

            cJSON_Delete(json_recv);
            free(ieee);
        }
    }
    
    close(socket_cb);
    cJSON_Delete(json_all);
    cJSON_Delete(json_dev);
    return 0;
}

void* thrd_hb(void* arg)
{
    while (1)
    {
        if (is_socket_valid)
        {
            send(socket_cb, HEART_BEAT, strlen(HEART_BEAT), 0);
        }
        sleep(HB_INTERVAL);
    }
}

void* thrd_ep(void* arg)
{
    int sock_fd;
    char request[REQ_SIZE] = {0};
    char* ieee;
    struct sockaddr_in remote_addr;

    int len;
    char buf[BUF_SIZE*2] = {0};
    char* recv_data = NULL;
    int recv_data_size = 0;

    if (!arg)
    {
        return;
    }

    ieee = (char*)arg;

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(DEST_IP);
    remote_addr.sin_port = htons(80);

    if ((sock_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        close(sock_fd);
        perror("sock_fd create error: ");
        return;
    }

    if (connect(sock_fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) < 0)
    {
        close(sock_fd);
        perror("sock_fd connect error: ");
        return;
    }

    sprintf(request, "GET /cgi-bin/rest/network/getIeeeEndPoint.cgi?" 
            "ieee=%s&callback=1234&encodemethod=NONE&sign=AAA "
            "HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", ieee);

    //printf("GetIeeeEndPoint request: \n%s\n", request);

    send(sock_fd, request, strlen(request), 0);

    while ((len = recv(sock_fd, buf, BUF_SIZE, 0)) > 0)
    {   
        recv_data = (char*)realloc(recv_data, sizeof(char)*(recv_data_size + len + 1));
        memcpy(recv_data + recv_data_size, buf, len);
        recv_data_size += len;
        *(recv_data + recv_data_size) = '\0';
    }   

    close(sock_fd);

    if (!recv_data)
    {
        return;
    }

    char json_str[BUF_SIZE*2] = {0};
    char* ep;
    cJSON* json_recv;
    cJSON* json_array;
    cJSON* json_item;
    cJSON* json_devparam;
    cJSON* json_tmp;
    int array_size = 0;
    int i;

    GetJsonStr(json_str, recv_data);
    json_recv = cJSON_Parse(json_str);
    if (!json_recv)
    {
        cJSON_Delete(json_recv);
        return;
    }
    free(recv_data);

    json_array = cJSON_GetObjectItem(json_recv, "response_params");
    if (!json_array)
    {
        cJSON_Delete(json_recv);
        return;
    }
    array_size = cJSON_GetArraySize(json_array);
    if (array_size == 0)
    {
        cJSON_Delete(json_recv);
        return;
    }

    for (i = 0; i < array_size; ++i)
    {
        json_item = cJSON_GetArrayItem(json_array, i );
        if (!json_item)
        {
            continue;
        }

        json_devparam = cJSON_GetObjectItem(json_item, "devparam");
        if (!json_devparam)
        {
            continue;
        }

        json_tmp = cJSON_GetObjectItem(json_devparam, "ep");
        if (!json_tmp)
        {
            continue;
        }

        ep = cJSON_PrintRawString(json_tmp);
        if (strcmp(ep, "02") != 0 || strcmp(ep, "03") != 0)
        {
            UpdateJsonDev(ieee, ep);
            GetBindList(ieee, ep);
        }
        free(ep);
    }

    cJSON_Delete(json_recv);
}

struct flock* file_lock(short type, short whence)
{
    static struct flock ret;
    ret.l_type = type ;
    ret.l_start = 0;
    ret.l_whence = whence;
    ret.l_len = 0;
    ret.l_pid = getpid();
    return &ret;
}

int InitJsonAll()
{
    FILE* fp;
    int fd;
    int file_size;
    char file_buf[BUF_SIZE*20];
    char* tmp;

    fp = fopen(BIND_LIST_FILE, "a+");
    if (fp == NULL)
    {
        fclose(fp);
        perror("open "BIND_LIST_FILE" error: ");
        return -1;
    }

    fd = fileno(fp);
    fcntl(fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    if (file_size > 0)
    {
        // 如果文件不为空，则将文件内容加载到json_all中
        fread(file_buf, sizeof(char), file_size, fp);
        json_all = cJSON_Parse(file_buf);
    }
    else
    {
        // 如果文件为空，则创建默认的json_all并保存到文件中
        json_all = cJSON_CreateObject();
        cJSON_AddNumberToObject(json_all, "request_id", 1234);
        cJSON_AddItemToObject(json_all, "response_params", cJSON_CreateArray());
        tmp = cJSON_PrintUnformatted(json_all);
        fwrite(tmp, sizeof(char), strlen(tmp), fp);
        free(tmp);
    }

    fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));   
    fclose(fp);
    return 0;
}

int InitJsonDev()
{
    FILE* fp;
    int fd;
    int file_size;
    char file_buf[BUF_SIZE*20];
    char* tmp;

    fp = fopen(BIND_DEV_FILE, "a+");
    if (fp == NULL)
    {
        fclose(fp);
        perror("open "BIND_DEV_FILE" error: ");
        return -1;
    }

    fd = fileno(fp);
    fcntl(fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));

    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    if (file_size > 0)
    {
        // 如果文件不为空，则将文件内容加载到json_dev中
        fread(file_buf, sizeof(char), file_size, fp);
        json_dev = cJSON_Parse(file_buf);
    }
    else
    {
        // 如果文件为空，则创建默认的json_dev并保存到文件中
        json_dev = cJSON_CreateObject();
        cJSON_AddItemToObject(json_dev, "dev_list", cJSON_CreateArray());
        tmp = cJSON_PrintUnformatted(json_dev);
        fwrite(tmp, sizeof(char), strlen(tmp), fp);
        free(tmp);
    }

    fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));   
    fclose(fp);
    return 0;
}

int UpdateJsonAll(char* json_str, char* ieee, char* ep)
{
    cJSON* json_add;
    cJSON* json_array;
    cJSON* json_item;
    cJSON* json_tmp;
    char* item_ieee;
    char* item_ep;
    int array_size;
    int i;
    int flag = 0;

    json_add = cJSON_Parse(json_str);

    json_array = cJSON_GetObjectItem(json_all, "response_params");
    array_size = cJSON_GetArraySize(json_array);
    
    for (i = 0; i < array_size; ++i)
    {
        json_item = cJSON_GetArrayItem(json_array, i);

        json_tmp = cJSON_GetObjectItem(json_item, "IEEE");
        item_ieee = cJSON_PrintRawString(json_tmp);
        
        json_tmp = cJSON_GetObjectItem(json_item, "EP");
        item_ep = cJSON_PrintRawString(json_tmp);

        if (strcmp(ieee, item_ieee) == 0 && strcmp(ep, item_ep) == 0)
        {
            // 如果对应EP的绑定信息已存在且有变化则更新并保存
            if (strcmp(cJSON_PrintUnformatted(json_add),
                        cJSON_PrintUnformatted(json_item)) != 0)
            {
                // 先删除已有的信息
                cJSON_DeleteItemFromArray(json_array, i);

                cJSON_AddItemToArray(json_array, json_add);
                printf("update JsonAll: IEEE=%s, EP=%s\n", ieee, ep);
                SaveJsonAll();
            }
            else
            {
                cJSON_Delete(json_add);
            }
            flag = 1;
            free(item_ieee);
            free(item_ep);
            break;
        }

        free(item_ieee);
        free(item_ep);
    }

    // 如果对应EP的绑定信息不存在则直接更新并保存
    if (!flag)
    {
        cJSON_AddItemToArray(json_array, json_add);
        printf("update JsonAll: IEEE=%s, EP=%s\n", ieee, ep);
        SaveJsonAll();
    }
    return 0;
}

int UpdateJsonDev(char* ieee, char* ep)
{
    cJSON* json_add;
    cJSON* json_array;
    cJSON* json_item;
    cJSON* json_tmp;
    char* item_ieee;
    char* item_ep;
    int array_size;
    int i;
    int flag = 0;

    json_array = cJSON_GetObjectItem(json_dev, "dev_list");
    array_size = cJSON_GetArraySize(json_array);
    
    for (i = 0; i < array_size; ++i)
    {
        json_item = cJSON_GetArrayItem(json_array, i);

        json_tmp = cJSON_GetObjectItem(json_item, "IEEE");
        item_ieee = cJSON_PrintRawString(json_tmp);
        
        json_tmp = cJSON_GetObjectItem(json_item, "EP");
        item_ep = cJSON_PrintRawString(json_tmp);

        if (strcmp(ieee, item_ieee) == 0 && strcmp(ep, item_ep) == 0)
        {
            flag = 1;
            free(item_ieee);
            free(item_ep);
            break;
        }

        free(item_ieee);
        free(item_ep);
    }

    // 如果对应设备EP的信息不存在则更新并保存
    if (!flag)
    {
        json_add = cJSON_CreateObject();
        cJSON_AddStringToObject(json_add, "IEEE", ieee);
        cJSON_AddStringToObject(json_add, "EP", ep);
        cJSON_AddItemToArray(json_array, json_add);
        printf("update JsonDev: IEEE=%s, EP=%s\n", ieee, ep);
        SaveJsonDev();
    }

    return 0;
}

char* GetEpByIEEE(char* ieee)
{
    cJSON* json_array;
    cJSON* json_item;
    cJSON* json_tmp;
    char* item_ieee;
    char* item_ep = NULL;
    int array_size;
    int i;

    json_array = cJSON_GetObjectItem(json_dev, "dev_list");
    array_size = cJSON_GetArraySize(json_array);
    
    for (i = 0; i < array_size; ++i)
    {
        json_item = cJSON_GetArrayItem(json_array, i);

        json_tmp = cJSON_GetObjectItem(json_item, "IEEE");
        item_ieee = cJSON_PrintRawString(json_tmp);
        
        if (strcmp(ieee, item_ieee) == 0)
        {
            json_tmp = cJSON_GetObjectItem(json_item, "EP");
            item_ep = cJSON_PrintRawString(json_tmp);
            free(item_ieee);
            return item_ep;
        }

        free(item_ieee);
    }

    return item_ep;
}

int DelDevFromJsonAll(char* ieee)
{
    cJSON* json_array;
    cJSON* json_item;
    cJSON* json_tmp;
    char* item_ieee;
    int array_size;
    int i;
    int count = 0;
    int flag = 1;

    json_array = cJSON_GetObjectItem(json_all, "response_params");
    array_size = cJSON_GetArraySize(json_array);

    while (flag)
    {
        flag = 0;
        for (i = 0; i < array_size; ++i)
        {
            json_item = cJSON_GetArrayItem(json_array, i);

            json_tmp = cJSON_GetObjectItem(json_item, "IEEE");
            item_ieee = cJSON_PrintRawString(json_tmp);

            if (strcmp(ieee, item_ieee) == 0)
            {
                cJSON_DeleteItemFromArray(json_array, i);
                array_size = cJSON_GetArraySize(json_array);
                count++;
                flag = 1;
                break;
            }
        }
    }

    if (count > 0)
    {
        SaveJsonAll();
    }
    return 0;
}

int SaveJsonAll()
{
    FILE* fp;
    int fd;
    char* tmp;

    fp = fopen(BIND_LIST_FILE, "w+");
    if (fp == NULL)
    {
        fclose(fp);
        perror("open "BIND_LIST_FILE" error: ");
        return -1;
    }

    fd = fileno( fp );
    fcntl(fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));

    tmp = cJSON_PrintUnformatted(json_all);
    fwrite(tmp, sizeof(char), strlen(tmp), fp);
    free(tmp);

    fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
    fclose(fp);
    //printf("save JsonAll\n");
    return 0;
}

int SaveJsonDev()
{
    FILE* fp;
    int fd;
    char* tmp;

    fp = fopen(BIND_DEV_FILE, "w+");
    if (fp == NULL)
    {
        fclose(fp);
        perror("open "BIND_DEV_FILE" error: ");
        return -1;
    }

    fd = fileno( fp );
    fcntl(fd, F_SETLKW, file_lock(F_WRLCK, SEEK_SET));

    tmp = cJSON_PrintUnformatted(json_dev);
    fwrite(tmp, sizeof(char), strlen(tmp), fp);
    free(tmp);

    fcntl(fd, F_SETLK, file_lock(F_UNLCK, SEEK_SET));
    fclose(fp);
    //printf("save JsonDev\n");
    return 0;
}

int GetBindList(char* ieee, char* ep)
{
    int socket_fd;
    char request[REQ_SIZE] = {0};
    struct sockaddr_in remote_addr;

    if (!ieee || !ep)
    {
        return -1;
    }

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(DEST_IP);
    remote_addr.sin_port = htons(80);

    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        close(socket_fd);
        perror("socket_fd create error: ");
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) < 0)
    {
        close(socket_fd);
        perror("socket_fd connect error: ");
        return -1;
    }

    sprintf(request, "GET /cgi-bin/rest/network/getBindList.cgi?" 
            "ieee=%s&ep=%s&callback=1234&encodemethod=NONE&sign=AAA "
            "HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", ieee, ep);

    //printf("GetBindList request: \n%s\n", request);

    send(socket_fd, request, strlen(request), 0);
    close(socket_fd);
}

int GetJsonStr(char* json_str, char* raw_str)
{
    char* s_begin;
    char* s_end;
    int len;
    s_begin = strchr(raw_str, '{');
    s_end = strrchr(raw_str, '}');

    if (!s_begin || !s_end)
    {
        return -1;
    }

    len = s_end - s_begin + 1;
    strncpy(json_str, s_begin, len);
    json_str[len] = '\0';
    return 0;
}

