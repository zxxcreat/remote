#include <zxx_common/common.h>


#define LOGS(a,b) sprintf(g_gate.logcache,a,b);log1(g_gate.logcache);

// 直接发送模式
#define BASE_SIZE_SEND (1024*128)
#define BASE_SIZE_MAX_INDEX 7
// 128 256 512 1024 2048 4096 8192
// 单独的cmd

int gate_ini();
int ini_lua_config(void);
int ini_epoll(void);

#define CACHE_NORMAL 3000
#define TIME_OUT_HEART 30
enum {
    FD_CHECK_SHA256=0,
    FD_CHECK_SECRET,
    FD_CHECK_ENCODE,
    FD_CHECK_FINISH
};
typedef struct 
{
    int socket;
    char ipstr[50];
    u64 tid;

    // send_target 缓存存在哪里
    int send_target;

    // 128 256 512 1024 2048 4096
    // send_rest 上一次没发完的 存进去
    CircularBuffer *send_rest;
    CircularBuffer *send_list[10];


    int re_tail;
    int re_max;
    char re_buffer[CACHE_NORMAL];

    // 个人aes
    AES_KEY aes_key_encode;
    AES_KEY aes_key_decode;
    // 加密的key
    char aes256_key32[32];
    // cbc 解密iv
    char aes256_iv16_encode[16];
    // cbc 加密密iv
    char aes256_iv16_decode[16];

    Node heart;

    unchar trans_server;

    int socket_state;
    
}single_socket;
u64 run_tid(void);
int ini_single_socket(int socket,char *ip,int ip_size,
single_socket * result,int need_send_buffer);
int ini_single_128(single_socket * result);
int delete_single(single_socket * result);


//////////////////
#define ACCEPT_MODE_NULL 0
#define ACCEPT_MODE_BLACK 1
#define ACCEPT_MODE_WRILT 2
typedef struct 
{
    lua_State * luavm;
    int ip_type;
    char ipstr[50];
    int ipstr_size;

    int port;
    int defult_id;
    int group_id;
    char logcache[2048];

    center_config center;

    struct sockaddr_in _addr4;
    struct sockaddr_in6 _addr6;

    int epollfd;
    int epollsocket;

    u64 tid_index;
    NodeInfo heart_beat;


    std::unordered_map<int,single_socket *> epoll_data;
    std::unordered_map<u64,int> tid2socket;

    i64 time_now;

    int accept_mode;
    // 4 是直接ip v6是md5的最后8位
    std::unordered_map<u64,int> white_ip4_list;
    std::unordered_map<u64,int> white_ip6_list;

    
    std::unordered_map<u64,int> black_ip4_list;
    std::unordered_map<u64,int> black_ip6_list;


    commend_trigger cmds[256];
    char key_join[50];
    int key_join_size;
    char check_str[50];
    int check_str_size;

    random_buffer  randoms;

    char send_cache[4096];
    char send_cache2[4096];
    big_infos big_buffer;
    commend_triggers_param params;




    char local_ip[50];
    int local_ip_len;
    int local_port;
}globe_gateway;
globe_gateway g_gate;
int epoll_pre_accept(char *ip ,int ip_size);
void epoll_post_accept(char *ip,int ip_size,int socket_fd);
void epoll_delete_single_socket(single_socket *data);

void epoll_on_receive(single_socket *data);

void deal_center_msg(single_socket *data,char *buffer,int size);
int deal_client_msg(single_socket *data,char *buffer,int size);
int  tcp_send_client(single_socket *data,int epollfd,char *buffer,int size);
int  tcp_send_centers(center_config *center,char *buffer,int size);
//epoll_delete_single_socket
//epoll_post_accept

void * recv_center_register(commend_triggers_param *data);
void * send2client(commend_triggers_param *data);
int send2client_single(single_socket *data,char *buffer,int size);







