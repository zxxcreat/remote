#ifndef CENTER_INCLUDE2
#define CENTER_INCLUDE2
#include "tool_common.h"


typedef struct 
{
    char ipstr[50];
    int ipstr_size;
    int port;
    int group_id;
    char password[50];
    int password_len;

    struct sockaddr_in addr;
    int socket;

    int is_connect;

}center_config;
#define CENTER_CONNECT_READY 10
int center_read_config(center_config *config,lua_State *vm);
int center_connect(center_config *config,char *ip,int ports);
int center_send_password(center_config *config);
int center_register(center_config *config,unchar id);


int epoll_tcp_send(int socket_fd,char *data,int size);
#endif