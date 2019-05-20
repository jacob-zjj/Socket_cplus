#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#ifdef _WIN32
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <WinSock2.h>
#else
	#include <unistd.h> //uni std
	#include <arpa/inet.h> 
	#include <string.h>
	#define SOCKET int 
	#define INVALID_SOCKET   (SOCKET)(~0)
	#define SOCKET_ERROR             (-1)
#endif
using namespace std;
#include "stdio.h"
#include <vector>
#include "MessageHeader.hpp"
#include "iostream"

class EasyTcpServer
{
private:
	SOCKET _sock;
	vector<SOCKET> g_clients;
public:
	int getSock() {
		return this->_sock;
	}
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
	}

	virtual ~EasyTcpServer()
	{
		Close();
	}

	//初始化Socket
	SOCKET InitSocket()
	{
#ifdef _WIN32
		/*启动Windows socket 2.x环境*/
		//版本号
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		//socket网络编程启动函数
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock)
		{
			printf("<socket = %d>关闭旧链接...\n", (int)_sock);
			//如果有链接先关闭
			Close();
		}
		//建立一个socket
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock)
		{
			printf("错误，建立Socket失败...\n");
		}
		else
		{
			printf("建立socket=<%d>成功...\n", _sock);
		}
		return _sock;
	}

	//绑定IP 和 端口号
	int Bind(const char* ip, unsigned short port)
	{
		/*if (INVALID_SOCKET == _sock)
		{
			InitSocket();
		}*/
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;//ipv4
		_sin.sin_port = htons(port);//端口号 由于主机和网络的数据类型不同 因此需要进行转换 使用 host to net unsigned short
#ifdef _WIN32
		if (ip)
		{
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");//服务器的ip地址 INADDR_ANY本机所有的Ip地址都可以访问 一般这样
		}
#else
		if (ip)
		{
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");//服务器的ip地址 INADDR_ANY本机所有的Ip地址都可以访问 一般这样
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));

		//有可能绑定失败
		if (SOCKET_ERROR == ret)
		{
			printf("错误，绑定网络端口<%d>失败...\n", port);
		}
		else
		{
			printf("绑定端口<%d>成功...\n", port);
		}
		return ret;
	}

	//监听端口号
	int Listen(int n)
	{
		//监听端口
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret)
		{
			printf("socket = <%d>错误，监听网络端口失败...\n", (int)_sock);
		}
		else
		{
			printf("socket = <%d>监听网络端口成功...\n", (int)_sock);
		}
		return ret;
	}

	//接收客户端链接
	int Accept()
	{
		//accept 等待接受客户端连接
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET _cSocket = INVALID_SOCKET;
		//nAddrLen类型是windows和linux之间不一样的
#ifdef _WIN32
		_cSocket = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		_cSocket = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
		if (INVALID_SOCKET == _cSocket)
		{
			printf("socket<%d>错误，接收到无效客户端SOCKET...\n", (int)_sock);
		}
		else
		{
			NewUserJoin userJoin;
			SendDataToAll(&userJoin);
			g_clients.push_back(_cSocket);
			printf("socket<%d>新客户端加入: socket = %d IP = %s \n", (int)_sock, (int)_cSocket, inet_ntoa(clientAddr.sin_addr));//inet_ntoa转换为可读地址
		}
		return _cSocket;
	}

	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				closesocket(g_clients[n]);
			}
			closesocket(_sock);
			_sock = INVALID_SOCKET;
			WSACleanup();
#else
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				close(g_clients[n]);
			}
			close(_sock);
#endif
		}
	}

	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			//cout << isRun() << endl;
			fd_set fdRead;//描述符(socket)集合
			fd_set fdWrite;
			fd_set fdExp;
			FD_ZERO(&fdRead);//清空fd_set集合类型的数据 其实就是将fd_count 置为0
			FD_ZERO(&fdWrite);//清理集合
			FD_ZERO(&fdExp);
			FD_SET(_sock, &fdRead);//将描述符加入集合中
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			SOCKET maxSock = _sock;
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(g_clients[n], &fdRead);//放入可读数据中查询 是否有可读数据
				if (maxSock < g_clients[n])
				{
					//找到所有描述符中的最大描述符
					maxSock = g_clients[n];
				}
			}
			timeval t = { 0, 0 };//时间变量 &t 最大为1秒
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			/*_sock + 1能够 使得在linux上正常使用*/
			/*以上方式为阻塞方式，如果没有客户端进入将阻塞在此处*/
			if (ret < 0)
			{
				printf("<socket = %d>任务结束。\n", _sock);
				Close();
				return false;//表示出错 跳出循环
			}
			//如果这个socket可读的话表示 有客户端已经连接进来了
			if (FD_ISSET(_sock, &fdRead))//判断描述符是否在集合中
			{
				//清理
				FD_CLR(_sock, &fdRead);
				Accept();
			}
			for (int n = (int)g_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(g_clients[n], &fdRead))
				{
					if (RecvData(g_clients[n]) == -1)
					{
						auto iter = g_clients.begin() + n;
						if (iter != g_clients.end())
						{
							g_clients.erase(iter);
						}
					}
				}
			}
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//接收数据 处理粘包
	int RecvData(SOCKET _cSocket)
	{
		//使用缓冲区来接受数据
		char szRecv[1024] = {};
		//5 接收客户端请求数据
		int nLen = (int)recv(_cSocket, (char*)&szRecv, sizeof(DataHeader), 0);
		//拆包 和 分包
		/*拆包和分包的作用主要是用在服务端接受数据时一次接受数据过长 和 过短的情况*/
		DataHeader* header = (DataHeader*)szRecv;
		if (nLen <= 0)
		{
			printf("客户端<Socket = %d>已经退出, 任务结束。\n", _cSocket);
			return -1;
		}
		recv(_cSocket, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		onNetMsg(_cSocket, header);
		return 0;
	}

	//响应网络消息
	virtual void onNetMsg(SOCKET _cSocket, DataHeader* header)
	{
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			
			Login* login = (Login*)header;
			printf("收到客户端<Socket = %d>请求:CMD_LOGIN 数据长度：%d userName = %s passWord = %s\n", _cSocket, login->dataLength, login->userName, login->PassWord);
			LoginResult ret;
			send(_cSocket, (char*)&ret, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{
			LoginOut *loginout = (LoginOut*)header;
			printf("收到命令:CMD_LOGINOUT 数据长度：%d userName = %s\n", loginout->dataLength, loginout->userName);
			LoginOutResult ret;
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

	//发送指定sock数据
	int SendData(SOCKET _cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	//群发消息
	void SendDataToAll(DataHeader* header)
	{
		//有新客户端加入群发给所有用户 类似像聊天室 或者狼人杀类型
		for (int n = (int)g_clients.size() - 1; n >= 0; n--)
		{
			SendData(g_clients[n], header);
		}
	}
};
#endif