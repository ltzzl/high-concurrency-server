#pragma once
//初始化一个用于监听的套接字
int initListenFd(unsigned short port);

//启动服务器程序(epoll)
int epollRun(int fld);
