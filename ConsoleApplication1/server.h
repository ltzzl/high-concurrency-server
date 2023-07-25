#pragma once
//初始化一个用于监听的套接字
int initListenFd(unsigned short port);

//启动服务器程序(epoll)
int epollRun(int fld);

//和客户端建立连接
// 监听文件描述符，epoll树的根节点
int acceptClient(int lfd,int epfd);
