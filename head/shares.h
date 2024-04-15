#include "pb_total.h"
#ifndef MACRO_SHARES 
#define MACRO_SHARES
#define UUID_SIZE_SET 16
enum BIG_CMD{
    // 登陆服务器没有 没有很大的需要发送过来的所以不需要
    // 252  253 254 255
    COMMON_CLIENTBIG_START=252,
    COMMON_CLIENTBIG_RUNNING,
    COMMON_CLIENTBIG_FINISH,
    COMMON_CLIENTBIG_END
};
enum TYPE_DESIGN8{
    //8位type的定义
    TYPE_DE_START=0,
    // 人物
    TYPE_DE_ROLE,
    TYPE_DE_END
};

//服务器来的消息 需要写出对应的函数注册
enum CMD1_LOGIN{
    CMD1_LOGIN_START=100,
    // 申请guid的最大值 物品之类的 以后有item后再实现
    CMD1_LOGIN_REQUIRE_INDEX,
    // 存某个玩家的信息
    CMD1_LOGIN_SAVE_PLAYER,

    CMD1_LOGIN_END
};
//服务器来的消息
enum CMD1_LOGIC{
    CMD1_LOGIC_START=100,
    // 申请guid的最大值 返回
    CMD1_LOGIC_REQUIRE_INDEX,


    CMD1_LOGIC_END
};
  // SearchRequest aaa;
    //  messaaaage.ByteSizeLong()
    // char *bbb;
    // aaa.ParseFromArray(bbb,20);

    
    // aaa.set_query(bbb,20);
    // aaa.query();

#endif
