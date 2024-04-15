#include "center_server.h"

globe_center g_center; 
int main(){

    if (ini_center_server() ==-1){
        return -1;
    }
    int Socket=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Socket == 0){
        log1(" Socket 填写出错！\n");
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
        close(Socket);
        return  -1;
    }

    g_center._addr4.sin_family = AF_INET;
    g_center._addr4.sin_addr.s_addr = inet_addr(g_center.center.ipstr);
    g_center._addr4.sin_port = htons(g_center.center.port);
    result=bind(Socket, (sockaddr *)&g_center._addr4, sizeof(g_center._addr4));
    if (result!=0){
        sprintf(g_center.cache,"bind fail %d\n",result);
        log1(g_center.cache);
        return  -1;
    }
    result=listen(Socket, SOMAXCONN);
    if (result!=0){
        log1("listen fail\n");
        return  -1;
    }

    int epoll_fd=epoll_create(getpid());
    g_center.epollfd=epoll_fd;
    g_center.socket=Socket;


    epoll_event accept_event;
    accept_event.events=EPOLLIN;
    accept_event.data.fd=Socket;
     //  add fd socket in epoll
    result=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,Socket,&accept_event);
    if (result<0){
        printf("epoll_ctl fail\n");
       return  -1;
    }
     int max_event=100;
    epoll_event *events=(epoll_event *)malloc(sizeof(epoll_event)*max_event);
    
     struct sockaddr_in *add4=&g_center._addr4;
    struct sockaddr_in6  *add6=&g_center._addr6;
    unsigned int lenth=sizeof(g_center._addr4);
    log1("启动成功！  \n");
    while(1){
        result=epoll_wait(epoll_fd,events,max_event,1000*10);
        g_center.time_now=times_get_time();
        if (result<0){
            printf("epoll_waite return %d\n",result);
            perror("shmget");
           return  -1;
        }
        for (int i=0;i<result;i++){
        // accept
            if (events[i].data.fd==Socket){
                int socket2 = accept(Socket,(struct sockaddr *)add4, &lenth);
                if (socket2<0){
                    printf("accept error\n");
                    continue;
                }
                accept_event.data.fd=socket2;
                if (epoll_ctl(epoll_fd,EPOLL_CTL_ADD,socket2,&accept_event)<0){
                    close(socket2);
                    printf("epoll_ctl int epoll error\n");
                    continue;
                }
                // 初始化
                center_on_accept(&accept_event);
                continue;
            }
            center_on_receive(&events[i]);
        }
    }

    return 0;
}


int ini_center_server(){
    //初始化lua
    g_center.luavm=lua_ini();
    lua_State * tvm=g_center.luavm;
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
    if (0!=center_read_config(&g_center.center,tvm)){
        log1("center config 读取失败\n");
        return -1;
    }
    for (int i=0;i<sizeof(g_center.server)/sizeof(g_center.server[0]);i++){
        g_center.server[i]=NULL;
    }
    g_center.cmds[CENTER_0_register]=center_cmd_CENTER_0_register;
    return 0;
}

servers_single * ini_servers_single(int Socket){
    char *new_buff=(char *)malloc(4096);
    servers_single *server=(servers_single *)new_buff;
    server->socket=Socket;
    server->tail=0;
    server->register_id=0;
    server->buffer=&new_buff[sizeof(servers_single)];
    server->buffer_size=4096-sizeof(servers_single);
    server->state=S_STATE_UNPROMISE;
    return server;
}
void delete_servers_single(servers_single *data){
    free(data);
}
int center_on_accept(epoll_event * data){
    int fd=data->data.fd;
    data->data.ptr=ini_servers_single(fd);
    if (epoll_ctl(g_center.epollfd, EPOLL_CTL_MOD, fd, data) < 0) {
        perror("epoll_ctl EPOLL_CTL_MOD failed");
    }
    return 0;
}

int center_on_receive(epoll_event * data){
    // socket 找到对应的id
    
    servers_single * info=(servers_single *)data->data.ptr; 
    int socket1=info->socket;
    int result;
 
    result=recv(socket1,&info->buffer[info->tail],info->buffer_size-info->tail,0);
    while(1){
        if (info->state == S_STATE_UNPROMISE){
            if (result<=0){
                printf("%d error code 接受出错\n",result);
                center_close(data);
                return 0;
            }
            info->tail+=result;
            if (info->tail<SHA256_DIGEST_LENGTH){
                return 0;
            }



            char tem1[50];char tem2[50];
            unsigned char sha_result1[50];
            unsigned char sha_result2[50];
            i64 now=g_center.time_now/1000;
            i64 sign=now/10;

            int num1=sprintf(tem1,g_center.center.password,sign);
            int num2=sprintf(tem2,g_center.center.password,sign-1);

            SHA256((unchar *)tem1,num1,(unchar *)sha_result1);
            SHA256((unchar *)tem2,num2,(unchar *)sha_result2);

            if (
                !(memcmp((char *)sha_result1, info->buffer, SHA256_DIGEST_LENGTH)==0||
                memcmp((char *)sha_result2, info->buffer, SHA256_DIGEST_LENGTH)==0)
            )
            {
                center_close(data);
                return 0; 
            }
            info->state =S_STATE_CHECKED;
            int unprocessed_length = info->tail - SHA256_DIGEST_LENGTH;
            memmove(info->buffer, &info->buffer[SHA256_DIGEST_LENGTH], unprocessed_length);
            info->tail = unprocessed_length;
            continue;
        }
        if (info->tail<=sizeof(uint32_t)){
            break;
        }
        uint32_t need_size=epoll_get_data_size(info->buffer);
        if (need_size<info->tail){
            break ;
        }
        // 大于 需要deal
        if (deal_servers_cmd(info)==-1){
            center_close(data);
        }
        int unprocessed_length = info->tail - need_size;
        memmove(info->buffer, &info->buffer[need_size], unprocessed_length);
        info->tail = unprocessed_length;
    }



    return 0;

}
int deal_servers_cmd(servers_single * data){
    //  看看target
    // size 4 +target 1 +cmd 1 +send 1
    char *buffer=data->buffer;
    int size=epoll_get_data_size(buffer);
    unchar target=buffer[4];
    if (target == g_center.center.group_id){
        unchar cmd=buffer[5];
        int cmdd=cmd;
        if (g_center.cmds[cmd] ==NULL){
            sprintf(g_center.cache,"没有这个cmd %d \n",cmdd);
            log1(g_center.cache);
            return 0;
        }else{
            g_center.params.buffer=&buffer[LOCAL_SEND_TCS+LOCAL_SEND_SIZE];
            g_center.params.buffer_size=size-LOCAL_SEND_TCS-LOCAL_SEND_SIZE;
            g_center.params.send_group=(unchar)buffer[6];
            // 借用big的指针
            g_center.params.big_info=data;

            if (g_center.params.buffer_size<0){
                return 0;
            }
            g_center.cmds[cmd](&g_center.params);
        }
        return 0;
    }
    // 需要转发
    if (g_center.server[target]==NULL){
        return 0;
    }
    if (epoll_tcp_send(g_center.server[target],buffer,size)==-1){
        return -1;
    }

    return 0;
}
int center_close(epoll_event * data){

    servers_single * info=(servers_single *)data->data.ptr; 
    int state=info->state;
    int socket=info->socket;
    unchar id=info->register_id;
    if (id !=0){
        g_center.server[id]=NULL;
    }
    epoll_ctl(g_center.epollfd, EPOLL_CTL_DEL,socket, NULL);
    close(data->data.fd);
    delete_servers_single(info);
}
void * center_cmd_CENTER_0_register(commend_triggers_param *data){
    unchar group=data->send_group;


    if ( g_center.server[group]!=NULL){
        sprintf(g_center.cache,"%d 服务器已经被注册了！ \n",(int)group);
        log1(g_center.cache);
        return NULL;
    }
    servers_single *info=(servers_single *)g_center.params.big_info;
    g_center.server[group]=info->socket;
    info->register_id=group;
    // 发送给 对面表示成功了
    epoll_ini_data_size_cmd2(g_center.cache,group,CENTER_0_register_re,
    g_center.center.group_id);
    epoll_tcp_send(g_center.server[group],g_center.cache,epoll_get_data_size(g_center.cache));
    
    sprintf(g_center.cache,"%d 服务器注册成功 \n",(int)group);
    log1(g_center.cache);
    return NULL;
}