#include <cstdio>
#include "server.h"
int main()
{
    //初始化一个用于监听的套接字
    int lfd = initListenFd(10000); 
    //unsigned short 为2个字节 0-65535 不要创建5000以下的端口，电脑的软件可能占用。 
    
    
    //启动服务器程序(epoll)
    
    return 0;
}