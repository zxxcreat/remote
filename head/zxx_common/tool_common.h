#ifndef COMMON_INCLUDE
#define COMMON_INCLUDE

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
// #include <netdb.h>
#include <unistd.h> // windows.h
#include <arpa/inet.h>
#include <pthread.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <unordered_map>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <ifaddrs.h>
#include <sys/eventfd.h> 
#include <sys/wait.h>
//#include <sys/types.h>



#include <zip/zlib.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <hiredis/hiredis.h>


#include "macro.h"
#include "times.h"
#include "tools2.h"
#include "lua_module.h"
#include "openssl_operate.h"
#include "random_file.h"
#include "config_read.h"
#include "delay.h"
#include "MurmurHash3.h"
#include "cJSON.h"
#endif