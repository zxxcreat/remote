#ifndef EXAMPLE_TIME
#define EXAMPLE_TIME
#include <sys/time.h>
#include <time.h>
#include "macro.h"
// 获取时间戳 获取结构 时间戳转结构 结构转时间戳
i64 times_get_time(void);
void times_struct2num(struct tm * data,time_t * num);
void times_num2struct(time_t num,struct tm * data);
#endif // EXAMPLE_H1