#ifndef BIGINFO_INCLUDE2
#define BIGINFO_INCLUDE2
#include "tool_common.h"
// 文件下buffer 长度默认4096
void big_send_start(char *buffer,unchar target_serverid,
unchar self_serverid,uint32_t total_size);
void big_send_running(char *buffer,unchar target_serverid,
unchar self_serverid,char *data,int data_size);
// no copy only build head
void big_send_running2(char *buffer,unchar target_serverid,
unchar self_serverid,int data_size);


void big_send_end(char *buffer,unchar target_serverid,
unchar self_serverid);
typedef struct 
{
    char *servers[256];
    int write_tail[256];
    int server_size[256];

}big_infos;
#define BIG_SIZE_MAX (1024*1024*32) // 最大32MB 自己选

void ini_recv_big(big_infos *data);

void * recv_big_start(commend_triggers_param *data);
void * recv_big_running(commend_triggers_param *data);



void * recv_big_end(commend_triggers_param *data);
void recv_big_reset(big_infos *data,unchar id);
//     size + target +cmd +self 已经被删除
// mserverid + size(file) 准备发送
// mserverid + data	      开始发送
// mserverid 		发送结束
#endif