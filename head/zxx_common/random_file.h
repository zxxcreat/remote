// 随机 可以对文件随机 所以 随机可以是 随机一个压缩的文件  大小随意就可以
// 随机还可以是某个文件的随机 没有分布的要求 那么就可以压缩一个文件读二进制
// 混合编成导致编不过去 
#include <stdlib.h> // srand, rand, RAND_MAX, EXIT_SUCCESS
#include <time.h> // time
#include <stdio.h>
#include <stdint.h>
#define RAMDOM_FILE_SIZE 4096*1024

// 还有文件的名字
#ifdef __cplusplus
extern "C"{
#endif
typedef struct {
    int max_size;
    int use_index;
    char buffer[4096];

}random_buffer;
unsigned long long random_int(unsigned long long m,unsigned long long n);



int ini_random_seed(void);
void ini_pre_random(random_buffer * data);

static inline uint64_t rotl(const uint64_t x, int k);
uint64_t next(void);
void jump(void);
void long_jump(void);

// 获取linux 系统级随机数
void get_linux_ramdom(char *buffer,int size);

void get_random(random_buffer * data,char *buffer,int size);
void get_random_real(random_buffer * data,char *buffer,int size);
#ifdef __cplusplus
}
#endif

