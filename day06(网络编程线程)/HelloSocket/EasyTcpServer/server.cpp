#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include "iostream"
#include "stdio.h"
#include <vector>
using namespace std;
/*使用报文的方式进行传输*/
//数据头
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR
};
struct DataHeader
{
	short dataLength;//数据长度 
	short cmd;//命令
};
//DataPackage
//包体
struct Login : public DataHeader
{
	//DataHeader header;
	Login() 
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
};
struct LoginResult : public DataHeader
{
	LoginResult()
	{
		dataLength = sizeof(LoginResult);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
};
struct LoginOut : public DataHeader
{
	LoginOut()
	{
		dataLength = sizeof(LoginOut);
		cmd = CMD_LOGINOUT;
	}	
	char userName[32];
};
struct LoginOutResult : public DataHeader
{
	LoginOutResult()
	{
		dataLength = sizeof(LoginOutResult);
		cmd = CMD_LOGINOUT_RESULT;
		result = 0;
	}
	int result;
};
//新客户端加入
struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		sock = 0;
	}
	int sock;
};
//创建一个全局的容器
vector<SOCKET> g_clients;

//创建一个函数进行处理
int processor(SOCKET _cSocket)
{
	//使用缓冲区来接受数据
	char szRecv[1024] = {};
	//5 接收客户端请求数据
	int nLen = recv(_cSocket, (char*)&szRecv, sizeof(DataHeader), 0);
	//拆包 和 分包
	/*拆包和分包的作用主要是用在服务端接受数据时一次接受数据过长 和 过短的情况*/
	DataHeader* header = (DataHeader*)szRecv;
	if (nLen <= 0)
	{
		printf("客户端<Socket = %d>已经退出, 任务结束。\n", _cSocket);
		return -1;
	}
	//printf("收到命令: %d 数据长度：%d\n", header.cmd, header.dataLength);
	/*判断所收到的数据*/ //多客户端进行收发数据的情况下使用
	//if (nLen > sizeof(DataHeader))
	//{
	//}
	switch (header->cmd)
	{
		case CMD_LOGIN:
		{
			recv(_cSocket, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			Login* login = (Login*)szRecv;
			printf("收到客户端<Socket = %d>请求:CMD_LOGIN 数据长度：%d userName = %s passWord = %s\n",_cSocket, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户名密码是否正确的过程
			LoginResult ret;
			//send(_cSocket, (char*)&header, sizeof(DataHeader), 0);
			send(_cSocket, (char*)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{
			recv(_cSocket, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LoginOut *loginout = (LoginOut*)szRecv;
			printf("收到命令:CMD_LOGINOUT 数据长度：%d userName = %s\n", loginout->dataLength, loginout->userName);
			//忽略判断用户名密码是否正确的过程
			LoginOutResult ret;
			//send(_cSocket, (char*)&header, sizeof(DataHeader), 0);
			send(_cSocket, (char*)&ret, sizeof(LoginOutResult), 0);
		}
		break;
		default:
		{
			DataHeader header = { 0, CMD_ERROR };
			send(_cSocket, (char*)&header, sizeof(DataHeader), 0);
		}
		break;
	}
}

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
	char msgBuf[] = "Hello, I'm Server.\n";
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
	while (true)
	{
		// 伯克利 BSD socket windows上第一个参数无意义
		//linux 表示描述符加1
		/*处理多客户端类型
		select(
		_In_ int nfds,
		_Inout_opt_ fd_set FAR * readfds,//可读
		_Inout_opt_ fd_set FAR * writefds,//可写
		_Inout_opt_ fd_set FAR * exceptfds,//异常
		_In_opt_ const struct timeval FAR * timeout//查询延迟
		);*/
		fd_set fdRead;//描述符(socket)集合
		fd_set fdWrite;
		fd_set fdExp;

		FD_ZERO(&fdRead);//清空fd_set集合类型的数据 其实就是将fd_count 置为0
		FD_ZERO(&fdWrite);//清理集合
		FD_ZERO(&fdExp);
		//typedef struct fd_set {
		//	u_int fd_count; 数量              /* how many are SET? */
		//	SOCKET  fd_array[FD_SETSIZE(64)];   /* an array of SOCKETs */
		//} fd_set;

		FD_SET(_sock, &fdRead);//将描述符加入集合中
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExp);
		for (int n = (int)g_clients.size() - 1; n >= 0; n --)
		{
			FD_SET(g_clients[n], &fdRead);//放入可读数据中查询 是否有可读数据
		}
		//nfds是一个整数值 是指fd_set集合中所有描述符(socket)的范围 而不是数量 
		//既是所有文件描述符最大值+1 在windows中这个参数可以写0
		//最后一个参数写成NULL表示一直阻塞在此等待
		timeval t = {1, 0};//时间变量 &t 最大为1秒
		//struct timeval {
		//long    tv_sec;         /* seconds */
		//long    tv_usec;        /* and microseconds */};
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExp, &t);
		/*以上方式为阻塞方式，如果没有客户端进入将阻塞在此处*/
		if (ret < 0)
		{
			printf("select任务结束。\n");
			break;//表示出错 跳出循环
		}
		//如果这个socket可读的话表示 有客户端已经连接进来了
		if (FD_ISSET(_sock, &fdRead))//判断描述符是否在集合中
		{
			//清理
			FD_CLR(_sock, &fdRead);
			//4、accept 等待接收客户端链接
			/*
			accept(
			_In_ SOCKET s,
			_Out_writes_bytes_opt_(*addrlen) struct sockaddr FAR * addr,
			_Inout_opt_ int FAR * addrlen
			);*/
			//accept 等待接受客户端连接
			sockaddr_in clientAddr = {};
			int nAddrLen = sizeof(sockaddr_in);
			SOCKET _cSocket = INVALID_SOCKET;
			_cSocket = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
			if (INVALID_SOCKET == _cSocket)
			{
				printf("错误，接收到无效客户端SOCKET...\n");
			}
			else
			{
				//有新客户端加入群发给所有用户 类似像聊天室 或者狼人杀类型
				for (int n = (int)g_clients.size() - 1; n >= 0; n--)
				{
					NewUserJoin userJoin;
					//发送给现有的每个客户端
					send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
				}
				g_clients.push_back(_cSocket);
				printf("新客户端加入: socket = %d IP = %s \n", _cSocket, inet_ntoa(clientAddr.sin_addr));//inet_ntoa转换为可读地址
			}
			
		}
		for (size_t n = 0; n < fdRead.fd_count; n++)
		{
			if (processor(fdRead.fd_array[n]) == -1)
			{
				auto iter = find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
		//printf("空闲时间处理其他业务\n");
	}
	//以防万一退出程序时 将所有套接字进行清理
	for (size_t n = g_clients.size() - 1; n > 0; n --)
	{
		closesocket(g_clients[n]);
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
	printf("已退出，任务结束\n");
	getchar();
	return 0;
}