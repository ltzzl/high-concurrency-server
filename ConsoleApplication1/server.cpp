#include "server.h"
#include "arpa/inet.h"
#include "cstdio"
#include "sys/epoll.h"
#include "fcntl.h"
#include "stdio.h"
#include <string.h>
#include "errno.h"
#include "unistd.h"
#include <strings.h>
#include "sys/stat.h"
#include "assert.h"
#include "sys/sendfile.h"


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
					//传入监听描述符，和需要挂上的树描述符
					acceptClient(lfd, epfd);

				}
				else {
					 //通讯 
					//当写缓冲区有空间时，触发写事件。一般情况下写事件一直触发。可以不用写判断
					
					//当收到信息时，读事件触发
					recvHttpRequest(fd, epfd);
				}
			}
		}

		return 0;
	}

	//连接客户端
	//监听文件描述符，epoll树根节点
	int acceptClient(int lfd, int epfd)
	{
		//1.建立连接
		//监听文件描述符，new一个结构体用于保存客户端的ip和端口信息 不需要则为NULL，结构体的大小
		int cfd = accept(lfd, NULL, NULL);
		if (cfd == -1) {
			perror("accept");
			return -1;
		}
		
		
		//2.设置为边沿非阻塞模式，提高效率。
		//得到连接文件描述符的属性
		//连接描述符，得到属性宏
		int flag = fcntl(cfd, F_GETFL);
		//追加非阻塞属性
		flag |= O_NONBLOCK;
		//将属性赋给连接文件描述符 fcntl是个变参函数
		//需要操作的文件描述符，操作类型，属性
		fcntl(cfd, F_SETFL,flag);


//3.将cfd连接文件描述符挂到epoll树上（和lfd上树一样）
//创建文件描述符的事件结构体
struct epoll_event ev;
//data是一个联合体，共用内存
//fd记录事件的文件描述符
ev.data.fd = cfd;
//events记录事件，对服务器来说是读事件，读取客户端发来的消息
//设置为边沿模式
ev.events = EPOLLIN | EPOLLET;

//通过epoll_create得到的树根节点，需要上树epoll_ctl_add,需要操作的文件描述符,文件描述符的事件结构体
int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);

//判断是否上树成功
if (ret == -1) {
	perror("accept");
	return -1;
}

return 0;
	}



	//通讯文件描述符，所在树节点的文件描述符（断开后不再检测事件）
	int recvHttpRequest(int cfd, int epfd)
	{
		char buf[4096] = { 0 };
		//创建一个用来接数据的temp;
		char temp[1024] = { 0 };

		//定义当前接受数据的长度和总共接受数据的长度
		int len, totle;
		//循环读取直到读完
		while (len = recv(cfd, buf, sizeof(buf), 0) > 0) {
			//判断每次接受是否小于总大小
			//读到请求的请求方式和请求行即可。
			if (totle += len < sizeof(buf)) {
				memcpy(buf + totle, temp, len);
			}
			totle += len;
		}

		//判断数据是否接受完毕
		//非阻塞在读完了再读空值会返回-1，失败了也会返回-1。可以通过errnum判断错误信息
		if (len == -1 && errno == EAGAIN) {
			//此时为读完
			//解析请求行
		}
		else if (len == 0) {
			//此时为断开连接
			epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
			close(cfd);

		}
		else {
			perror("recv");
		}
		return 0;
	}

	int parseRequestLine(const char* line, int cfd)
	{
		//将请求行分为三部分。get/post请求头  客户端请求的静态资源  客户端请求的http版本
		// get /xxx/1.jpg http/1.1
		//各部分之间是通过空格分开的 读空格可以区分  sscanf可以字符串格式化并拆分（正则）

		//请求方式
		char method[12];

		//请求的静态资源
		char path[1024];

		sscanf(line, "%[^ ] %[^ ]", method, path);
		//若为post请求 则不处理
		//此方法不区分大小写
		if (strcasecmp(method, "get") != 0) {
			return -1;

		}
		//处理客户端请求的静态资源（目录或文件）
		char* file = NULL;

		//判断切换到的是否为资源根目录
		if (strcmp(path, "/") == 0) {
			file = "./";
		}
		else {
			//不为根目录则去掉 / 变成xxx/1.jpg格式即可
			file = path + 1;
		}

		//获取文件属性
		struct stat st;

		//文件路径，储存文件属性的结构体
		int ret = stat(file, &st);

		if(ret = -1){
			//文件不存在 回复一个404页面
		}

		//判断文件类型
		if (S_ISDIR(st.st_mode)) {
			//若为目录则返回值为1
			//把目录的内容发送给客户端

		}
		else {
			//不是目录则为文件，发送给客户端
		}

		return 0;
	}

	int sendFile(const char* fileName, int cfd)
	{
		//tcp可以一点一点的发送文件（流没有固定的大小限制），而不是读完全部发。
		//两端建立连接后，可以不停发送

		//回复是http的响应格式，sendfile为第四部分，第三部分写空行，第一第二部分用函数拼出来


		//将文件打开，只读
		int fd = open(fileName, O_RDONLY);
		
		//判断是否打开成功，断言方式
		assert(fd > 0);

		////不停在文件中读数据
		//while (1)
		//{
		//	char buf[1024];
		//	int len = read(fd, buf, sizeof(buf));
		//	if (len > 0) {
		//		//发送给客户端
		//		//防止读数据过快，浏览器解析过慢导致缓存堆满或其他问题
		//		send(cfd, buf, len, 0);
		//		//休眠1ms
		//		usleep(10);
		//	}
		//	else if (len == 0) {
		//		//读完了
		//		break;
		//	}
		//	else {
		//		perror("read");
		//	}
		//}
		
		//不写while循环，使用linux自带的sendFile函数，简单且减少内存拷贝
		
		//发送文件的通信描述符，需要传入函数的文件的文件描述符，发送位置的偏移量，发送数据的大小
		//求出文件大小 需要读取的文件的文件描述符，起始的偏移量，最终的漂移动量   从0到文件尾的总偏移量就是文件大小
		int size = lseek(fd, 0, SEEK_END);
		sendfile(cfd, fd, NULL, size);


		return 0;
	}

	//通信的文件描述符，状态行的状态码，状态行的状态描述，响应头的响应类型，响应头长度（不知道写-1或不指定）
	int sendHeadMsg(int cfd, int status, const char* descr, const char* type, int length)
	{
		//在调用sendfile前先调用这个函数把前三部分发出去，再加上sendfile组成一个完整的http响应格式
		//状态行
		char buf[4096] = { 0 };
		//拼接字符串
		sprintf(buf, "http/1.1 %d %s\s\n", status, descr);


		//响应头
		//buf+strlen(buf)预留空白内存
		sprintf(buf + strlen(buf), "content-type:%s\r\n", type);
		//再拼上第二行和第三行空行\r\n
		sprintf(buf + strlen(buf), "content-length:%d\r\n\r\n", length);

		//发送出去
		send(cfd, buf, strlen(buf), 0);
		return 0;
	}




	
