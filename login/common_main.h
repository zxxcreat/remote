#ifndef LOGIN_INCLUDE1
#define LOGIN_INCLUDE1
#include "common_s.h"
// 三个线程  发送 接收 logic
// target 转为index

// sendto servers;
// sendto clients;
void send_servers(unchar target,unchar cmd,char *buffer,int size);
u64 normal_gen_index(unchar type,u64 index);
#endif

