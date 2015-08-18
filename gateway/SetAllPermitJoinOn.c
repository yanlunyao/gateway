/********************************************************************************
 * 文件名：SetAllPermitJoinOn.c
 * 创建者：马晓槟
 * 创建时间：2014-08-25
 * 功能描述：打开ZigBee网络中协调器及所有路由器的允许入网功能。
 * 参数说明：
 *    second：允许入网的时间，单位s
 *    callback、encodemethod、sign和大洋其他API相同
 *    传给SetAllPermitJoinOn.cgi的second、callback、encodemethod和sign参数会直接
 *    传给setPermitJoinOn.cgi。
 * 实现思路：先调用getZBNode.cgi获取ZigBee网路中的协调器及所有路由器的IEEE地址，
 *           然后再对获取到IEEE地址的每个设备调用setPermitJoinOn.cgi。
 *
 * 编译指令：
 *    $ arm-linux-gcc cJSON/cJSON.c SetAllPermitJoinOn.c -lm -o SetAllPermitJoinOn.cgi
 ********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "cJSON.h"

#define BUF_SIZE        1024 
#define ZB_EndDevice    2

#define ST_Success          0
#define ST_QueryString      1
#define ST_TooFewPara       2
#define ST_SockCreate       3
#define ST_SockConnect      4
#define ST_GetInfo          5
#define ST_JsonParse        6
#define ST_InvalidResPara   7
#define ST_ArraySize        8
#define ST_ParaSecond       9

const char GetZBNode[] = "GET /cgi-bin/rest/network/getZBNode.cgi?"
                        "callback=1234&encodemethod=NONE&sign=AAA HTTP/1.1\r\n"
                        "Host: 127.0.0.1\r\n\r\n";

void printResult(int status)
{
    char* msg = NULL;
    switch (status)
    {
        case ST_Success: msg = "success"; break;
        case ST_QueryString: msg = "query string is null"; break;
        case ST_TooFewPara: msg = "too few parameter"; break;
        case ST_SockCreate: msg = "socket create error"; break; 
        case ST_SockConnect: msg = "socket connect error"; break; 
        case ST_GetInfo: msg = "get node info error"; break;
        case ST_JsonParse: msg = "json parse error"; break;
        case ST_InvalidResPara: msg = "invalide response_params"; break;
        case ST_ArraySize: msg = "array size is zero"; break;
        case ST_ParaSecond: msg = "parameter second error"; break;
        default: break;
    }
    printf("{\"request_id\":1234,\"response_params\":"
            "{\"status\":%d,\"status_msg\":\"%s\"}}", status, msg);
}

int main()
{
    char* para_str = NULL;
    char* s_begin = NULL;
    char para[30] = {0};
    char* tmp_ptr = para;
    printf("Content-type: text/html\n\n");      // CGI返回头

    if (!getenv("QUERY_STRING"))
    {
        printResult(ST_QueryString);
        return -1;
    }

    // 获取传入的所有参数
    para_str = getenv("QUERY_STRING");

    if (!(s_begin = strstr(para_str, "second="))
            || !strstr(para_str, "callback=")
            || !strstr(para_str, "encodemethod=")
            || !strstr(para_str, "sign="))
    {
        printResult(ST_TooFewPara);
        return -1;
    }

    // TODO: 判断每个参数的合法性

    s_begin = strstr(s_begin, "=");
    s_begin++;
    if (*s_begin == '\0' || *s_begin == '&')
    {
        printResult(ST_ParaSecond);
        return -1;
    }

    while ((*tmp_ptr++ = *s_begin++) != '\0')
    {
        if (*s_begin == '&')
        {
            break;
        }
    }

    //=========================================================================
    // 调用getZBNode.cgi，并获取返回结果

    int socket_fd;
    int len;
    struct sockaddr_in remote_addr;
    char buf[BUF_SIZE] = {0};
    char* recv_data = NULL;
    int recv_data_size = 0;

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    remote_addr.sin_port = htons(80);

    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        close(socket_fd);
        printResult(ST_SockCreate);
        return -1;
    }

    if (connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) < 0)
    {
        close(socket_fd);
        printResult(ST_SockConnect);
        return -1;
    }

    send(socket_fd, GetZBNode, strlen(GetZBNode), 0);

    while ((len = recv(socket_fd, buf, BUF_SIZE, 0)) > 0)
    {
        recv_data = (char*)realloc(recv_data, sizeof(char)*(recv_data_size + len + 1));
        memcpy(recv_data + recv_data_size, buf, len);
        recv_data_size += len;
        *(recv_data + recv_data_size) = '\0';
    }
    
    close(socket_fd);

    if (!recv_data)
    {
        printResult(ST_GetInfo);
        return -1;
    }

    //=========================================================================

    cJSON* json_recv;
    cJSON* json_array;
    cJSON* json_node;
    cJSON* json_tmp;

    char* begin = strchr(recv_data, '{');
    char* end = strrchr(recv_data, '}');

    if (!begin || !end)
    {
        free(recv_data);
        printResult(ST_JsonParse); 
        return -1;
    }

    int json_len = end - begin + 1;
    char* json_str = (char*)calloc(sizeof(char), json_len + 1);   // 增加一个字节保存'\0'
    strncpy(json_str, begin, json_len);
    free(recv_data);
    recv_data = NULL; 
    json_recv = cJSON_Parse(json_str);
    if (!json_recv)
    {
        free(json_str);
        cJSON_Delete(json_recv);
        printResult(ST_JsonParse); 
        return -1;
    }
    free(json_str);
    json_str = NULL;

    json_array = cJSON_GetObjectItem(json_recv, "response_params");
    if (!json_array)
    {
        cJSON_Delete(json_recv);
        printResult(ST_InvalidResPara);
        return -1;
    }
    int array_size = cJSON_GetArraySize(json_array);
    if (array_size == 0)
    {
        cJSON_Delete(json_recv);
        printResult(ST_ArraySize);
        return -1;
    }

    char* ieee = NULL;
    char request[BUF_SIZE] = {0};
    int i = 0;
    for (i = 0; i < array_size; ++i)
    {
        // 对于其中某个设备操作失败直接忽略

        json_tmp = cJSON_GetArrayItem(json_array, i);
        if (!json_tmp) 
        {
            continue;
        }

        json_node = cJSON_GetObjectItem(json_tmp, "node");
        if (!json_node)
        {
            continue;
        }

        json_tmp = cJSON_GetObjectItem(json_node, "node_type");
        if (!json_tmp) 
        {
            continue;
        }

        if (atoi(cJSON_PrintUnformatted(json_tmp)) == ZB_EndDevice)
        {
            continue;
        }

        json_tmp = cJSON_GetObjectItem(json_node, "ieee");
        if (!json_tmp) 
        {
            continue;
        }

        ieee = cJSON_PrintRawString(json_tmp);
        sprintf(request, "GET /cgi-bin/rest/network/setPermitJoinOn.cgi?"
                "ieee=%s&%s HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n", ieee, para_str);
        free(ieee);

        if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            close(socket_fd);
            continue;
        }

        if (connect(socket_fd, (struct sockaddr*)&remote_addr, sizeof(struct sockaddr)) < 0)
        {
            close(socket_fd);
            continue;
        }
        send(socket_fd, request, strlen(request), 0);
        close(socket_fd);
    }

    cJSON_Delete(json_recv);
    printResult(ST_Success);

    char cmd[BUF_SIZE] = {0};
    sprintf(cmd, "/gl/bin/led-zigbee %s > /dev/null 2>&1 &", para);
    system("killall -9 led-zigbee");
    system(cmd);

    return 0;
}

