#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include "iostream"
#include "stdio.h"
using namespace std;
//加入windows的静态链接库--在windows里面支持这种写法
//最好还是在属性设置中去加入ws2_32.lib
//#pragma comment(lib, "ws2_32.lib")
int main() 
{
	/*启动Windows socket 2.x环境*/
	//版本号
	/*
	socket编程中：
	声明调用不同的Winsock版本。例如MAKEWORD(2,2)就是调用2.2版，MAKEWORD(1,1)就是调用1.1版。
	不同版本是有区别的，例如1.1版只支持TCP/IP协议，而2.0版可以支持多协议。2.0版有良好的向
	后兼容性，任何使用1.1版的源代码、二进制文件、应用程序都可以不加修改地在2.0规范下使用。
	此外winsock 2.0支持异步 1.1不支持异步.
	*/
	WORD ver = MAKEWORD(2, 2);
	/*WSADATA，一种数据结构。这个结构被用来存储被WSAStartup函数调用后返回的Windows Sockets数据。它包含Winsock.dll执行的数据。*/
	WSADATA dat;
	//socket网络编程启动函数 启动服务器
	WSAStartup(ver, &dat);
	//---------------------------
	//--用Socket API建立简易TCP服务端
	//1、建立一个socket  套接字 （windows） linux上指的是指针
	/*socket(
	_In_ int af,(表示什么类型的套接字)
	_In_ int type,(数据流)
	_In_ int protocol
	);*/
	//IPV4的网络套接字 AF_INET
	//IPV6的网络套接字 AF_INET6
	SOCKET _sock =  socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//IPPROTO_TCP 使用TCP协议

	//2、bind 绑定用于接收客户端链接的网络端口
	/*
	bind(
	_In_ SOCKET s,
	_In_reads_bytes_(namelen) const struct sockaddr FAR * name,
	_In_ int namelen
	);
	*/
	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;//ipv4
	_sin.sin_port = htons(4567);//端口号 由于主机和网络的数据类型不同 因此需要进行转换 使用 host to net unsigned short
	_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");//服务器的ip地址 INADDR_ANY本机所有的Ip地址都可以访问 一般这样
	//有可能绑定失败
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) 
	{
		printf("错误，绑定网络端口失败...\n");
	}
	else
	{
		printf("绑定端口成功...\n");
	}

	//3、listen 监听网络端口
	/*
	listen(
	_In_ SOCKET s,
	_In_ int backlog
	);*/
	//5 代表的是接口最多5个客户端可与服务器进行连接
	if (SOCKET_ERROR == listen(_sock, 5))
	{
		printf("错误，监听网络端口失败...\n");
	}
	else 
	{
		printf("监听网络端口成功...\n");
	}

	//4、accept 等待接收客户端链接
	/*
	accept(
	_In_ SOCKET s,
	_Out_writes_bytes_opt_(*addrlen) struct sockaddr FAR * addr,
	_Inout_opt_ int FAR * addrlen
	);*/
	sockaddr_in clientAddr = {};//客户端地址
	int nAddrLen = sizeof(sockaddr_in);//地址长度
	SOCKET _cSocket = INVALID_SOCKET;
	//char msgBuf[] = "Hello, I'm Server.";
	_cSocket = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSocket)
	{
		printf("错误，接收到无效客户端SOCKET...\n");
	}
	printf("新客户端加入: socket = %d IP = %s \n", _cSocket, inet_ntoa(clientAddr.sin_addr));//inet_ntoa转换为可读地址
	//一直不停的收发数据
	char _recvBuf[128] = {};//接收缓冲区
	while (true)
	{
		//5 接收客户端请求数据
		/*recv(
			_In_ SOCKET s,
			_Out_writes_bytes_to_(len, return) __out_data_source(NETWORK) char FAR * buf,
			_In_ int len,
			_In_ int flags
		);*/
		//以128长度来接受
		int nLen = recv(_cSocket, _recvBuf, 128, 0);
		if (nLen <= 0)
		{
			printf("客户端已经退出, 任务结束。");
			break;
		}
		printf("收到命令: %s\n", _recvBuf);
		//6 处理请求
		if (0 == strcmp(_recvBuf, "getName"))
		{
			//7 send 向客户端发送一条数据
			char msgBuf[] = "xiao qiang.";
			//加1的目的是为了将字符串的末尾一并加入 方便客户端进行字符串长度的计算
			send(_cSocket, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else if(0 == strcmp(_recvBuf, "getAge"))
		{
			//7 send 向客户端发送一条数据
			char msgBuf[] = "22";
			send(_cSocket, msgBuf, strlen(msgBuf) + 1, 0);
		}
		else
		{
			//7 send 向客户端发送一条数据
			char msgBuf[] = "?????";
			send(_cSocket, msgBuf, strlen(msgBuf) + 1, 0);
		}
	}
	

	//5、send 向客户端发送一条数据
	/*send(
	_In_ SOCKET s,
	_In_reads_bytes_(len) const char FAR * buf,
	_In_ int len,
	_In_ int flags
	);*/
	//char msgBuf[] = "Hello, I'm Server.";
	//+1表示将结尾符一并发送过去
	//send(_cSocket, msgBuf, strlen(msgBuf) + 1, 0);

	//6、关闭套接字closesocket
	closesocket(_sock);

	//---------------------------
	WSACleanup();
	printf("已退出，任务结束");
	getchar();
	cout << "hello..." << endl;
	return 0;
}