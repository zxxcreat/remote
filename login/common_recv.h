#ifndef C_RECV_INCLUDE1
#define C_RECV_INCLUDE1
#include "common_s.h"
// 三个线程  发送 接收 logic
// target 转为index
#define CACHE_MAX 4096
typedef struct{
    char buffer[CACHE_MAX];
    int tail;
}commons_caches_globe;

void* thread_recv_main(void* arg);
//int find_index_push(int size);
void* thread_recv_main22(void* arg);
#endif