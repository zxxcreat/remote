#include "common_send.h"
globe_common *g_common2;
commons_caches_globe g_recv2;
//
void* thread_send_main(void* arg){
    g_common2=(globe_common *)arg;
    g_recv2.tail=0;
    unchar groupself=g_common2->self_group;
    while(1){
        //获取
        sem_wait(&g_common2->send_b_semaphore);
        pop_QUEUE_manage(g_common2->send_b,g_recv2.buffer,&g_recv2.tail);
        g_recv2.buffer[g_common2->send_b->reset_index]=groupself;
        // 可以在这里序列化和反序列化
        sem_wait(&g_common2->center_semaphore);
        epoll_tcp_send(g_common2->center.socket,g_recv2.buffer,g_recv2.tail);
        sem_post(&g_common2->center_semaphore);
    }
    // 数据库的写进程 把东西写进数据库
}
