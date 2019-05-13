#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include "iostream"
#include "stdio.h"
using namespace std;
/*使用报文的方式进行传输*/
//数据头
enum CMD
{
	CMD_LOGIN, 
	CMD_LOGINOUT,
	CMD_ERROR
};
struct DataHeader
{
	short dataLength;//数据长度
	short cmd;//命令
};
//DataPackage
//包体
struct Login 
{
	char userName[32];
	char PassWord[32];
};
struct LoginResult
{
	int result;
};
struct LoginOut
{
	char userName[32];
};
struct LoginOutResult
{
	int result;
};

int main() 
{
	/*启动Windows socket 2.x环境*/
	//版本号
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	//socket网络编程启动函数
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
	char msgBuf[] = "Hello, I'm Server.";
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

	_cSocket = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
	if (INVALID_SOCKET == _cSocket)
	{
		printf("错误，接收到无效客户端SOCKET...\n");
	}
	printf("新客户端加入: socket = %d IP = %s \n", _cSocket, inet_ntoa(clientAddr.sin_addr));//inet_ntoa转换为可读地址
	//一直不停的收发数据
	//char _recvBuf[128] = {};//接收缓冲区
	while (true)
	{
		//使用header来接受客户端传过来的命令数据 通过命令判断客户端的需求
		DataHeader header = {};
		//5 接收客户端请求数据
		int nLen = recv(_cSocket, (char*)&header, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			printf("客户端已经退出, 任务结束。");
			break;
		}
		printf("收到命令: %d 数据长度：%d\n", header.cmd, header.dataLength);
		switch (header.cmd)
		{
			case CMD_LOGIN:
				{
					Login login = {};
					recv(_cSocket, (char*)&login, sizeof(login), 0);
					//忽略判断用户名密码是否正确的过程
					LoginResult ret = { 1 };
					send(_cSocket, (char*)&header, sizeof(DataHeader), 0);
					send(_cSocket, (char*)&ret, sizeof(LoginResult), 0);
				}
				break;
			case CMD_LOGINOUT:
				{
					LoginOut loginout = {};
					recv(_cSocket, (char*)&loginout, sizeof(LoginOut), 0);
					//忽略判断用户名密码是否正确的过程
					LoginOutResult ret = { 1 };
					send(_cSocket, (char*)&header, sizeof(DataHeader), 0);
					send(_cSocket, (char*)&ret, sizeof(LoginOutResult), 0);
				}
				break;
			default:
				header.cmd = CMD_ERROR;
				header.dataLength = 0;
				send(_cSocket, (char*)&header, sizeof(DataHeader), 0);
			break;
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