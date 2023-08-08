#pragma once
//初始化一个用于监听的套接字
int initListenFd(unsigned short port);

//启动服务器程序(epoll)
int epollRun(int fld);

//和客户端建立连接
// 监听文件描述符，epoll树的根节点
int acceptClient(int lfd,int epfd);

//接受客户端http的请求消息

int recvHttpRequest(int cfd, int epfd);

//get 请求解析请求行
int parseRequestLine(const char* line, int cfd);

//发送文件
int sendFile(const char* fileName, int cfd);

//发送响应头（状态行，响应头）
int sendHeadMsg(int cfd, int status, const char* descr, const char* type, int length);
