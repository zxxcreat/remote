#ifndef EXAMPLE_DELAY
#define EXAMPLE_DELAY

#include <sys/time.h>
#include <map> 
#include <unordered_map> 
#include "times.h"
#include "macro.h"
// 获取时间戳 获取结构 时间戳转结构 结构转时间戳
typedef void*(*delay_func)(void *data); 
// 最多50ms一次 所有的都存在key
// 所有间隔和时间单位都是50ms起步
#define GAP_MIN_PER 50
#define COUNTS_FOREVER 999999999
typedef struct 
{
    // 持续的次数
    int count;
    int id;
    // 参数
    void *param_ptr;
    
    delay_func funcs;
    // 单位ms
    u64 gap; 
    u64 start_time;

}delay_single;
#define MAX_OP_COUNT 100
typedef struct 
{
    // 56位时间  8位是额外的
    // 找到定时器 修改定时器
    // 创建定时器 通过id

    //两个操作 删除0 和挪动 1
    int op_list[MAX_OP_COUNT];
    // 删除或者挪动的是哪个id
    int id_list[MAX_OP_COUNT];
    // 挪动到哪里计算就行
    int op_size;


    std::unordered_map<int,u64> id2real;
    /// ///// 是time<<8
    std::map<u64,delay_single> data;
}delay_manage;

// 任何delay
int insert_delay_single(delay_manage *data,delay_single *param);
int runing_delay_manage(delay_manage *data,u64 time_now);

i64 count_rest_delay_manage(delay_manage *data,u64 time_now);
void delete_delay_manage(delay_manage *data,int id);
#endif // EXAMPLE_H1