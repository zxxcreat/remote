
#ifndef MACRO_MY 
#define MACRO_MY
//https://republicengineeringstudent.gitee.io/2022/06/22/OpenSSL/
// opensll 介绍
typedef unsigned char  unchar;
typedef long long i64;
typedef unsigned long long u64;
typedef unsigned short u16;
// 需要再加
typedef struct{
    // 这两个是需要大数据传输的时候才需要初始化和使用
    // 初始化需要处理好
    void * big_info;
    //cmd 处理
    void * deals;





    // 动态给予的 所有的必须要要
    char *buffer;
    int buffer_size;
    //发送的id
    unchar send_group;
    
}commend_triggers_param;
typedef void*(*commend_trigger)(commend_triggers_param *data); 


// 获取int a第b(0-31)位的数据
#define GET_INT_BIT(a,b) ((a)>>(b))&1
// 设置int a第b(0-31)位的数据为1
#define SET_INT_BIT1(a,b) (a)|=(1<<(b)) //将X的第Y位置1
// 设置int a第b(0-31)位的数据为0
#define SET_INT_BIT0(a,b)  (a)&=~(1<<(b))

// bc 32位数 a 64位
#define MAKE_INT64(a,b,c) a=b;a=a<<32;a=a|c; 
#define PARES_INT64(a,b,c) c=a;a=a>>32;b=a;

// bc 16位数 a 32位
#define MAKE_INT32(a,b,c) a=b;a=a<<16;c=c&0xffff;a=a|c; 
#define PARES_INT32(a,b,c) c=a;c=c&0xffff; a=a>>16;b=a;
// 每个mcu大小1500左右 留出100备用
#define MAX_TRANS_SIZE 1350
#define MAX_SEND_SIZE 1400
//                4     1     1    1   x
// 内部传输数据构成 size+target+cmd+send+data
// size的大小
#define LOCAL_SEND_SIZE 4
// target+ cmd +send的大小
#define LOCAL_SEND_TCS 3

#define HEAD_LOCAL_COUNT_SIZE (LOCAL_SEND_SIZE+LOCAL_SEND_TCS)


#define GROUP_ID_INDEX 6
#define CMD_INDEX_BIN 5



// 通用的 不允许修改 0-100 
#define CENTER_0_register  0
#define CENTER_0_register_re  1
// 内部bigdata  不一定一次性能发完
#define RECV_BIG_START  5
#define RECV_BIG_RUNNING  6
#define RECV_BIG_END  7
// 除了gate 其他的都需要
// 某某离线了
#define SOCKET_KEY_OFF  8
// 某某发来信息
#define SOCKET_KEY_TRANS  9

enum GATEWAT{
    GATEWAT_START=100,
    GATEWAT_KICK ,
    GATEWAT_SEND_CLIENT,
    GATEWAT_SET_MODE,
    GATEWAT_END
};



#endif