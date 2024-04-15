#ifndef CONFIGS_INCLUDE2
#define CONFIGS_INCLUDE2
#include "tool_common.h"
typedef struct{
    char ip_str[50];
    int ip_size;
    int port;
    unchar group_id;
}config_login;
typedef struct{
    char ip_str[50];
    int ip_size;
    int port;
    unchar group_id;
}config_logic;
typedef struct{
    config_logic logic;
    config_login login;
    u64 servers_id;
}config_all;

int config_read_servers(config_all *data,lua_State *vm);
#endif