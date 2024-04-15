#include "common_main.h"
globe_common g_common;

int main() {
    // sem_t semaphore;

    // // 创建匿名信号量
    // sem_init(&semaphore, 0, 1);
    // sem_destroy(&semaphore);
    // 
    g_common.luavm=lua_ini();
    
    lua_State * tvm=g_common.luavm;
    //配置文件
    if (luaL_loadfile(tvm, "script/config.lua") != LUA_OK) {
        lua_error_m(tvm);
        printf("lua 代码加载失败\n");
        return -1;
    }
    // 执行 Lua 函数
    if (lua_pcall(tvm, 0, 0, 0) != LUA_OK) {
        lua_error_m(tvm);
        printf("lua 代码加载失败\n");
        return -1;
    }
    lua_settop(tvm, 0);
    if (config_read_servers(&g_common.config,g_common.luavm)==-1){
        log1("读取配置文件失败！\n");
        return -1;
    }


    ///////////////////////////////
 




    //随机数初始化
    ini_random_seed();
    ini_pre_random(&g_common.randoms);
    // 还有什么
    ini_recv_big(&g_common.big_buffer);
    //初始化param 除了center 都需要初始化这个
    g_common.params.big_info=&g_common.big_buffer;
    g_common.params.deals=g_common.cmds;






    g_common.recv_b=ini_QUEUE_manage(4);
    g_common.send_b=ini_QUEUE_manage(6);
    sem_init(&g_common.recv_b_semaphore, 0, 0);
    sem_init(&g_common.send_b_semaphore, 0, 0);

    for (int i=0;i<sizeof(g_common.cmds)/sizeof(g_common.cmds[0]);i++){
        g_common.cmds[i]=NULL;
    }
    g_common.cmds[RECV_BIG_START]=recv_big_start;
    g_common.cmds[RECV_BIG_RUNNING]=recv_big_running;
    g_common.cmds[RECV_BIG_END]=recv_big_end;






//     printf("开始\n");
//  pthread_t threads1; // 存储线程标识符的数组
//     if (pthread_create(&threads1, NULL, thread_recv_main22, (void *)&g_common)) {
//         fprintf(stderr, "Error creating thread\n");
//         return 1;
//     }


//  char buffss[4096];
//     int sizes;

//     char *bufferss2=0;
//     int sizess2=0;

//     u64 count=0;
//     int amount=100;



//     u64 save64[1000];
//      printf("------------1\n");
//     while (1){
//         sem_wait(&g_common.recv_b_semaphore);
//         //printf("-----------------pop count %dll\n",count);
//         pop_QUEUE_manage(g_common.recv_b,buffss,&sizes);
//         bufferss2=&buffss[LOCAL_SEND_SIZE+LOCAL_SEND_TCS];
//         sizess2=sizes-LOCAL_SEND_SIZE-LOCAL_SEND_TCS;
//         if (sizess2 != sizeof(u64)*amount){
//             printf("传输出错 1 %d %lld\n",sizess2,sizeof(u64)*amount);
//             exit(1);
//         }
//         memcpy(save64,bufferss2,sizess2);
//         for (int i=0;i<amount;i++){
//             if (count!=save64[i]){
//                 printf("传输出错2  %lld %lld\n",count,save64[i]);
//                 printf("-----------------pop count %dll\n",count);
//                 exit(1);
//             }

//             count++;
//         }
//     }
//     return 0;






    // 测试一下
    
    // 设置为1 可以使用
    sem_init(&g_common.center_semaphore, 0, 1);




    // 初始化的时候 确定自己是的 groupid ip 和str 如何确定？
    #if (SERVER_MODE_SETTING == SERVER_MODE_LOGIN)
        g_common.self_group=g_common.config.login.group_id;
        g_common.self_ipstr=g_common.config.login.ip_str;
        g_common.self_port=g_common.config.login.port;
    #endif

    #if (SERVER_MODE_SETTING == SERVER_MODE_LOGIC)
        g_common.self_group=g_common.config.logic.group_id;
        g_common.self_ipstr=g_common.config.logic.ip_str;
        g_common.self_port=g_common.config.logic.port;
    #endif

    center_config *temc=&g_common.center;
    unchar self_id=g_common.self_group;
    char *ip=g_common.self_ipstr;
    int port=g_common.self_port;
    if (0!=center_read_config(temc,tvm)){
        log1("center config 读取失败\n");
        return -1;
    }
    if (0!=center_connect(temc,ip,port)){
        log1("center config 连接\n");
        return -1;
    }
    if (0!=center_send_password(temc)){
        log1("center config 密码失败\n");
        return -1;
    }
    if (0!=center_register(temc,self_id)){
        log1("center config 注册失败\n");
        return -1;
    }
    
    lua_settop(tvm, 0);
    log1("配置读取成功！ 成功连接到center\n");

    if (runing_on_start(&g_common)==-1){
        log1("预启动失败！！！\n");
        return -1;
    }
    sleep(1);
    
    // 一个发送 一个接收线程
    pthread_t threads[2]; // 存储线程标识符的数组
    if (pthread_create(&threads[0], NULL, thread_recv_main, (void *)&g_common)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    if (pthread_create(&threads[1], NULL, thread_send_main, (void *)&g_common)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    // 本地线程 连接socket 存信息 就登陆注册两个技能
    // 循环和等待信号量都可以
    // 循环就是不断的获取sem value
    // 不循环就是直接等待
   
    logic_run();

    // 等待所有线程完成
    for (int i = 0; i < 2; i++) {
        if (pthread_join(threads[i], NULL)) {
            fprintf(stderr, "Error joining thread\n");
            return 2;
        }
    }

    printf("All threads completed successfully\n");

    return 0;
}

void logic_run(){
    //同一时间 最多有256个dealycall 多了不行
    struct timespec timeout;
    i64 rest_call;
    i64 diff=0;
    i64 time_n2=0;
    int result;


    int sizes=0;
    while (1){
        // 查看delay函数 // 定时器和延时调用还是需要的
       
        clock_gettime(CLOCK_REALTIME, &timeout);  // 获取当前时间
        g_common.time_now=timeout.tv_sec;
        g_common.time_now=g_common.time_now*1000;
        g_common.time_now+=(timeout.tv_nsec/1000000);
        time_n2=g_common.time_now;
        
        rest_call=count_rest_delay_manage(&g_common.delay_info,g_common.time_now);
        if (rest_call<=0){
           rest_call=0;
           runing_delay_manage(&g_common.delay_info,g_common.time_now);
        }
        diff=rest_call;
        timeout.tv_sec+=(rest_call/1000);
        rest_call=rest_call%1000;
        rest_call=rest_call*1000000;
        timeout.tv_nsec+=rest_call;
        if (timeout.tv_nsec >= 1000000000) {
            timeout.tv_sec += 1;
            timeout.tv_nsec -= 1000000000;
        }

        result=sem_timedwait(&g_common.recv_b_semaphore,&timeout);
        g_common.time_now=times_get_time();
        if (g_common.time_now - time_n2>=diff){
            runing_delay_manage(&g_common.delay_info,g_common.time_now);
        }
        if (result ==-1 &&errno == ETIMEDOUT){
            continue;
        }
        pop_QUEUE_manage(g_common.recv_b,g_common.recv_center,&sizes);
        
        g_common.params.buffer=&g_common.recv_center[(LOCAL_SEND_SIZE+LOCAL_SEND_TCS)];
        g_common.params.buffer_size=sizes-(LOCAL_SEND_SIZE+LOCAL_SEND_TCS);
        g_common.params.send_group=g_common.recv_center[GROUP_ID_INDEX];
        unchar cmdss=g_common.recv_center[CMD_INDEX_BIN];
        if (g_common.params.buffer_size>0){
            if (g_common.cmds[cmdss]!=NULL){
                g_common.cmds[cmdss](&g_common.params);
            }
        }
        

        // (LOCAL_SEND_SIZE+LOCAL_SEND_TCS);
    }
}


void send_servers(unchar target,unchar cmd,char *buffer,int size){
    epoll_ini_data_size_cmd2(g_common.head_sendcahche,target,cmd,g_common.self_group);
    epoll_set_data_size(
        g_common.head_sendcahche,
        epoll_get_data_size(g_common.head_sendcahche)+size
    );
    if(size<=MAX_TRANS_SIZE) {
        insert_QUEUE_manage2(
        g_common.send_b,
        g_common.head_sendcahche,
        HEAD_LOCAL_COUNT_SIZE,
        buffer,
        size
        );
        sem_post(&g_common.send_b_semaphore);
        return ;
    }

    // split 


    int total_send=epoll_get_data_size(g_common.head_sendcahche);
    //1 head 2 content 



    big_send_start(g_common.split_cache,target,g_common.self_group,total_send);
    insert_QUEUE_manage( g_common.send_b,g_common.split_cache,epoll_get_data_size(g_common.split_cache));
    sem_post(&g_common.send_b_semaphore);

    // 1 
    epoll_ini_data_size_cmd2(g_common.head_sendcahche2,target,RECV_BIG_RUNNING,g_common.self_group);

    //head 1 
    g_common.split_param.buffer[0]=g_common.head_sendcahche2;
    g_common.split_param.buffersize[0]=HEAD_LOCAL_COUNT_SIZE;
    //head 2 
    g_common.split_param.buffer[1]=g_common.head_sendcahche;
    g_common.split_param.buffersize[1]=HEAD_LOCAL_COUNT_SIZE;
    //content
    g_common.split_param.buffer[2]=buffer;
    g_common.split_param.buffersize[2]=MAX_TRANS_SIZE;
    buffer=&buffer[MAX_TRANS_SIZE];
    size=size-MAX_TRANS_SIZE;
    g_common.split_param.sizes=3;

    epoll_set_data_size( 
        g_common.split_param.buffer[0],
        HEAD_LOCAL_COUNT_SIZE+HEAD_LOCAL_COUNT_SIZE+MAX_TRANS_SIZE
    );
    insert_QUEUE_manage3(g_common.send_b,&g_common.split_param);
    sem_post(&g_common.send_b_semaphore);
    int send_size;
    while(size>0){
        send_size=MAX_TRANS_SIZE;
        if (size<MAX_TRANS_SIZE){
            send_size=size;
        }
        g_common.split_param.buffer[1]=buffer;
        g_common.split_param.buffersize[1]=send_size;
        g_common.split_param.sizes=2;
        epoll_set_data_size( 
        g_common.split_param.buffer[0],
        HEAD_LOCAL_COUNT_SIZE+send_size
        );
        buffer=&buffer[send_size];
        size-=send_size;

        insert_QUEUE_manage3(g_common.send_b,&g_common.split_param);
        sem_post(&g_common.send_b_semaphore);
    }

}
u64 normal_gen_index(unchar type,u64 index){
    u64 result=type;
    result=result<<56;
    result+=index;
    return result;
}

