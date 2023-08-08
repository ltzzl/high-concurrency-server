#include <cstdio>
#include "server.h"
#include <iostream>
#include "unistd.h"



int main(int argc,char* argv[])
{
    if (argc < 3) {
        //服务器绑定的端口  服务器绑定的资源目录
        std::cout << "./a.out port path" << std::endl;
        return -1;
    }
    unsigned short port = atoi(argv[1]);
    //将服务器的工作目录切换到请求路径对应的目录
    chdir(argv[2]);



    //初始化一个用于监听的套接字
    int lfd = initListenFd(10000); 
    //unsigned short 为2个字节 0-65535 不要创建5000以下的端口，电脑的软件可能占用。 
    
    
    //启动服务器程序(epoll)
    
    return 0;
}