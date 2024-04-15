#ifndef C_RUN_INCLUDE1
#define C_RUN_INCLUDE1
#include "common_s.h"
// 三个线程  发送 接收 logic
// target 转为index
#define COMMON_RECV_OTHERS -1000
#define MAX_ROLE_NUMBER 3
#define MAX_ROLE_NAME_SIZE 30



// 玩家来的消息
enum LOGIN_CMD2{
    LOGIN_CMD2_START=0,



    // 没有big 因为都是简单的信息查询 不需要
    // 注册账户
    LOGIN_REGISTER ,
    // 登陆账户
    LOGIN_LOGININ,


    // 创建角色
    LOGIN_CREATE_ROLE,
    //选择角色
    LOGIN_SELECT_ROLE,



    LOGIN_END
};

typedef struct {
    char data[32];
} Char_Hash;
// "12345678901234567890123456789012"
// 哈希函数
struct Char_HashHash {
    size_t operator()(const Char_Hash& qi) const {
        // 计算并返回queue_info的哈希值
        uint64_t out[2]; // 用于存储128位哈希值的数组
        MurmurHash3_x64_128(qi.data, sizeof(qi.data), 0, out); // 以0作为种子
        return out[1]; // 返回128位哈希值的后64位
    }
    
};

// 比较函数
struct Char_HashEqual {
    bool operator()(const Char_Hash& lhs, const Char_Hash& rhs) const {
        // 比较两个queue_info对象是否相等
        return memcmp(lhs.data, rhs.data, sizeof(lhs.data))==0;
    }
};

typedef void*(*self_commend_trigger)(u64 tid,unchar group,char *buffer,int size); 

typedef struct{
    char is_exist[8];
    // 已经登陆了？
    u64 tid;
    u64 guids[3];


}GUID_INFO;
typedef struct{
    // cache的内容
    char cahce[3][4096];
    // cache的大小
    int caches[3];
    // 玩家身上的变量
    char * array;
    int arrays;
    char *kv_int;
    int kv_ints;
    char *kv_str;
    int kv_strs;

    // 传给logic 可能分段传输的 现在只有变量 后面要加再说


}player_infos;



typedef struct{
    // 需要初始化



    u64 index_login_max;
    u64 index_logic_max;

    self_commend_trigger cmds[256];
    sqlite3 *db;
    char *zErrMsg;
    sqlite3_stmt* stmt;
    // 所有的账户 hash 缓存避免大量io
    std::unordered_map<u64, bool> account_exsit_cache;


    // 玩家尝试登陆的次数  1分钟只允许一个账号尝试登陆三次 成功变成0 失败+1 
    //                accounthash times
    std::unordered_map<u64, u64> login_times;


    // 已经登陆的 账户需要选择guid 
    std::unordered_map<u64, GUID_INFO> already_login;

    char cache1[200];
    char cache2[200];


    player_infos player_sendcache;

}globe_common2;
int runing_on_start(void *data);
inline int register_player_cmd_func(unchar id,self_commend_trigger data);

// 通用的
void * recv_SOCKET_KEY_TRANS(commend_triggers_param *data);
void * recv_SOCKET_KEY_OFF(commend_triggers_param *data);
// 自己的 
void * recv_LOGIN_REQUIRE_INDEX(commend_triggers_param *data);
void * recv_LOGIN_SAVE_PLAYER(commend_triggers_param *data);



void *cmd2_LOGIN_REGISTER(u64 tid,unchar group,char *buffer,int size);
int cmd2_LOGIN_REGISTER2(char *account, int asize,char *password,int psize);


void *cmd2_LOGIN_LOGININ(u64 tid,unchar group,char *buffer,int size);
int cmd2_LOGIN_LOGININ2(u64 tid ,char *account ,int  asize,char *password,int psize);



void *cmd2_LOGIN_CREATE_ROLE(u64 tid,unchar group,char *buffer,int size);
int cmd2_LOGIN_CREATE_ROLE2(u64 tid,int index,char *name ,int size,int avatar);

void *cmd2_LOGIN_SELECT_ROLE(u64 tid,unchar group,char *buffer,int size);
int cmd2_LOGIN_SELECT_ROLE2(u64 tid,int index, player_infos *infos);



int t_callbackss(void *data, int argc, char **argv, char **azColName);

u64 normal_gen_index2(unchar type);
#endif