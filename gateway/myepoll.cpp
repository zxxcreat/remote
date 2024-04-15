#include "myepoll.h"
int main(){



    // rsa_all rsa;int esize=10;
    // char encode_data[10];
    // for (int i=0;i<esize;i++){
    //     encode_data[i]=i;
    // }
    // log1("1 \n");
    // rsa_ini(&rsa);
    // log1("2 \n");
    // char encode_data2[4096];char encode_data3[4096];
    // char encode_data4[4096];char encode_data5[4096];
    // log1("3 \n");
    
    // int tttt=rsa_encode_1(rsa.rsa,encode_data,esize,encode_data2,4096);
    // printf("\n %d \n",tttt);
    // tttt=rsa_decode_1(rsa.rsa,encode_data2,4096/8,encode_data3,4096);
    // printf("\n %d \n",tttt);
    
    // for (int i=0;i<esize;i++){
    //     printf("%02x ",encode_data3[i]);
    // }
    // int s1=rsa_public_char(rsa.rsa,encode_data3,4096);
    // printf("\n %d \n",s1);
    // int s2=rsa_private_char(rsa.rsa,encode_data4,4096);
    // printf("\n %d \n",s2);
    
    // int s3=rsa_decode(encode_data2,4096/8,encode_data4,s2,encode_data5,4096);
    // printf("\n %d \n",s3);
    // for (int i=0;i<esize;i++){
    //     printf("%02x ",encode_data5[i]);
    // }

    // return 0;
    //加载lua读脚本
    if (gate_ini()!=0){
        return -1;
    }
    if (ini_epoll()!=0){
        return -1;
    }


    // epoll_event buffer
    int max_event=100;
    epoll_event *events=(epoll_event *)malloc(sizeof(epoll_event)*max_event);
    int epoll_fd=g_gate.epollfd;
    int Socket=g_gate.epollsocket;


    struct sockaddr_in *add4=&g_gate._addr4;
    struct sockaddr_in6  *add6=&g_gate._addr6;
    socklen_t  add4len=sizeof(g_gate._addr4);
    socklen_t  add6len=sizeof(g_gate._addr6);
    int socket2 =0;
    char *ipt=0;
    int ip_sizes=0;
    uint32_t ip_num;uint8_t ip_num2[16];
    epoll_event accept_event;
    accept_event.events=EPOLLIN;
    while(1){
        int result=epoll_wait(epoll_fd,events,max_event,1000*10);
        // 检测心跳包
        g_gate.time_now=times_get_time();
        if (result<0){
            printf("epoll_waite return %d\n",result);
            perror("shmget");
           return  -1;
        }
        for (int i=0;i<result;i++){
            // accept
            if (events[i].data.fd==Socket){

                
                if (g_gate.ip_type==4){
                    socket2 = accept(Socket,(struct sockaddr *)add4, &add4len);
                }else{
                    socket2 = accept(Socket,(struct sockaddr *)add6, &add6len);
                }
                if (socket2<0){
                    printf("accept error\n");
                    continue;
                }

                // 获取 IP 地址的字符串形式
                if (g_gate.ip_type==4){
                    char ip_str[INET_ADDRSTRLEN];
                    memset(ip_str, 0, sizeof(ip_str));
                    inet_ntop(AF_INET, &(add4->sin_addr), ip_str, sizeof(ip_str));
                    //printf("IPv4 address (string): %s\n", ip_str);

                    // 获取 IP 地址的数值形式
                    ip_num = ntohl(add4->sin_addr.s_addr);
                    ipt=(char *)&ip_num;
                    ip_sizes=sizeof(ip_num);
                    //printf("IPv4 address (numeric): %u\n", ip_num);
                }else{
                    // 获取 IP 地址的字符串形式
                    char ip_str2[INET6_ADDRSTRLEN];
                    memset(ip_str2, 0, sizeof(ip_str2));
                    inet_ntop(AF_INET6, &(add6->sin6_addr), ip_str2, sizeof(ip_str2));
                    //printf("IPv6 address (string): %s\n", ip_str2);

                    // 获取 IP 地址的数值形式
                    memcpy(ip_num2, &(add6->sin6_addr.s6_addr), sizeof(ip_num2));
                    ipt=(char *)ip_num2;ip_sizes=16;
                    // printf("IPv6 address (numeric): ");
                    // for (int i = 0; i < 16; i++) {
                    //     printf("%02x", ip_num2[i]);
                    // }
                    // printf("\n");
                }

                // 查看黑白名单 是否让加入
                if (epoll_pre_accept(ipt,ip_sizes)==0){
                    close(socket2);
                    continue;
                }
                accept_event.data.fd=socket2;
                if (epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket2,&accept_event)<0){
                    close(socket2);
                    printf("epoll_ctl int epoll error\n");
                    continue;
                }
                epoll_post_accept(ipt,ip_sizes,socket2);
                continue;
            }
            // receive
            // if (globe_ptr->epoll_data.find(events[i].data.fd)==globe_ptr->epoll_data.end()){
            //     printf("==epoll_data.end() \n");
            //    return  -1;
            // }
            //判断读写
            if (events[i].events & EPOLLIN) {
                // 发生了可读事件
                // 处理读操作
                // 重置心跳包
                 single_socket * new_data=g_gate.epoll_data[events[i].data.fd];
                if (events[i].data.fd !=g_gate.center.socket){
                    //如果不是center
                    deleteNode(&new_data->heart,&g_gate.heart_beat);
                    new_data->heart.data=g_gate.time_now;
                    insertNode(&new_data->heart,&g_gate.heart_beat);
                }
                

                epoll_on_receive(new_data);
            }

           if (events[i].events & EPOLLOUT) {
                // 发生了可写事件
                // 处理写操作
                single_socket * new_data=g_gate.epoll_data[events[i].data.fd];
                if (tcp_send_client(new_data,epoll_fd,NULL, 0)<0){
                    epoll_delete_single_socket(new_data);
                }
            }
            //////////// 
        }
        // 查看要不要踢了某个人
        // 遍历心跳包
        Node* current = g_gate.heart_beat.head;
        Node* tem;
        i64 diff=0;
        while (current != NULL) {
            // 
            diff=g_gate.time_now-current->data;
            if (diff<=TIME_OUT_HEART){
                break;
            }
            // 找到人 踢了
           
            single_socket * tar=(single_socket *)current->ptr;

            current = current->next;
            epoll_delete_single_socket(tar);
        }
    }




    
}
int gate_ini(){
    // 检查4096
    if (sizeof(single_socket)>4096){
        log1("single_socket > 4096 !\n");
        return -1;
    }
    g_gate.tid_index=0;
    g_gate.accept_mode=ACCEPT_MODE_BLACK;

    for (int i=0;i<sizeof(g_gate.cmds)/sizeof(g_gate.cmds[0]);i++){
        g_gate.cmds[i]=NULL;
    }
    // 加载黑名单和白名单




    //随机数初始化
    ini_random_seed();
    ini_pre_random(&g_gate.randoms);
    // 还有什么
    ini_recv_big(&g_gate.big_buffer);
    //初始化param 除了center 都需要初始化这个
    g_gate.params.big_info=&g_gate.big_buffer;
    g_gate.params.deals=g_gate.cmds;

    if (ini_lua_config()!=0){
        return -1;
    }
    // cmd 触发
    g_gate.cmds[RECV_BIG_START]=recv_big_start;
    g_gate.cmds[RECV_BIG_RUNNING]=recv_big_running;
    g_gate.cmds[RECV_BIG_END]=recv_big_end;
    g_gate.cmds[CENTER_0_register_re]=recv_center_register;

    g_gate.cmds[GATEWAT_SEND_CLIENT]=send2client;
    
    // 单独 
    // 转发 设置模式 kick 

    return 0;
}
int ini_lua_config(void){

    //初始化lua
    g_gate.luavm=lua_ini();
    lua_State * tvm=g_gate.luavm;
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

    // 读取
    lua_getglobal(tvm, "gateway_config");
    if (!lua_istable(tvm, -1)){
        log1("没有config这个表\n");
        return -1;
    }




    lua_getfield(tvm, -1, "ip_type");
    if (!lua_isinteger(tvm, -1)){
        log1("没有ip_type这个字段\n");
        return -1;
    }
    g_gate.ip_type=luaL_checkinteger(tvm, -1);
    lua_pop(tvm,1);


    const char *str=NULL;

    lua_getfield(tvm, -1, "ip");
    if (lua_type(tvm, -1)!=LUA_TSTRING){
        log1("ip 字段不是string\n");
        return -1;
    }
    size_t length = 0;
    str=luaL_checklstring(tvm,-1,&length);
    memcpy(g_gate.ipstr,str,length);g_gate.ipstr[length]=0;
    g_gate.ipstr_size=length;
    lua_pop(tvm,1);


    lua_getfield(tvm, -1, "port");
    if (!lua_isinteger(tvm, -1)){
        log1("没有 port 这个字段\n");
        return -1;
    }
    g_gate.port=luaL_checkinteger(tvm, -1);
    lua_pop(tvm,1);

  
   lua_getfield(tvm, -1, "defult_id");
    if (!lua_isinteger(tvm, -1)){
        log1("没有 defult_id 这个字段\n");
        return -1;
    }
    g_gate.defult_id=luaL_checkinteger(tvm, -1);
    lua_pop(tvm,1);



    lua_getfield(tvm, -1, "group_id");
    if (!lua_isinteger(tvm, -1)){
        log1("没有 group_id 这个字段\n");
        return -1;
    }
    g_gate.group_id=luaL_checkinteger(tvm, -1);
    lua_pop(tvm,1);

    lua_getfield(tvm, -1, "key_join");
    if (lua_type(tvm, -1)!=LUA_TSTRING){
        log1("key_join 字段不是string\n");
        return -1;
    }
    length = 0;
    str=luaL_checklstring(tvm,-1,&length);
    memcpy(g_gate.key_join,str,length);
    g_gate.key_join[length]=0;
    g_gate.key_join_size=length;
    lua_pop(tvm,1);



    lua_getfield(tvm, -1, "check_str");
    if (lua_type(tvm, -1)!=LUA_TSTRING){
        log1("check_str 字段不是string\n");
        return -1;
    }
    length = 0;
    str=luaL_checklstring(tvm,-1,&length);
    memcpy(g_gate.check_str,str,length);
    g_gate.check_str[length]=0;
    g_gate.check_str_size=length;
    lua_pop(tvm,1);


    lua_getfield(tvm, -1, "local_ip");
    if (lua_type(tvm, -1)!=LUA_TSTRING){
        log1("check_str 字段不是string\n");
        return -1;
    }
    length = 0;
    str=luaL_checklstring(tvm,-1,&length);
    memcpy(g_gate.local_ip,str,length);
    g_gate.local_ip[length]=0;
    g_gate.local_ip_len=length;
    lua_pop(tvm,1);

    lua_getfield(tvm, -1, "local_port");
    if (!lua_isinteger(tvm, -1)){
        log1("没有 local_port 这个字段\n");
        return -1;
    }
    g_gate.local_port=luaL_checkinteger(tvm, -1);
    lua_pop(tvm,1);
    


    center_config *temc=&g_gate.center;
    unchar self_id=g_gate.group_id;
    if (0!=center_read_config(temc,tvm)){
        log1("center config 读取失败\n");
        return -1;
    }
    if (0!=center_connect(temc,g_gate.local_ip,g_gate.local_port)){
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
    return 0;

}
int ini_epoll(void){
    int Socket = 0;
    if (g_gate.ip_type==4){
        Socket=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    
    if (g_gate.ip_type==6){
        Socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    }
    if (Socket == 0){
        log1(" ip_type 填写出错！\n");
        return -1;
    }


    int opt_val = 1;
    int opt_len = sizeof(int);
    int result=setsockopt(Socket,SOL_SOCKET,SO_REUSEADDR, (&opt_val), opt_len);
    if (result==-1){
        close(Socket);
        return  -1;
    }
    int flag = 1;
    result = setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, &opt_val, opt_len);
    if (result == -1) {
        // 处理错误
    }
    if (g_gate.ip_type == 4) {
        g_gate._addr4.sin_family = AF_INET;
        g_gate._addr4.sin_addr.s_addr = inet_addr(g_gate.ipstr);
        g_gate._addr4.sin_port = htons(g_gate.port);
        
        result = bind(Socket, (struct sockaddr *)&g_gate._addr4, sizeof(g_gate._addr4));
        if (result != 0) {
            sprintf(g_gate.logcache, "Bind failed: %d\n", result);
            log1(g_gate.logcache);
            return -1;
        }
        
        result = listen(Socket, SOMAXCONN);
        if (result != 0) {
            log1("Listen failed\n");
            return -1;
        }
    } else {
        memset(&g_gate._addr6, 0, sizeof(g_gate._addr6));
        g_gate._addr6.sin6_family = AF_INET6;
        inet_pton(AF_INET6, g_gate.ipstr, &g_gate._addr6.sin6_addr);
        g_gate._addr6.sin6_port = htons(g_gate.port);
        
        result = bind(Socket, (struct sockaddr *)&g_gate._addr6, sizeof(g_gate._addr6));
        if (result != 0) {
            sprintf(g_gate.logcache, "Bind failed: %d\n", result);
            log1(g_gate.logcache);
            return -1;
        }
        
        result = listen(Socket, SOMAXCONN);
        if (result != 0) {
            log1("Listen failed\n");
            return -1;
        }
    }


    int epoll_fd=epoll_create(getpid());
    g_gate.epollfd=epoll_fd;
    g_gate.epollsocket=Socket;


    epoll_event accept_event;
    accept_event.events=EPOLLIN;
    accept_event.data.fd=Socket;
     //  add fd socket in epoll
    result=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,Socket,&accept_event);
    if (result<0){
        printf("epoll_ctl fail\n");
       return  -1;
    }



    epoll_event accept_event2;
    accept_event2.events=EPOLLIN;
    accept_event2.data.fd=g_gate.center.socket;
    // 链接center 
    if (epoll_ctl(epoll_fd,EPOLL_CTL_ADD,g_gate.center.socket,&accept_event2)<0){
        close(g_gate.center.socket);
        printf("center 加入epoll失败\n");
        return -1;
    }
    // center 的基础信息 加入
    epoll_post_accept(g_gate.center.ipstr,g_gate.center.ipstr_size,g_gate.center.socket);
    return 0;
}
u64 run_tid(){
    u64 key=g_gate.group_id;
    key=key<<56;key+=g_gate.tid_index;
    g_gate.tid_index++;
    return key;
}
int ini_single_socket(int socket,char *ip,int ip_size,
single_socket ** result,int need_send_buffer){
    *result=(single_socket *)malloc(4096);
    (*result)->socket=socket;
    memcpy((*result)->ipstr,ip,ip_size);
    (*result)->ipstr[ip_size]=0;
    (*result)->tid=run_tid();
    if (need_send_buffer ==1 ){
        (*result)->send_target=-1;
        (*result)->send_rest=ini_CircularBuffer2(4096);
        for (int i=0;i<sizeof((*result)->send_list)/sizeof((*result)->send_list[0]);i++){
            (*result)->send_list[i]=NULL;
        }
        (*result)->send_list[0]=ini_CircularBuffer2(4096);
    }

    (*result)->re_tail=0;
    (*result)->re_max=CACHE_NORMAL;
    (*result)->heart.ptr=result;
    // 先转给默认id
    (*result)->trans_server=g_gate.defult_id;
    
    (*result)->socket_state=FD_CHECK_SHA256;
    return 0;
}
int ini_single_128(single_socket * result){
    //result->send_buffer_128=ini_CircularBuffer2(128*1024);
    return 0;
}
int delete_single(single_socket * result){
    if (result->send_rest){
        free(result->send_rest);
    }
    for (int i=0;i<sizeof(result->send_list)/sizeof(result->send_list[0]);i++){
        if (result->send_list[i]!=NULL){
            free(result->send_list[i]);
        }
    }
    free(result);
}
void epoll_post_accept(char *ip,int ip_size,int socket_fd){
    single_socket *new_data;
    if (socket_fd==g_gate.center.socket){
        ini_single_socket(socket_fd,ip,ip_size,&new_data,0);
        
    }else{

        ini_single_socket(socket_fd,ip,ip_size,&new_data,1);
        // 心跳包
         new_data->heart.data=g_gate.time_now;
        insertNode(&new_data->heart,&g_gate.heart_beat);
    }

    g_gate.epoll_data[socket_fd]=new_data;
    g_gate.tid2socket[new_data->tid]=socket_fd;
}
int epoll_pre_accept(char *ip ,int ip_size){
    
    if (g_gate.center.is_connect!=CENTER_CONNECT_READY){
        // center 还没有准备好
        return 0;
    }
    int is_allowed=1;
    
    if (g_gate.accept_mode==ACCEPT_MODE_BLACK){
        
        if (g_gate.ip_type==4){
            uint32_t ip32=0;
            memcpy(&ip32,ip,4);
            if (g_gate.black_ip4_list.find(ip32)!=g_gate.black_ip4_list.end()){
                is_allowed=0;
            }
        }else{
            // ipv6 封前缀64位
            u64 ipv6_first;
            memcpy(&ipv6_first,ip,sizeof(u64));
            if (g_gate.black_ip6_list.find(ipv6_first)!=g_gate.black_ip6_list.end()){
                is_allowed=0;
            }

        }
    }

    if (g_gate.accept_mode==ACCEPT_MODE_WRILT){
        is_allowed=0;
        
        if (g_gate.ip_type==4){
            uint32_t ip32=0;
            memcpy(&ip32,ip,4);
            if (g_gate.white_ip4_list.find(ip32)!=g_gate.white_ip4_list.end()){
                is_allowed=1;
            }
        }else{
            // ipv6 封前缀64位
            u64 ipv6_first;
            memcpy(&ipv6_first,ip,sizeof(u64));
            if (g_gate.white_ip6_list.find(ipv6_first)!=g_gate.white_ip6_list.end()){
                is_allowed=1;
            }
            
        }
    }
    if (g_gate.accept_mode==ACCEPT_MODE_NULL){
        is_allowed=1;
    }
    // 1 允许 0 不允许
    return is_allowed;
}
void epoll_delete_single_socket(single_socket *data){



    close(data->socket);
    g_gate.epoll_data.erase(data->socket);
    g_gate.tid2socket.erase(data->tid);
    epoll_ctl(g_gate.epollfd, EPOLL_CTL_DEL,data->socket, NULL);

    if (data->socket==g_gate.center.socket){
        
        g_gate.center.is_connect=0;
        center_config *center=&g_gate.center;
        while(1){
            if (0!=center_connect(center,g_gate.local_ip,g_gate.local_port)){
                log1("center config 连接\n");
                continue ;
            }
            if (0!=center_send_password(center)){
                log1("center config 密码失败\n");
                continue;
            }
            if (0!=center_register(center,g_gate.group_id)){
                log1("center config 注册失败\n");
                continue;
            }
            while(1){
                epoll_event accept_event2;
                accept_event2.events=EPOLLIN;
                accept_event2.data.fd=g_gate.center.socket;
                // 链接center 
                if (epoll_ctl(g_gate.epollfd,EPOLL_CTL_ADD,g_gate.center.socket,&accept_event2)<0){
                    close(g_gate.center.socket);
                    printf("center 加入epoll失败\n");
                    continue;
                }
                break;
            }
           
            // center 的基础信息 加入
            epoll_post_accept(g_gate.center.ipstr,g_gate.center.ipstr_size,g_gate.center.socket);
            break;
        }   
    }else{
        // 不是center 先删除心跳包
        deleteNode(&data->heart,&g_gate.heart_beat);
        if (data->socket_state== FD_CHECK_FINISH){
            // 需要告诉默认服务器 如果登陆服务器 某个b断开了
            epoll_ini_data_size_cmd2(g_gate.send_cache,data->trans_server,SOCKET_KEY_OFF,g_gate.group_id);
            char *tidc=(char *)data->tid;
            memcpy(&g_gate.send_cache[epoll_get_data_size(g_gate.send_cache)],tidc,sizeof(data->tid));
            epoll_set_data_size(g_gate.send_cache,epoll_get_data_size(g_gate.send_cache)+sizeof(data->tid));
            tcp_send_centers(&g_gate.center,g_gate.send_cache,epoll_get_data_size(g_gate.send_cache));
            
        }

    }
    delete_single(data);
}
void epoll_on_receive(single_socket *data){
    //玩家的数据必须得是小端  客户端逻辑
    
    int rest=data->re_max-data->re_tail;
    char *buffer_head=&data->re_buffer[data->re_tail];
    int result=recv(data->socket,buffer_head,rest,MSG_DONTWAIT);
    switch (result) {
        case -1:
            switch (errno) {
                case EAGAIN:
                    //printf("资源暂时不可用，请稍后重试。\n");
                    break;
                case EINTR:
                    //printf("操作被中断，请重试。\n");
                    break;
                case EBADF:
                    //printf("无效的文件描述符。\n");
                    break;
                case EFAULT:
                    //printf("无效的缓冲区地址。\n");
                    break;
                case ENOTCONN:
                    //printf("套接字未连接。\n");
                    break;
                case EINVAL:
                    //printf("无效的参数。\n");
                    break;
                case ENOMEM:
                    //printf("内存不足。\n");
                    break;
                case ETIMEDOUT:
                    //printf("连接超时。\n");
                    break;
                default:
                    //printf("其他接收错误。错误代码：%d，错误消息：%s\n", errno, strerror(errno));
                    break;
            }
            break;
        case 0:
            //printf("连接已被对方关闭。\n");
            epoll_delete_single_socket(data);
            return ;
            break;
        default:
            //printf("成功接收到数据。接收到的字节数：%d\n", result);
            // 处理接收到的数据
            data->re_tail+=result;
            break;
    }
    // 头
    int copy_index=0;
    // 还有多少字节需要处理
    int rest_re=data->re_tail;
    //
    char * target_bufferptr;
    while(1){
        if (rest_re <sizeof(uint32_t)){
            break ;
        }
        target_bufferptr=&data->re_buffer[copy_index];
        int got_size=epoll_get_data_size(target_bufferptr);
        if (got_size<sizeof(uint32_t)){
            epoll_delete_single_socket(data);
            return ;
        }
        if (got_size<rest_re){
            break ;
        }
        if (got_size>MAX_TRANS_SIZE){
            epoll_delete_single_socket(data);
            return ;
        }
        // 区分给谁的socket
        if (data->socket==g_gate.center.socket){
            //center 来的消息
            deal_center_msg(data,&target_bufferptr[sizeof(uint32_t)],got_size-sizeof(uint32_t));
        }else{
            int r=deal_client_msg(data,&target_bufferptr[sizeof(uint32_t)],got_size-sizeof(uint32_t));
            if (r!=0){
                epoll_delete_single_socket(data);
                return;
            }
        }
        copy_index+=got_size;
        rest_re-=got_size;
    }
    if (copy_index!=0){
        for (int i=copy_index;i<data->re_tail;i++){
            data->re_buffer[i-copy_index]=data->re_buffer[i];
        }
        data->re_tail=rest_re;
    }
}
void deal_center_msg(single_socket *data,char *buffer,int size){
    // target_server + cmd+ send_server +data 删除 target和cmd 只留下data
    unchar cmd=buffer[1];
    int cmdd=cmd;
    if (g_gate.cmds[cmd] ==NULL){
        LOGS("gateway 没有命令 %d \n",cmdd);
        return ;
    }else{
        g_gate.params.buffer=&buffer[LOCAL_SEND_TCS];
        g_gate.params.buffer_size=size-LOCAL_SEND_TCS;
        g_gate.params.send_group=(unchar)buffer[2];
        if (g_gate.params.buffer_size<=0){
            return ;
        }
        g_gate.cmds[cmd](&g_gate.params);
    }
}
int deal_client_msg(single_socket *data,char *buffer,int size){
    if (size<=0){
        return -1;
    }
    //size + 加密数据 
    // 加密数据 = size + 内容 
    switch (data->socket_state)
    {
    case FD_CHECK_SHA256:{
        if (size !=32){
            return -1;
        }
        char tem1[50];char tem2[50];
        unsigned char sha_result1[50];
        unsigned char sha_result2[50];
        i64 now=g_gate.time_now/1000;
        i64 sign=now/10;

        int num1=sprintf(tem1,g_gate.key_join,sign);
        int num2=sprintf(tem2,g_gate.key_join,sign-1);

        SHA256((unchar *)tem1,num1,(unchar *)sha_result1);
        SHA256((unchar *)tem2,num2,(unchar *)sha_result2);

        if (
            !(memcmp((char *)sha_result1, buffer, 32)==0||
            memcmp((char *)sha_result2, buffer, 32)==0)
        )
        {
            
            return -1; 
        }
        data->socket_state=FD_CHECK_SECRET;
        log1("check 256 success\n");
        /* code */
        break;
    };
        
        
    case FD_CHECK_SECRET:{
        if (524!=size){
            return -1;
        }
        //传给玩家的是 size+ 加密数据
        // 加密是size+ data 
        // 总的就是 size + size+data
        // 传给加密后的数字 32位aes和16位iv
        char content[100];
        int write_index=4;
        epoll_set_data_size(content,write_index);
        get_random(&g_gate.randoms,&content[write_index],32+16);
        write_index=write_index+32+16;
        epoll_set_data_size(content,write_index);
        AES_set_decrypt_key((unchar *)&content[4] ,256,&data->aes_key_decode);
        AES_set_encrypt_key((unchar *)&content[4] ,256,&data->aes_key_encode);
        memcpy(data->aes256_iv16_decode,&content[4+32],16);
        memcpy(data->aes256_iv16_encode,&content[4+32],16);

        char out[600];
        int res=rsa_encode(content,write_index,
        buffer,size,
        &out[sizeof(uint32_t)],512);
        if (res !=512){
            return -1;
        }
        // 发送
        //.................
        epoll_set_data_size(out,512+sizeof(uint32_t));
        if (tcp_send_client(data,g_gate.epollfd,out,res)!=0){
            return -1;
        }
        
        data->socket_state=FD_CHECK_ENCODE;
        /* code */
        break;
    };
        
    case FD_CHECK_ENCODE:{
        AES_cbc_encrypt(
        (unsigned char *)buffer,
        (unsigned char *)g_gate.send_cache,
        size,
        &data->aes_key_decode,
        (unsigned char *)data->aes256_iv16_decode,AES_DECRYPT
        );
        // size +加密后的大小？ 
        // 加密 = size + 大小?
        uint32_t size_decode=0;
        char * size_decodec=(char *)&size_decode;
        memcpy(size_decodec,g_gate.send_cache,sizeof(uint32_t));
        if (size_decode<sizeof(uint32_t)|| size_decode>MAX_TRANS_SIZE){
            return -1;
        }
        char *buff2=&g_gate.send_cache[sizeof(uint32_t)];
        size_decode=size_decode-sizeof(uint32_t);
        if (size_decode<10 ||size_decode>50){
            return -1;
        }
        unchar ensure;
        for (int i=0;i<10;i++){
            ensure=buff2[i];
            if (ensure!=i){
                return -1;
            }
        }
        data->socket_state=FD_CHECK_FINISH;
        /* code */
        break;
    }
        
    case FD_CHECK_FINISH:
        /* code */
        // 解密后发给center
        {
            epoll_ini_data_size_cmd2(g_gate.send_cache,data->trans_server,SOCKET_KEY_TRANS,g_gate.group_id);
            // 添加临时id
            memcpy(&g_gate.send_cache[epoll_get_data_size(g_gate.send_cache)],&data->tid,sizeof(data->tid));
            epoll_set_data_size(g_gate.send_cache,epoll_get_data_size(g_gate.send_cache)+sizeof(data->tid));
            AES_cbc_encrypt(
            (unsigned char *)buffer,
            (unsigned char *)g_gate.send_cache2,
            size,
            &data->aes_key_decode,
            (unsigned char *)data->aes256_iv16_decode,AES_DECRYPT
            );
            // 解密后获得大小
            int decode_size=epoll_get_data_size(g_gate.send_cache2);
            if (decode_size<=sizeof(uint32_t)|| decode_size>MAX_TRANS_SIZE){
                return -1;
            }
            // memcpy(
            // &g_gate.send_cache[epoll_get_data_size(g_gate.send_cache)],
            // &g_gate.send_cache2[sizeof(uint32_t)],
            // decode_size-sizeof(uint32_t)
            // );
            memcpy(
            &g_gate.send_cache[epoll_get_data_size(g_gate.send_cache)],
            g_gate.send_cache2,
            decode_size
            );
            epoll_set_data_size(
                g_gate.send_cache,
                epoll_get_data_size(g_gate.send_cache)+decode_size
            );
            // cmd 是哪个发送给哪个设置决定的
            //  如果崩溃了 如何删除所有的
            if (tcp_send_centers(&g_gate.center,g_gate.send_cache,epoll_get_data_size(g_gate.send_cache))<0){
                return -1;
            }
        }
        
        break;
    
    default:
        break;
    }
    return 0;

}
int  tcp_send_client(single_socket *data,int epollfd,
char *buffer,int size){
    //buffer =size+data
    //缓存前面再加个大小？ 因为不知道发送了多少 如果大小的第一位是1 那么就是剩余的
    int socket=data->socket;
    int result=0;
    if (data->send_target<0){
        if (size<=0){
            // 可能是可以写了 发送信号  但是如果<0 就是没有存储的可发送数据
            return 0;
        }
        char *tbuffer=buffer;
        int rest=size;
        int repeat_time=0;
        while(1){
            result=send(socket, tbuffer, rest, MSG_DONTWAIT);
            if (result ==0){
                // 连接已关闭
                return -1;
            }
            if (result>0){
                rest-=result;
                tbuffer=&tbuffer[result];
                continue;
            }
            if (rest<=0){
                break;
            }
            if (result<0){
                switch (errno) {
                    case EAGAIN:{
                        // 当send返回EAGAIN时，表示当前无法发送更多数据，需要等待后续的可写事件
                        struct epoll_event event;
                        event.events = EPOLLIN|EPOLLOUT;
                        event.data.fd = socket;
                        if (epoll_ctl(epollfd, EPOLL_CTL_MOD, socket, &event) == -1) {
                            perror("epoll_ctl");
                            return -1;
                        }  
                        data->send_target++;
                        // 存一下每发完的放到缓存里面 每次先发没法完的


                        uint32_t urest=rest+sizeof(uint32_t);
                        insert_CircularBuffer(data->send_rest,(char *)urest,sizeof(uint32_t));
                        insert_CircularBuffer(data->send_rest,tbuffer,rest);
                        


                        return 0;
                        break;
                    }
                    case EINTR:
                        {
                            // 当send返回EINTR时，表示发送操作被信号中断，可以选择重试发送
                            // 如果选择重试，可以在此处添加相应的重试逻辑
                            repeat_time++;
                            // 重试的次数太多了 直接放入可选队列把
                            if (repeat_time>3){
                                struct epoll_event event;
                                event.events = EPOLLIN|EPOLLOUT;
                                event.data.fd = socket;
                                if (epoll_ctl(epollfd, EPOLL_CTL_MOD, socket, &event) == -1) {
                                    perror("epoll_ctl");
                                    return -1;
                                }
                                data->send_target++;
                                uint32_t urest=rest+sizeof(uint32_t);
                                insert_CircularBuffer(data->send_rest,(char *)urest,sizeof(uint32_t));
                                insert_CircularBuffer(data->send_rest,tbuffer,rest);
                                return 0;
                            }
                            break;
                        }
                        
                    case EPIPE:
                        // 当send返回EPIPE时，表示写入了一个已关闭的连接
                        // 可以在此处添加相应的处理逻辑
                    case ECONNRESET:
                        // 当send返回ECONNRESET时，表示连接被对端重置
                        // 可以在此处添加相应的处理逻辑
                        {
                            return -1;
                        }
                    default:
                        {
                            // 其他错误，可以根据实际需求进行相应的处理
                            perror("send");
                            return -1;
                        }
                        
                }
            }

        }
        
    }
    if (data->send_target>=0){
        // 判断一下data->send_target 能不能放下
        
        if (size>0){
            int rest=data->send_list[data->send_target]->maxSize-data->send_list[data->send_target]->saved_size;
            if (rest>size){
                if (data->send_target>=BASE_SIZE_MAX_INDEX){
                    // 受不了了 直接删除这个socket
                    return -1;
                }
                data->send_target++;
                i64 need_size=1<<data->send_target;
                need_size=need_size*BASE_SIZE_SEND;
                data->send_list[data->send_target]=ini_CircularBuffer2(need_size);
            }
            insert_CircularBuffer(data->send_list[data->send_target],buffer,size);
        }
        int sizes_c=0;
        char *tchar=g_gate.send_cache;
        //先看一下rest
        int repeat_time=0;
        
        
        pop_CircularBuffer(data->send_rest,&tchar,&sizes_c);
        if (sizes_c >0){
            tchar=&tchar[sizeof(uint32_t)];
            sizes_c=sizes_c-sizeof(uint32_t);
            int indexs=0; int max_index=sizes_c;
            
            
            while(1){
                result=send(socket, tchar, sizes_c, MSG_DONTWAIT);
                if (result ==0){
                    // 连接已关闭
                    return -1;
                }
                if (result>0){
                    indexs+=result;
                    continue;
                }
                if (indexs>=max_index){
                    break;
                }
                if (result<0){
                    switch (errno) {
                        case EAGAIN:{
                            // 当send返回EAGAIN时，表示当前无法发送更多数据，需要等待后续的可写事件
                            int rests=max_index-indexs;
                            reset_CircularBuffer(data->send_rest);
                            uint32_t urest=rests+sizeof(uint32_t);
                            insert_CircularBuffer(data->send_rest,(char *)urest,sizeof(uint32_t));
                            insert_CircularBuffer(data->send_rest,&tchar[indexs],rests);
                            return 0;
                            break;
                        }
                        case EINTR:
                            {
                                // 当send返回EINTR时，表示发送操作被信号中断，可以选择重试发送
                                // 如果选择重试，可以在此处添加相应的重试逻辑
                                repeat_time++;
                                // 重试的次数太多了 直接放入可选队列把
                                if (repeat_time>3){
                                    int rests=max_index-indexs;
                                    reset_CircularBuffer(data->send_rest);
                                    uint32_t urest=rests+sizeof(uint32_t);
                                    insert_CircularBuffer(data->send_rest,(char *)urest,sizeof(uint32_t));
                                    insert_CircularBuffer(data->send_rest,&tchar[indexs],rests);
                                    return 0;
                                }
                                break;
                            }
                            
                        case EPIPE:
                            // 当send返回EPIPE时，表示写入了一个已关闭的连接
                            // 可以在此处添加相应的处理逻辑
                        case ECONNRESET:
                            // 当send返回ECONNRESET时，表示连接被对端重置
                            // 可以在此处添加相应的处理逻辑
                            {
                                return -1;
                            }
                        default:
                            {
                                // 其他错误，可以根据实际需求进行相应的处理
                                perror("send");
                                return -1;
                            }
                            
                    }
                }
            }
        }
        
        
        reset_CircularBuffer(data->send_rest);

        
        
        sizes_c=0;tchar=g_gate.send_cache;
        int rest=0;
        for (int i=0;i<=BASE_SIZE_MAX_INDEX;i++){
            if (data->send_list[i]==NULL){
                continue;
            }
            while(1){
                if (data->send_list[i]->saved_size==0){
                    if (i!=0){
                        free(data->send_list[i]);
                        data->send_list[i]=NULL;
                        break;
                    }
                }
                
                pop_CircularBuffer(data->send_list[i],&tchar,&sizes_c);
                if (sizes_c==0){
                    return -1;
                }
                rest=sizes_c;
                repeat_time=0;
                while(1){
                    // 发送
                    result=send(socket, tchar, sizes_c, MSG_DONTWAIT);

                    if (result ==0){
                        // 连接已关闭
                        return -1;
                    }
                    if (result>0){
                        rest-=result;
                        tchar=&tchar[result];
                        continue;
                    }
                    if (rest<=0){
                        break;
                    }
                    if (result<0){
                        switch (errno) {
                            case EAGAIN:{
                                // 当send返回EAGAIN时，表示当前无法发送更多数据，需要等待后续的可写事件
                                uint32_t urest=rest+sizeof(uint32_t);
                                insert_CircularBuffer(data->send_rest,(char *)urest,sizeof(uint32_t));
                                insert_CircularBuffer(data->send_rest,tchar,rest);
                                return 0;
                                break;
                            }
                            case EINTR:
                                {
                                    // 当send返回EINTR时，表示发送操作被信号中断，可以选择重试发送
                                    // 如果选择重试，可以在此处添加相应的重试逻辑
                                    repeat_time++;
                                    // 重试的次数太多了 直接放入可选队列把
                                    if (repeat_time>3){
                                        uint32_t urest=rest+sizeof(uint32_t);
                                        insert_CircularBuffer(data->send_rest,(char *)urest,sizeof(uint32_t));
                                        insert_CircularBuffer(data->send_rest,tchar,rest);
                                        return 0;
                                    }
                                    perror("send");
                                    break;
                                }
                                
                            case EPIPE:
                                // 当send返回EPIPE时，表示写入了一个已关闭的连接
                                // 可以在此处添加相应的处理逻辑
                            case ECONNRESET:
                                // 当send返回ECONNRESET时，表示连接被对端重置
                                // 可以在此处添加相应的处理逻辑
                                {
                                    return -1;
                                }
                            default:
                                {
                                    // 其他错误，可以根据实际需求进行相应的处理
                                    perror("send");
                                    return -1;
                                }
                                
                        }
                    }
                }
            }

        }
        // 最后判断是不是所有的都是空的
        u64 is_empty=0;
        for (int i=0;i<=BASE_SIZE_MAX_INDEX;i++){
            is_empty+=(u64)data->send_list[i];
        }
        if (is_empty==0){
            struct epoll_event event;
            event.events = EPOLLIN;
            event.data.fd = socket;
            if (epoll_ctl(epollfd, EPOLL_CTL_MOD, socket, &event) == -1) {
                perror("epoll_ctl");
                return -1;
            } 
            data->send_target=-1;
        }

    }
    return -1;

}


int  tcp_send_centers(center_config *center,char *buffer,int size){
    int result=send(center->socket,buffer,size,0);
    if (result<=0){
       epoll_delete_single_socket(g_gate.epoll_data[center->socket]);
    }
    return 0;
}
void * recv_center_register(commend_triggers_param *data){
    g_gate.center.is_connect=CENTER_CONNECT_READY;
}
void * send2client(commend_triggers_param *data){
    // 加密
    if (data->buffer_size<=sizeof(uint32_t)){
        return NULL;
    }
    // size +tar+cmd+selfid 已经删除 还剩下数据  是size+data
    //0-3   8 8 8 8 8 8 8 8  data
    // num  keys 
    uint32_t number=0;
    memcpy(&number,data->buffer,sizeof(number));
    int total_size_min=sizeof(number)+number*sizeof(u64);
    if (data->buffer_size<=total_size_min){
        return NULL;
    }
    u64 tid_single=0;
    int index_tid=0;
    for (int i=0;i<number;i++){
        index_tid=i*sizeof(u64)+sizeof(uint32_t);
        memcpy(&tid_single,&data->buffer[index_tid],sizeof(u64));
        if (g_gate.tid2socket.find(tid_single)==g_gate.tid2socket.end()){
            continue;
        }
        single_socket * info=g_gate.epoll_data[g_gate.tid2socket[tid_single]];
        send2client_single(info,&data->buffer[total_size_min],data->buffer_size-total_size_min);
    }
    return NULL;

}
int send2client_single(single_socket *data,char *buffer,int size){
    // size  发送给玩家的每次大小肯定小于1350？
    // 必须的
    if (size<=0 || size>=MAX_SEND_SIZE){
        return -1;
    }
    // 转发的没有头head 的size 需要加上size+ size+data
    
    // 加密后加个size
    int send_size=size+sizeof(uint32_t);
    uint32_t encode_size=send_size/16;
    if ((send_size%16) !=0){
        encode_size++;
    }
    encode_size=encode_size*16;
    
    // 密文中的大小设置一下
    memcpy(&g_gate.send_cache2[sizeof(uint32_t)],buffer,size);
    epoll_set_data_size(g_gate.send_cache2,send_size);

    AES_cbc_encrypt(
        (unchar *)buffer,
        (unchar *)g_gate.send_cache2,
        send_size,
        &data->aes_key_encode,
        (unchar *)data->aes256_iv16_encode,
        AES_ENCRYPT
    );
    // 加密后的大小设置一下
    memcpy(g_gate.send_cache,g_gate.send_cache2,encode_size);
    epoll_set_data_size(g_gate.send_cache,encode_size+sizeof(uint32_t));
    tcp_send_client(data,g_gate.epollfd,g_gate.send_cache,epoll_get_data_size(g_gate.send_cache));
    return 0;
}