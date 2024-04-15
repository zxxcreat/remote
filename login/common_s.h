#ifndef LOGIN_C_INCLUDE2
#define LOGIN_C_INCLUDE2

#include <zxx_common/common.h>
#include <shares.h>
#include <semaphore.h>
#include <sqlite/sqlite3.h>
#include "common_recv.h"
#include "common_send.h"
#include "common_main.h"
#include "run.h"

// 1 2 4 8 16 32 64 128 256 512 最大1g的队列大小 



#define SERVER_MODE_LOGIN 1 
#define SERVER_MODE_LOGIC 2 
#define SERVER_MODE_SETTING SERVER_MODE_LOGIN
// typedef struct{
//     // 接收锁
//     sem_t semaphore;
//     // 发送方专用
//     int index_next;
//     int max_index_used;

//     CircularBuffer *buffer[10];
// }queue_info;
typedef struct{




    // 对应的socket
    sem_t center_semaphore;
    config_all config;
    center_config center;


    random_buffer  randoms;
    big_infos big_buffer;
    commend_triggers_param params;
    commend_trigger cmds[256];
    u64 time_now;


    lua_State * luavm;
    
    struct sockaddr_in _addr4;
    

    // 最大一个g的队列 互斥访问 再加一个index 表示下一个的index
    // 也就是 size 4 + tar 1 + cmd + send 1  +x 额外信息 大小 表示线程传来的额外信息
    // queue_info recv_b;
    // queue_info send_b;
    QUEUE_manage *recv_b;
    sem_t recv_b_semaphore;
    
    QUEUE_manage *send_b;
    sem_t send_b_semaphore;

    unchar self_group;
    char *self_ipstr;
    int self_port;


    delay_manage delay_info;
    delay_single delay_param;

    char recv_center[4096];
    char head_sendcahche[HEAD_LOCAL_COUNT_SIZE];
    char head_sendcahche2[HEAD_LOCAL_COUNT_SIZE];
    QUEUE_param_split split_param;
    char split_cache[4096];

}globe_common;

void logic_run();

void * test_p(void *);



#endif