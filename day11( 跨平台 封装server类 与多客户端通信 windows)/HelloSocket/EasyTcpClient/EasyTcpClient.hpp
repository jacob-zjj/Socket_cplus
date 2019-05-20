#ifndef _EasyTcpClient_hpp_ 
#define _EasyTcpClient_hpp_
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
#include "stdio.h"
#include "MessageHeader.hpp"
class EasyTcpClient 
{
	SOCKET _sock;
public:
	EasyTcpClient() 
	{
		_sock = INVALID_SOCKET;
	}
	/*虚析构函数*/
	virtual ~EasyTcpClient() 
	{
		Close();
	}

	/*初始化socket*/
	void initSocket() 
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
			printf("<socket = %d>关闭旧链接...\n", _sock);
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
	}

	/*链接服务器*/
	//此处加一个常量标识符 为了避免在linux中的报错
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock)
		{
			initSocket();
		}
		//2、链接服务器 connect
		sockaddr_in _sin = {};//将结构体初始化
		_sin.sin_family = AF_INET;//ipv4
		_sin.sin_port = htons(port);//将网络转换为成无符号类型
#ifdef _WIN32
		_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		_sin.sin_addr.s_addr = inet_addr(ip);
#endif
		printf("<socket=%d>正在连接服务器...\n", _sock);
		int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)); //使用sizeof(sockaddr_in)类型更安全
		if (SOCKET_ERROR == ret)
		{
			printf("<socket=%d>错误，链接服务器<%s : %d>失败...\n", _sock, ip, port);
		}
		else
		{
			printf("<socket=%d>链接服务器<%s : %d>成功...\n", _sock, ip, port);
		}
		return ret;
	}

	/*关闭socket*/
	void Close() 
	{
		//防止重复调用
		if (_sock != INVALID_SOCKET)
		{
			//关闭win socket 2.x环境
#ifdef _WIN32
			closesocket(_sock);//windows下使用
			WSACleanup();
#else
			close(_sock);
#endif
			_sock = INVALID_SOCKET;
		}

	}

	/*处理网络信息*/
	bool OnRun() 
	{
		if (isRun())
		{
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0, 0 };
			int ret = select(_sock + 1, &fdReads, 0, 0, &t);
			if (ret < 0)
			{
				printf("<socket = %d>任务结束1！！！\n", _sock);
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads))
			{
				FD_CLR(_sock, &fdReads);
				if (RecvData(_sock) == -1)
				{
					printf("<socket = %d>任务结束2！！！\n", _sock);
					Close();
					return false;
				}
			}
			return true;
		}
		return false;
	}

	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	/*接收数据 处理粘包 拆分包*/
	int RecvData(SOCKET _cSocket)
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
			printf("<socket = %d>与服务器断开连接, 任务结束。\n", _cSocket);
			return -1;
		}
		recv(_cSocket, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
		onNetMsg(header);
		return 0;
	}

	/*响应*/
	virtual void onNetMsg(DataHeader* header)
	{
		//printf("收到命令: %d 数据长度：%d\n", header.cmd, header.dataLength);
		/*判断所收到的数据*/ //多客户端进行收发数据的情况下使用
		switch (header->cmd)
		{
		case CMD_LOGIN_RESULT:
		{
			LoginResult* login = (LoginResult*)header;
			printf("<socket = %d>收到服务端消息:CMD_LOGIN_RESULT 数据长度：%d \n", _sock, login->dataLength);
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{
			LoginOutResult* loginout = (LoginOutResult*)header;
			printf("<socket = %d>收到服务端消息:CMD_LOGINOUT_RESULT 数据长度：%d \n", _sock, loginout->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			NewUserJoin* userJoin = (NewUserJoin*)header;
			printf("<socket = %d>收到服务端消息:CMD_NEW_USER_JOIN 数据长度：%d \n", _sock, userJoin->dataLength);
		}
		break;
		}
	}
	
	//发送数据
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
private:
};
#endif