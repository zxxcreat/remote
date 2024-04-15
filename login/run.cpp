#include "run.h"
globe_common *g_run;
globe_common2 g_data;
int runing_on_start(void *data){
    g_run=(globe_common *)data;
    // 基础的命令
    g_run->cmds[SOCKET_KEY_TRANS]=recv_SOCKET_KEY_TRANS;
    // 离线了 删除缓存
    g_run->cmds[SOCKET_KEY_OFF]=recv_SOCKET_KEY_OFF;
    

    //////注册玩家cmd 有些是二进制数据 有些是json 或者protobuf数据
    register_player_cmd_func(LOGIN_REGISTER,cmd2_LOGIN_REGISTER);
    register_player_cmd_func(LOGIN_LOGININ,cmd2_LOGIN_LOGININ);


    //login  is exit ? no got
    for (int i=0;i<sizeof(g_data.cmds)/sizeof(g_data.cmds[0]);i++){
        g_data.cmds[i]=NULL;
    }
    // 查找数据库文件 没有就创建
    char path[100];
    sprintf(path,"./database/%lld/player.db", g_run->config.servers_id);

    int rc = sqlite3_open(path, &g_data.db);
    if (rc != SQLITE_OK) {
        return -1;
    }
    char * sql1="SELECT name FROM sqlite_master WHERE type='table' AND name='accounts';";
    rc = sqlite3_prepare_v2(g_data.db, sql1, -1, &g_data.stmt, 0);
    if (rc != SQLITE_OK) {
        printf("SQL error: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return -1;
    }
    rc = sqlite3_step(g_data.stmt);
    sqlite3_finalize(g_data.stmt);
    if (rc == SQLITE_DONE) {
        // 没有表
        char *sql_create;
        // 密码的sha +时间 sha256
        sql_create = "CREATE TABLE accounts("  \
        // md5 前八为
        "accounthash   INTEGER UNSIGNED BIGINT PRIMARY KEY NOT NULL,"\
        "account       BLOB     NOT NULL," \
        "password256   BLOB     NOT NULL," \
        "password_salt INTEGER  NOT NULL," \
        // 玩家拥有的guid
        "guid1      INTEGER UNSIGNED BIGINT,"\

        "guid2      INTEGER UNSIGNED BIGINT,"\

        "guid3      INTEGER UNSIGNED BIGINT,"\
        ")";
        rc=sqlite3_exec(g_data.db,sql_create,t_callbackss,0,NULL);
        if( rc != SQLITE_OK ){
            printf("SQL error: %s\n", sqlite3_errmsg(g_data.db));
            //sqlite3_free(g_data.zErrMsg);
            return -1;
        }
        
    }
    //查询account 把所有账号全放到内存中 避免频繁io
    sql1="SELECT accounthash FROM accounts";

    rc = sqlite3_prepare_v2(g_data.db, sql1, -1, &g_data.stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("SQL error: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return -1;
    }
    char MD5re[MD5_DIGEST_LENGTH];
    u64 MD5_64=0;
    char *account_buffer=path;
    while ((rc = sqlite3_step(g_data.stmt)) == SQLITE_ROW) {
        if (sqlite3_column_type(g_data.stmt, 0) != SQLITE_NULL) {
            sqlite3_int64 intValue = sqlite3_column_int64(g_data.stmt, 0);
            g_data.account_exsit_cache[intValue]=true;
        }
    }
    sqlite3_finalize(g_data.stmt);
    // 执行完毕

    //角色表 
    sql1="SELECT name FROM sqlite_master WHERE type='table' AND name='roles_info';";
    rc = sqlite3_prepare_v2(g_data.db, sql1, -1, &g_data.stmt, 0);
    if (rc != SQLITE_OK) {
        printf("SQL error: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return -1;
    }
    rc = sqlite3_step(g_data.stmt);
    sqlite3_finalize(g_data.stmt);
    if (rc == SQLITE_DONE) {
        // 只想用64位来当key
        // 没有这个表 创建一个index 表 代表了index
        char *sql_create;
        sql_create = "CREATE TABLE roles_info("  \
        // 玩家会生成一个临时id 这个guid 是唯一标识符 

        //当前服务器的唯一标识 type 8 + index 56 合区需要变换
        "GUID   INTEGER UNSIGNED BIGINT PRIMARY KEY  NOT NULL,"\
        //唯一标识 16位
        "UUID   BLOB  NOT NULL,"\

        "name      TEXT,"\
        // 头像
        "avatar     INTEGER"\
        // 基础数据  u64 t[100];
        "int100      BLOB,"\
        // str int 
        "kv_int      BLOB,"\
        // str str
        "kv_str      BLOB"\
        ")";
        rc=sqlite3_exec(g_data.db,sql_create,t_callbackss,0,NULL);
        if( rc != SQLITE_OK ){
            printf("SQL error: %s\n", sqlite3_errmsg(g_data.db));
            //sqlite3_free(g_data.zErrMsg);
            return -1;
        }
    }
    // 获取配置信息 存储配置信息
    // 自己管自己？启动读 自己写？
    ////////// 直接寻找roles_info 最大的guidindex 
    // logic开启服务器获得一次就行  异常崩掉了也不会村到数据库的 
    sql1= "SELECT MAX(GUID) FROM roles_info";
    rc = sqlite3_prepare_v2(g_data.db, sql1, -1, &g_data.stmt, 0);
    if (rc != SQLITE_OK) {
        printf("SQL error: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return -1;
    }

    rc = sqlite3_step(g_data.stmt);
    g_data.index_login_max=1;
    if (rc == SQLITE_ROW) {
        sqlite3_int64 maxGuid = sqlite3_column_int64(g_data.stmt, 0);
        g_data.index_login_max=maxGuid;
        g_data.index_login_max++;

        g_data.index_login_max=g_data.index_login_max& 0xffffffffffffff;
        printf("Max GUID: %lld\n", maxGuid);
    } else if (rc == SQLITE_DONE) {
        printf("No data found in roles_info table.\n");
    } else {
        printf("Failed to retrieve max GUID: %s\n", sqlite3_errmsg(g_data.db));
        return -1;
    }
    sqlite3_finalize(g_data.stmt);

    //login 自己发送请求



    return 0;
}
void * recv_SOCKET_KEY_TRANS(commend_triggers_param *data){
    // tid size4 + data
    /////////                tid 8     size4 + cmd1 +data
    int head_size=sizeof(u64) +sizeof(uint32_t) + 1 ;
    if (data->buffer_size <head_size ){
        return NULL;
    }
    u64 tid=0;
    memcpy(&tid,data->buffer,sizeof(tid));
    unchar cmd2=data->buffer[head_size-1];
    data->buffer=&data->buffer[head_size];
    data->buffer_size-=head_size;
    if (data->buffer_size>0){
        if (g_data.cmds[cmd2]!=NULL){
            g_data.cmds[cmd2](
                tid,
                data->send_group,
                data->buffer,
                data->buffer_size
            );
        }
    }
    return NULL;
}
void * recv_SOCKET_KEY_OFF(commend_triggers_param *data){
    if (data->buffer_size<sizeof(u64)){
        return NULL;
    }
    u64 tid=0;
    memcpy(&tid,data->buffer,sizeof(tid));
    if (tid !=0){
        if (g_data.already_login.find(tid)!=g_data.already_login.end()){
            g_data.already_login.erase(tid);
        }
    }
    return NULL;
}
void * recv_LOGIN_REQUIRE_INDEX(commend_triggers_param *data){
    if (data->buffer_size<sizeof(u64)){
        return NULL;
    }
    // 寻找最大的index 发给logic  TYPE_DESIGN8  物品

    return NULL;
}
void * recv_LOGIN_SAVE_PLAYER(commend_triggers_param *data){
    // 存玩家信息 暂时只有guid 一个 //有可能以后删除掉 需要分散存储 避免时间过长
    // 序列化
    if (data->buffer_size<=0){
        return NULL;
    }
    sql::player_info message;
    if (!message.ParseFromArray(data->buffer, data->buffer_size)) {
        printf("player_info 解析不成功 无法存储\n");
        return NULL;
    }
    // 直接存到 role
    
}
void *cmd2_LOGIN_REGISTER(u64 tid,unchar group,char *buffer,int size){
    //账号密码
    // json？ 还是什么其他的？ 无所谓 写一个函数来抽象解析一下就行 暂时用json就行
    // 关键是 用json麻烦程度和protobuf没什么区别 可能pb还方便点

}

int cmd2_LOGIN_REGISTER2(char *account, int asize,char *password,int psize){
    if (asize>20 || asize<6){
        return -1;
    }
    if (psize>20 || psize<6){
        return -1;
    }
    int is_allowed=0;
    for (int i=0;i<asize;i++){
        if (
            !((account[i]>48 && account[i] <57) ||
            (account[i]>97 &&account[i] <122) ||
             (account[i]>65 &&account[i] <90))
        ){
            return -2;
        }
    }
    // 查找数据库是否有这个account
    char MD5re[MD5_DIGEST_LENGTH];
    u64 MD5_64=0;
    MD5((unchar *)account,asize,(unchar *)MD5re);
    memcpy(&MD5_64, MD5re, sizeof(MD5_64));
    if (g_data.account_exsit_cache.find(MD5_64)!=g_data.account_exsit_cache.end()){
        // 已经有这个账号了
        return -3;
    }
    // 注册成功 
    i64 salt=0;
    while(salt ==0){
        get_random_real(&g_run->randoms,(char *)&salt,sizeof(salt));
    }


    memcpy(g_data.cache2,password,psize);
    memcpy(&g_data.cache2[psize],&salt,sizeof(salt));

    char sha256hash[SHA256_DIGEST_LENGTH];
    SHA256((unchar *)g_data.cache2,psize+sizeof(salt),(unchar *)sha256hash);
    const char *sql_insert = "INSERT INTO accounts (accounthash account, password256,password_salt) VALUES (? ,?, ?, ?);";


    // 准备插入语句
    if (sqlite3_prepare_v2(g_data.db, sql_insert, -1, &g_data.stmt, NULL) != SQLITE_OK) {
        printf( "无法准备插入语句: %s\n", sqlite3_errmsg(g_data.db));
        return COMMON_RECV_OTHERS;
    }

    // 绑定 account 值
    if (sqlite3_bind_int64(g_data.stmt, 1, MD5_64) != SQLITE_OK) {
        printf("无法绑定 sqlite3_bind_int64: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }
    // 假设 accountValue 是一个指向 account 数据的指针，accountSize 是其大小
    if (sqlite3_bind_blob(g_data.stmt, 2, account, asize, SQLITE_STATIC) != SQLITE_OK) {
        printf("无法绑定 account: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }

    // 绑定 password256 值 (sha256hash)
    if (sqlite3_bind_blob(g_data.stmt, 3, sha256hash, SHA256_DIGEST_LENGTH, SQLITE_STATIC) != SQLITE_OK) {
        printf( "无法绑定 password256: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }

    // 绑定salt
    if (sqlite3_bind_int64(g_data.stmt, 4, salt) != SQLITE_OK) {
        printf("无法绑定 sqlite3_bind_int64: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }

    // 执行插入语句
    if (sqlite3_step(g_data.stmt) != SQLITE_DONE) {
        printf( "插入账号失败: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }

    // 清理资源
    sqlite3_finalize(g_data.stmt);
    g_data.account_exsit_cache[MD5_64]=true;
    return 0;
}
void *cmd2_LOGIN_LOGININ(u64 tid,unchar group,char *buffer,int size){
    // 找到玩家 
}
int cmd2_LOGIN_LOGININ2(u64 tid ,char *account ,int  asize,char *password,int psize){
     if (asize>20 || asize<6){
        return -1;
    }
    if (psize>20 || psize<6){
        return -2;
    }
    char MD5re[MD5_DIGEST_LENGTH];
    u64 MD5_64=0;
    MD5((unchar *)account,asize,(unchar *)MD5re);
    memcpy(&MD5_64, MD5re, sizeof(MD5_64));
    if (g_data.account_exsit_cache.find(MD5_64)==g_data.account_exsit_cache.end()){
        // 没有这个账户
        return -11;
    }
    //

    char * sql="select  (password256,password_salt,guid1,guid2,guid3) from accounts where accounthash =? ;";
    if (sqlite3_prepare_v2(g_data.db, sql, -1, &g_data.stmt, NULL) != SQLITE_OK) {
        printf( "无法准备插入语句: %s %d \n", sqlite3_errmsg(g_data.db),__LINE__);
        return COMMON_RECV_OTHERS;
    }

    if (sqlite3_bind_int64(g_data.stmt, 1, MD5_64) != SQLITE_OK) {
        printf("无法绑定 sqlite3_bind_int64: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }

    int rc;int indexs=0;GUID_INFO guids;
    guids.tid=tid;
    memset(guids.is_exist,0,sizeof(guids.is_exist));
    while ((rc = sqlite3_step(g_data.stmt)) == SQLITE_ROW) {
        indexs=0;
        // 8 位serverid+xxxxx +type
        // if (sqlite3_column_type(g_data.stmt, indexs) != SQLITE_NULL) {
        //     sqlite3_int64 intValue = sqlite3_column_int64(g_data.stmt, indexs);

        // }
        // indexs++;

        // //第一列: account (BLOB)
        // const void *account = sqlite3_column_blob(g_data.stmt, indexs);
        // int accountSize = sqlite3_column_bytes(g_data.stmt, indexs);
        // indexs++;

        //  password256 (BLOB)
        const void *password256 = sqlite3_column_blob(g_data.stmt, indexs);
        int password256Size = sqlite3_column_bytes(g_data.stmt, indexs);
        indexs++;
        if (password256Size!=SHA256_DIGEST_LENGTH){
            sqlite3_finalize(g_data.stmt);
            return -3;
        }



        i64 salt=0;
        if (sqlite3_column_type(g_data.stmt, indexs) != SQLITE_NULL) {
            salt = sqlite3_column_int64(g_data.stmt, indexs);

        }
        indexs++;


        int guid_index=0;
        guids.guids[guid_index]=0;
        guids.is_exist[guid_index]=0;
        if (sqlite3_column_type(g_data.stmt, indexs) != SQLITE_NULL) {
            guids.guids[guid_index] = sqlite3_column_int64(g_data.stmt, indexs);
        }
        if (guids.guids[guid_index] !=0){
            guids.is_exist[guid_index]=1;
        }
        indexs++;guid_index++;

        guids.guids[guid_index]=0;
        guids.is_exist[guid_index]=0;
        if (sqlite3_column_type(g_data.stmt, indexs) != SQLITE_NULL) {
            guids.guids[guid_index] = sqlite3_column_int64(g_data.stmt, indexs);
        }
        if (guids.guids[guid_index] !=0){
            guids.is_exist[guid_index]=1;
        }
        indexs++;guid_index++;

        
        guids.guids[guid_index]=0;
        guids.is_exist[guid_index]=0;
        if (sqlite3_column_type(g_data.stmt, indexs) != SQLITE_NULL) {
            guids.guids[guid_index] = sqlite3_column_int64(g_data.stmt, indexs);
        }
        if (guids.guids[guid_index] !=0){
            guids.is_exist[guid_index]=1;
        }
        indexs++;guid_index++;




        if (salt==0){
            sqlite3_finalize(g_data.stmt);
            return COMMON_RECV_OTHERS;
        }
        // 判断密码
        memcpy(g_data.cache1,password,psize);
        memcpy(&g_data.cache1[psize],&salt,sizeof(salt));

        char sha256hash[SHA256_DIGEST_LENGTH];
        SHA256((unchar *)g_data.cache1,psize+sizeof(salt),(unchar *)sha256hash);

        if (memcmp((char *)sha256hash, password256, SHA256_DIGEST_LENGTH)!=0){
            sqlite3_finalize(g_data.stmt);
            // 密码不对
            return -10;
        }
        // 登陆成功 如果玩家已经在线上 需要如何处理？ 踢掉在线玩家
        
        if (g_data.already_login.find(MD5_64)!=g_data.already_login.end()){
            //通知踢掉这个玩家 需要这个时候通知马？ logic决定
            //g_data.already_login[MD5_64].tid;

        }
       


        break;
    }

    if (rc != SQLITE_DONE) {
        // 如果 sqlite3_step 不返回 SQLITE_DONE, 则表示出错
        printf("查询失败: %s %d\n", sqlite3_errmsg(g_data.db), __LINE__);
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }

    // 完成后，清理 statement 资源
    sqlite3_finalize(g_data.stmt);
    
    // tid -> info
    g_data.already_login[tid]=guids;
    // guid 的信息发给对方
    // 查找guid 的信息 发送基本信息给对方姓名和头向之类的
       

  





    return 0;
}
void *cmd2_LOGIN_CREATE_ROLE(u64 tid,unchar group,char *buffer,int size){

}
int cmd2_LOGIN_CREATE_ROLE2(u64 tid,int index,char *name ,int size,int avatar){
    // 只有tid 如何确定是哪个？
    if (g_data.already_login.find(tid)==g_data.already_login.end()){
        return COMMON_RECV_OTHERS;
    }
    auto *info =&g_data.already_login[tid];

    if (index<0 || index>=MAX_ROLE_NUMBER){
        return -1;
    }
    if (info->is_exist[index]==1){
        return -2;
    }
    if (size >MAX_ROLE_NAME_SIZE || size<=0){
        return -3;
    }
    // 需要确定是哪个头像 暂时不做限制
    if (avatar <0 || avatar>10000){
        return -4;
    }
    //name的重复和敏感词之类的
    
    

    char * sql="INSERT INTO roles_info (GUID,UUID, name,avatar) VALUES (? ,?, ?, ?)";
    if (sqlite3_prepare_v2(g_data.db, sql, -1, &g_data.stmt, NULL) != SQLITE_OK) {
        printf( "无法准备插入语句: %s %d \n", sqlite3_errmsg(g_data.db),__LINE__);
        return COMMON_RECV_OTHERS;
    }
    // 如果有了这个guid 杂拌？

    u64 gen_GUID=normal_gen_index(TYPE_DE_ROLE,g_data.index_login_max);
    g_data.index_login_max++;
    // uuid 前面加一个serverid
    char gen_UUID[UUID_SIZE_SET];
    memcpy(gen_UUID,&g_run->config.servers_id,sizeof(g_run->config.servers_id));
    memcpy(&gen_UUID[sizeof(u64)],&gen_GUID,sizeof(gen_GUID));

    if (sqlite3_bind_int64(g_data.stmt, 1, gen_GUID) != SQLITE_OK) {
        printf("无法绑定 sqlite3_bind_int64: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }
    if (sqlite3_bind_blob(g_data.stmt, 2, gen_UUID, UUID_SIZE_SET, SQLITE_STATIC) != SQLITE_OK) {
        printf("无法绑定 account: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }
    if (sqlite3_bind_text(g_data.stmt, 3, name,size,0) != SQLITE_OK) {
        printf("无法绑定 sqlite3_bind_text: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }
    if (sqlite3_bind_int(g_data.stmt, 4, avatar) != SQLITE_OK) {
        printf("无法绑定 sqlite3_bind_int: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }
    int rc=0;
    while ((rc = sqlite3_step(g_data.stmt)) == SQLITE_ROW) {
        
    }
    if (rc != SQLITE_DONE) {
        // 如果 sqlite3_step 不返回 SQLITE_DONE, 则表示出错
        printf("插入失败 %s %d\n", sqlite3_errmsg(g_data.db), __LINE__);
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }
    // 插入成功
    info->is_exist[index]=1;
    



    return 0;

}

void *cmd2_LOGIN_SELECT_ROLE(u64 tid,unchar group,char *buffer,int size){

    int result =cmd2_LOGIN_SELECT_ROLE2(tid,1,&g_data.player_sendcache);
    if(result==COMMON_RECV_OTHERS){
        return NULL;
    }
    if (result==0){

    }

}
int cmd2_LOGIN_SELECT_ROLE2(u64 tid,int index, player_infos *infos){
    if (g_data.already_login.find (tid)==g_data.already_login.end()){
        // 没有tid
        return -1;
    }
    if (index<0 || index>=MAX_ROLE_NUMBER){
        return -4;
    }
    auto info=g_data.already_login[tid];
    if (info.is_exist[index]!=1){
        // 不存在这个index
        return -2;
    }
    u64 guid_t=info.guids[index];
    if (guid_t ==0){
        return COMMON_RECV_OTHERS;
    }
    // 从roles_info 查找数据后再打包
//  sql_create = "CREATE TABLE roles_info("  \
//         // 玩家会生成一个临时id 这个guid 是唯一标识符 

//         //当前服务器的唯一标识 type 8 + index 56 合区需要变换
//         "GUID   INTEGER UNSIGNED BIGINT PRIMARY KEY  NOT NULL,"\
//         //唯一标识 16位
//         "UUID   BLOB  NOT NULL,"\

//         "name      TEXT,"\
//         // 头像
//         "avatar     INTEGER"\
//         // 基础数据  u64 t[100];
//         "int100      BLOB,"\
//         // str int 
//         "kv_int      BLOB,"\
//         // str str
//         "kv_str      BLOB"\
//         ")";
    char *sql = "select (UUID,avatar,int100,kv_int,kv_str) form roles_info where GUID=? ;";
    if (sqlite3_prepare_v2(g_data.db, sql, -1, &g_data.stmt, NULL) != SQLITE_OK) {
        printf( "无法准备插入语句: %s %d \n", sqlite3_errmsg(g_data.db),__LINE__);
        return COMMON_RECV_OTHERS;
    }
    if (sqlite3_bind_int64(g_data.stmt, 1, guid_t) != SQLITE_OK) {
        printf("无法绑定 sqlite3_bind_int64: %s\n", sqlite3_errmsg(g_data.db));
        sqlite3_finalize(g_data.stmt);
        return COMMON_RECV_OTHERS;
    }
    int rc=0;
    int indexs=0;
    // 判断
    char * int64_100=NULL;
    char * kv_int=NULL;
    char * kv_str=NULL;
    while ((rc = sqlite3_step(g_data.stmt)) == SQLITE_ROW) {
        indexs=0;
        if (sqlite3_column_type(g_data.stmt, indexs) != SQLITE_NULL) {
            const void *temb = sqlite3_column_blob(g_data.stmt, indexs);
            int tembs = sqlite3_column_bytes(g_data.stmt, indexs);
            if (tembs>0){
                if (tembs<4096){
                    int64_100=infos->cahce[0];
                }else{
                    int need =tembs/4096;
                    if (tembs%4096 !=0){
                        need++;
                    }
                    int64_100=(char *)malloc(need*4096);
                }
                infos->caches[0]=tembs;
                memcpy(int64_100,temb,tembs);
            }
        }
        indexs++;

        if (sqlite3_column_type(g_data.stmt, indexs) != SQLITE_NULL) {
            const void *temb = sqlite3_column_blob(g_data.stmt, indexs);
            int tembs = sqlite3_column_bytes(g_data.stmt, indexs);
            if (tembs>0){
                if (tembs<4096){
                    kv_int=infos->cahce[1];
                }else{
                    int need =tembs/4096;
                    if (tembs%4096 !=0){
                        need++;
                    }
                    kv_int=(char *)malloc(need*4096);
                }
                infos->caches[1]=tembs;
                memcpy(kv_int,temb,tembs);
            }
        }
        indexs++;

        if (sqlite3_column_type(g_data.stmt, indexs) != SQLITE_NULL) {
            const void *temb = sqlite3_column_blob(g_data.stmt, indexs);
            int tembs = sqlite3_column_bytes(g_data.stmt, indexs);
            if (tembs>0){
                if (tembs<4096){
                    kv_str=infos->cahce[2];
                }else{
                    int need =tembs/4096;
                    if (tembs%4096 !=0){
                        need++;
                    }
                    kv_str=(char *)malloc(need*4096);
                }
                infos->caches[2]=tembs;
                memcpy(kv_str,temb,tembs);
            }
        }
        indexs++;
        break;
    }

    sqlite3_finalize(g_data.stmt);
    infos->array=int64_100;
    infos->arrays=infos->caches[0];

    infos->kv_int=kv_int;
    infos->kv_ints=infos->caches[1];

    infos->kv_str=kv_str;
    infos->kv_strs=infos->caches[2];
    int res=0;


    
    while(1){
        if (rc != SQLITE_DONE){
        // 出问题了 那么直接判断释放
            res=COMMON_RECV_OTHERS;
            break;
        }


        return 0;
    }




    if (infos->array!= NULL){
        if (infos->arrays >4096){
            free(infos->array);
        }
    }
    if (infos->kv_int!= NULL){
        if (infos->kv_ints >4096){
            free(infos->kv_int);
        }
    }
    if (infos->kv_str!= NULL){
        if (infos->kv_strs >4096){
            free(infos->kv_str);
        }
    }



    
  
    return res;
}
int register_player_cmd_func(unchar id,self_commend_trigger data){
    g_data.cmds[id]=data;
    return 0;
}
int t_callbackss(void *data, int argc, char **argv, char **azColName) {
    
    // for (int i = 0; i < argc; i++) {
    //     printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    // }
    // printf("\n");

    
    return 0;
}

