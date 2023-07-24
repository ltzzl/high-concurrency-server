#include "server.h"
#include "arpa/inet.h"
#include "cstdio"
#include "sys/epoll.h"


int initListenFd(unsigned short port)
{
	//1.创建一个监听套接字
	//af_inet 为ipv4 ，流式协议包含tcp，0为tcp，
	int lfd = socket(AF_INET, SOCK_STREAM, 0);

	//判断是否创建成功
	if (lfd == -1) {
		//报错
		perror("socket");
		return -1;
	}


	//2.设置端口复用
	int opt = 1;
	//监听用文件描述符，sol_socket,so_reusepot也可以，指定一个整型变量的地址为1是可以复用,整型内存大小
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof opt);

	//判断是否设置复用成功
	if (ret == -1) {
		perror("socket");
		return -1;
	}



	//3.绑定端口

	//创建结构体
	struct sockaddr_in addr;
	//设置地址族协议为ipv4
	addr.sin_family = AF_INET;
	//指定端口的网络字节序（大端） 通过初始化监听套接字的参数传进来
	addr.sin_port = htons(port);
	//指定绑定的本地ip地址 设置为0为任意 且0无大小端之分
	addr.sin_addr.s_addr = INADDR_ANY;

	//监听用文件描述符，，绑定的地址 用sockaddr_in结构体更好 则需要类型转换 ，结构体大小
	ret = bind(lfd, (struct sockaddr*)&addr, sizeof addr);

	//判断是否绑定成功
	if (ret == -1) {
		perror("bind");
		return -1;

		//4.设置监听
			//监听用文件描述符，监听数最大为128 大于128时恒为128
		ret = listen(lfd, 128);

		//判断是否设置监听成功
		if (ret == -1) {
			perror("listen");
			return -1;



			//5.将fd返回给调用者
			return lfd;
		}
	}
}

	int epollRun(int lfd)
	{
		//创建一个epoll实例

		//参数被弃用，指定一个大于0的数字即可。无实际意义
		int epfd = epoll_create(1);

		//判断epfd是否创建成功
		if (epfd == -1) {
			perror("epoll_create");
			return -1;
		}

		
		
		//添加到epoll树上

		//创建文件描述符的事件结构体
		struct epoll_event ev;
		//data是一个联合体，共用内存
		//fd记录事件的文件描述符
		ev.data.fd = lfd;
		//events记录事件，对服务器来说是读事件，读取客户端发来的消息
		ev.events = EPOLLIN;

		//通过epoll_create得到的树根节点，需要上树epoll_ctl_add,需要操作的文件描述符,文件描述符的事件结构体
		int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);

		//判断是否上树成功
		if (ret == -1) {
			perror("epoll_stl");
			return -1;
		}

		

		//持续检测

		//创建结构体数组
		//持续检测，大小不重要。记录epoll树上激活的事件结构体。
		struct epoll_event evs[1024];
		//计算事件结构体数组大小 总字节数除以一个元素的大小
		int size = sizeof(evs) / sizeof(struct epoll_event);
		
		while (1) {
			
			//epoll_wait是线程安全函数，判断是否触发事件并进行处理
			//进行条件判断的epoll树描述符，事件结构体数组，事件结构体数组大小，无事件触发阻塞时间 -1为一直阻塞直到触发事件
			int num = epoll_wait(epfd,evs, size, -1);
			
			//记录触发的事件，循环取出判断是否是监听的描述符。
			for (int i = 0; i < num; ++i) {
				int fd = evs[i].data.fd;
				if (fd == lfd) {
					//建立新链接，经过epoll_wait检测到lfd后accept函数不阻塞

				}
				else {
					 //通讯 
					//当写缓冲区有空间时，触发写事件。一般情况下写事件一直触发。可以不用写判断
					
					//当收到信息时，读事件触发
				}
			}
		}

		return 0;
	}
