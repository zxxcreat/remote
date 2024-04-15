
#ifndef EXAMPLE_H1
#define EXAMPLE_H1
// 头文件
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
// //////////////////////环形队列 前提所有的数据都是小端模式的
typedef struct {
    unsigned char *buffer; // 缓冲区
    int head;              // 头部指针，指向下一个要读取的位置
    int tail;              // 尾部指针，指向下一个要写入的位置
    int maxSize;           // 缓冲区的最大容量
    int saved_size;        // 塞入的大小 
} CircularBuffer;
CircularBuffer* ini_CircularBuffer(int size);
CircularBuffer* ini_CircularBuffer2(int size);
int insert_CircularBuffer(CircularBuffer *data,char * buffers,int size);
inline int count_rest_size_CircularBuffer(CircularBuffer *data){
    return data->maxSize - data->saved_size;
}
uint32_t is_already_CircularBuffer(CircularBuffer *data);
// 如果跨地形了 直接复制到buffer 没有跨地形 直接返回一个指针不复制
void pop_CircularBuffer(CircularBuffer *data,char ** buffers,int *size);

void delete_CircularBuffer(CircularBuffer *data);
void delete_CircularBuffer1(CircularBuffer *data);
void reset_CircularBuffer(CircularBuffer *data);
// 不加锁的读写 需要测试

typedef struct {
    unsigned char *buffer; // 缓冲区
    int head;              // 头部指针，指向下一个要读取的位置
    int tail;              // 尾部指针，指向下一个要写入的位置
    int maxSize;           // 缓冲区的最大容量
} CircularBufferex;
CircularBufferex* ini_CircularBufferex(int size);
inline void delete_CircularBufferex(CircularBufferex *data){
    free(data);data=NULL;
}
// insert pop rest
int count_Rest_CircularBufferex(CircularBufferex *data);
// insert_pop 
int insert_CircularBufferex(CircularBufferex *data,char *buffer,int size);
void pop_CircularBufferex(CircularBufferex *data,char * buffers,int *size);

// 

////////////////////////////// arm的弱内存模型需要加内存屏障访问 x86不需要
//  写 release     acquire 读 

#define QUEUE_SIZE_BEGINS (1024*1024)
typedef struct {
    // 发送方使用
    // 1 2 4 8 16 32 64 128 256 512 
    CircularBufferex * list[10];
    int next_index;
    int times_use;
    int max_index;

    int reset_index;
    // 接收方使用
    int get_index;
} QUEUE_manage;
#define COUNT_INDEX_QUEUE_SIZE(a) (QUEUE_SIZE_BEGINS*(1<<a))
////////////////////////////////////4 recv 6 send
QUEUE_manage *  ini_QUEUE_manage(int replace);
void insert_QUEUE_manage(QUEUE_manage * data,char *buffer,int size);
//  
typedef struct {
    // head in index 0
    char *buffer[10];
    int buffersize[10];
    int sizes;
} QUEUE_param_split;
void insert_QUEUE_manage2(QUEUE_manage * data,char *head,int hsize,char *buffer,int size);
void insert_QUEUE_manage3(QUEUE_manage * data,QUEUE_param_split *params);
void pop_QUEUE_manage(QUEUE_manage *data,char * buffers,int *size);

















///////////////////////////////////////////二进制拼凑逻辑
uint32_t epoll_get_data_size(char* buffer);
void epoll_set_data_size(char* buffer,uint32_t size);
void epoll_ini_data_size_cmd(char *buffer,unsigned char serverid,unsigned char cmd);
void epoll_ini_data_size_cmd2(char *buffer,unsigned char serverid,unsigned char cmd,unsigned char selfid);

////////////////////////////////////////////// 简单node
typedef struct Node {
    long long data;
    void *ptr;
    struct Node* prev;
    struct Node* next;
} Node;

typedef struct NodeInfo {
    Node* head;
    Node* tail;
    int num;
} NodeInfo;
void insertNode(Node *newNode, NodeInfo *info) ;
void deleteNode(Node *nodeToDelete, NodeInfo *info);





uint32_t mode_changes(uint32_t data);

void log1(char *s);
#define DEUBG_PRINTF printf
#define LOG1(a,b,c) sprintf(a,b,c);log1(a);
#ifdef __cplusplus
}
#endif

#endif // EXAMPLE_H1
