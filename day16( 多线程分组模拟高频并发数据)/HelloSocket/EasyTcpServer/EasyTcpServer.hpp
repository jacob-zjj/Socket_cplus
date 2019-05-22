#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#ifdef _WIN32
	#define FD_SETSIZE	1024
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
#include "CELLTimestamp.hpp"
//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240
#endif
//建立一个新的socket模型
class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) 
	{
		/*适合更老的c++编译器*/
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
		_lastPos = 0;
	}
	SOCKET sockfd()
	{
		return _sockfd;
	}
	char* msgBuf()
	{
		return _szMsgBuf;
	}
	int getLastPos()
	{
		return _lastPos;
	}
	void setLastPos(int pos)
	{
		this->_lastPos = pos;
	}
private:
	SOCKET _sockfd;//fdset文件描述符 file 
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE * 10];
	//消息缓冲区的数据尾部位置
	int _lastPos;
};

//new 堆内存(内存条)   一般变量则是放在栈空间中的
class EasyTcpServer
{
private:
	SOCKET _sock;
	vector<ClientSocket*> _clients;
	//创建一个计时器
	CELLTimestamp _tTime;
	int _recvCount;
public:
	int getSock() {
		return this->_sock;
	}
public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
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
		SOCKET cSock = INVALID_SOCKET;
		//nAddrLen类型是windows和linux之间不一样的
#ifdef _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
		cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
		if (INVALID_SOCKET == cSock)
		{
			printf("socket<%d>错误，接收到无效客户端SOCKET...\n", (int)_sock);
		}
		else
		{
			//NewUserJoin userJoin;
			//SendDataToAll(&userJoin);
			_clients.push_back(new ClientSocket(cSock));
			//printf("socket<%d>新客户端<%d>加入: socket = %d IP = %s \n", (int)_sock, _clients.size(), (int)cSock, inet_ntoa(clientAddr.sin_addr));//inet_ntoa转换为可读地址
		}
		return cSock;
	}

	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				closesocket(_clients[n]->sockfd());
				delete _clients[n];
			}
			closesocket(_sock);
			_sock = INVALID_SOCKET;
			WSACleanup();
#else
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				close(_clients[n]->sockfd());
				delete _clients[n];
			}
			close(_sock);
			_sock = INVALID_SOCKET;
#endif
			//清理一下
			_clients.clear();
		}
	}
	//int _nCount = 0;
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
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				FD_SET(_clients[n]->sockfd(), &fdRead);//放入可读数据中查询 是否有可读数据
				if (maxSock < _clients[n]->sockfd())
				{
					//找到所有描述符中的最大描述符
					maxSock = _clients[n]->sockfd();
				}
			}

			timeval t = { 1, 0 };//时间变量 &t 最大为1秒
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);//NULL阻塞类型
			//printf("select ret =  %d count = %d\n", ret, _nCount ++);
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
				//在链接的时候不去发送数据 这样加快链接速度
				return true;
			}
			for (int n = (int)_clients.size() - 1; n >= 0; n--)
			{
				if (FD_ISSET(_clients[n]->sockfd(), &fdRead))
				{
					if (RecvData(_clients[n]) == -1)
					{
						auto iter = _clients.begin() + n;
						if (iter != _clients.end())
						{
							delete _clients[n];
							_clients.erase(iter);
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
	//使用缓冲区来接受数据
	char _szRecv[RECV_BUFF_SIZE] = {};

	//接收数据 处理粘包
	int RecvData(ClientSocket* pClient)
	{
		//5 接收客户端请求数据
		int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		if (nLen <= 0)
		{
			printf("客户端<Socket = %d>已经退出, 任务结束。\n", pClient->sockfd());
			return -1;
		}
		//将收取的数据拷贝到消息缓冲区
		memcpy( pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
		//将消息缓冲区的数据尾部位置后移
		pClient->setLastPos(pClient->getLastPos() + nLen);

		//判断消息缓冲区的数据长度大于消息头DataHeader长度
		while (pClient->getLastPos() >= sizeof(DataHeader))//循环是为了解决粘包
		{
			//这时就可以知道当前消息的长度
			DataHeader* header = (DataHeader*)pClient->msgBuf();
			//判断消息缓冲区的数据长度大于消息长度
			if (pClient->getLastPos() >= header->dataLength)
			{
				//剩余未处理消息缓冲区数据的长度
				int nSize = pClient->getLastPos() - header->dataLength;
				//处理网络消息
				onNetMsg(pClient->sockfd(), header);
				//将消息缓冲区剩余未处理数据前移
				memcpy(pClient->msgBuf(), pClient->msgBuf() + header->dataLength, nSize);
				//消息缓冲区的尾部位置前移
				pClient->setLastPos(nSize);
			}
			else
			{
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}
		return 0;
	}

	//响应网络消息
	virtual void onNetMsg(SOCKET cSock, DataHeader* header)
	{
		_recvCount++;
		auto t1 = _tTime.getElapsedSecond();
		//当达到一定的时间输出
		if (t1 >= 1.0)
		{
			printf("time<%lf>, socket<%d>, clients<%d>, _recvCount<%d>\n", t1, _sock, _clients.size(), _recvCount);
			_recvCount = 0;
			_tTime.update();
		}
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			
			Login* login = (Login*)header;
			//printf("收到客户端<Socket = %d>请求:CMD_LOGIN 数据长度：%d userName = %s passWord = %s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//LoginResult ret;
			//send(cSock, (char*)&ret, sizeof(LoginResult), 0);
			//SendData(cSock, &ret);
		}
		break;
		case CMD_LOGINOUT:
		{
			LoginOut *loginout = (LoginOut*)header;
			//printf("收到命令:CMD_LOGINOUT 数据长度：%d userName = %s\n", loginout->dataLength, loginout->userName);
			//LoginOutResult ret;
			//send(cSock, (char*)&ret, sizeof(LoginOutResult), 0);
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			printf("<socket = %d>收到未定义消息 数据长度：%d \n", cSock, header->dataLength);
			//DataHeader ret;
			//send(cSock, (char*)&header, sizeof(DataHeader), 0);
			//SendData(cSock, &ret);
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
		for (int n = (int)_clients.size() - 1; n >= 0; n--)
		{
			SendData(_clients[n]->sockfd(), header);
		}
	}
};
#endif