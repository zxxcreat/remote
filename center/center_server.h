#include <zxx_common/common.h>
#define S_STATE_UNPROMISE 0
#define S_STATE_CHECKED 1
typedef struct{
    // 对应的socket
    int server[256];
    
    char cache[4096];
    lua_State * luavm;
    center_config center;
    int epollfd;
    int socket;
    struct sockaddr_in _addr4;
    struct sockaddr_in6 _addr6;
    u64 time_now;

    commend_trigger cmds[256];
    commend_triggers_param params;

}globe_center;  


typedef struct{
    int socket;
    char *buffer;
    int buffer_size;
    int tail;
    int state;
    unchar register_id;
}servers_single;  

servers_single * ini_servers_single(int Socket);
void delete_servers_single(servers_single *data);



int ini_center_server();
int center_on_accept(epoll_event * data);
int center_on_receive(epoll_event * data);

int center_close(epoll_event * data);
int deal_servers_cmd(servers_single * data);
void * center_cmd_CENTER_0_register(commend_triggers_param *data);
