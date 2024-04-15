#include "common_recv.h"
globe_common *g_common1;
commons_caches_globe g_recv1;
char log_caches[4096];
void* thread_recv_main(void* arg){
    g_common1=(globe_common *)arg;
    g_recv1.tail=0;
    int rest=0;
    int result=0;
    // 额外的信息0 多少个 1 是index
    int next_index=0;
    while(1){
        rest=CACHE_MAX-g_recv1.tail;
        result=recv(g_common1->center.socket,&g_recv1.buffer[g_recv1.tail],rest,0);
        if (result<0){
            // 需要锁住center 然后 无限重联
            sem_wait(&g_common1->center_semaphore);
            while(1){
                sleep(1);
                close(g_common1->center.socket);
                /////////////////////// 需要修改的地方 
                center_config *temc=&g_common1->center;
                unchar self_id=g_common1->self_group;
                char *ip=g_common1->self_ipstr;
                int port=g_common1->self_port;
                //////////////////////
                if (0!=center_connect(temc,ip,port)){
                    log1("center config 连接\n");
                    continue;
                }
                if (0!=center_send_password(temc)){
                    log1("center config 密码失败\n");
                    continue;
                }
                if (0!=center_register(temc,self_id)){
                    log1("center config 注册失败\n");
                    continue;
                }
                break;
            }
            sem_post(&g_common1->center_semaphore);
        }
        if (g_recv1.tail<sizeof(uint32_t)){
            continue;
        }
        int send_size=epoll_get_data_size(g_recv1.buffer);
        if (send_size<=sizeof(uint32_t)|| send_size>=MAX_SEND_SIZE){
            sprintf(log_caches,"传送的数据 字节数有问题 %d \n",send_size);
            log1(log_caches);
            return 0;
        }
        if (send_size<g_recv1.tail){
            continue;
        }
        // 增加额外信息 然后放到buffer 只负责post 加锁吗？
        

        
        // sem_wait(&g_common1->recv_b.semaphore);
        
        // int send_indexs=find_index_push(ex_data[0]+send_size);
        // ex_data[1]=send_indexs;
        // insert_CircularBuffer(
        //     g_common1->recv_b.buffer[send_indexs],
        //     g_recv1.buffer,
        //     LOCAL_SEND_SIZE+LOCAL_SEND_TCS
        // );
        // insert_CircularBuffer(
        //     g_common1->recv_b.buffer[send_indexs],
        //     (char *)ex_data,
        //     2
        // );
        // insert_CircularBuffer(
        //     g_common1->recv_b.buffer[send_indexs],
        //     &g_recv1.buffer[LOCAL_SEND_SIZE+LOCAL_SEND_TCS],
        //     send_size-(LOCAL_SEND_SIZE+LOCAL_SEND_TCS)
        // );
        // sem_post(&g_common1->recv_b.semaphore);
        // 序列化和反序列化可以在这进行 好了之后再发送给logic 
        insert_QUEUE_manage(g_common1->recv_b,g_recv1.buffer,send_size);
        sem_post(&g_common1->recv_b_semaphore);
        
        int unprocessed_length = g_recv1.tail - send_size;
        memmove(g_recv1.buffer, &g_recv1.buffer[send_size], unprocessed_length);
        g_recv1.tail = unprocessed_length;
    }
    

}
void* thread_recv_main22(void* arg){
    g_common1=(globe_common *)arg;
    //测试
    u64 sends[1000];
    u64 count=0;
    int send_time=100;
    char bufferss[4096];


    u64 t1=times_get_time();
    i64 total_size=0;
    i64 olds=0;
    int GBtr=0;
    i64 count3=0;
    printf("thread_recv_main22--------------------------1\n");
    while(1){
        for (int i=0;i<send_time;i++){
            sends[i]=count3;
            count3++;
        }
        epoll_ini_data_size_cmd2(bufferss,0,0,0);
        memcpy(&bufferss[epoll_get_data_size(bufferss)],sends,send_time*sizeof(u64));
        epoll_set_data_size(bufferss,epoll_get_data_size(bufferss)+send_time*sizeof(u64));




        


        
        // printf("manager %d %d %d %d  \n",
        // g_common.recv_b->get_index,
        // g_common.recv_b->next_index,
        // g_common.recv_b->times_use,
        // g_common.recv_b->max_index
        // );
        // printf("----------------------------2 %lld %d\n",count,epoll_get_data_size(bufferss));
        // for (int i=0;i<10;i++){
        //     if (g_common.recv_b->list[i]){
        //         printf(" %d %d %d %d  \n",
        //             g_common.recv_b->list[i]->head,
        //             g_common.recv_b->list[i]->tail,
        //             g_common.recv_b->list[i]->maxSize,
        //             i
        //         );
        //     }
        // }

        // printf("-----------------sai----------------------------\n");
        insert_QUEUE_manage(g_common1->recv_b,bufferss,epoll_get_data_size(bufferss));
        sem_post(&g_common1->recv_b_semaphore);









        total_size+=epoll_get_data_size(bufferss);
        if (total_size/(1024*1024*1024)!=olds){
            olds=total_size/(1024*1024*1024);
            printf("传输了1gb %lld %lld %llu\n",count3,total_size,times_get_time()-t1);
            
            GBtr++;

            sleep(1);
            t1=times_get_time();
            // if (GBtr % 5==0){
            //     printf("暂停1s %lld \n",count3);
            //     sleep(1);


               
            //     i64 count44=0;
            //     while(1){
                   
            //         count44++;
            //         if (count44 %1000==0){
            //             printf("暂停1s %lld %d\n",count3,g_common1->recv_b->max_index);
            //             sleep(1);
            //         }
            //          //继续传输 1s传输1000个数据
            //         for (int i=0;i<send_time;i++){
            //             sends[i]=count3;
            //             count3++;
            //         }
                     
            //         epoll_ini_data_size_cmd2(bufferss,0,0,0);
            //         memcpy(&bufferss[epoll_get_data_size(bufferss)],sends,send_time*sizeof(u64));
            //         epoll_set_data_size(bufferss,epoll_get_data_size(bufferss)+send_time*sizeof(u64));
            //         insert_QUEUE_manage(g_common1->recv_b,bufferss,epoll_get_data_size(bufferss));
            //         sem_post(&g_common1->recv_b_semaphore);
                    
            
            //     }
                



            //     exit(1);
            // }
        }
        count++;
    }

   
}