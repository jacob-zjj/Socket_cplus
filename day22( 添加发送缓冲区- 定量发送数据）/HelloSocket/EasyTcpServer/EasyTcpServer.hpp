#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#ifdef _WIN32
	#define FD_SETSIZE	2506
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
//using namespace std;
#include "stdio.h"
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include "MessageHeader.hpp"
#include "iostream"
#include "CELLTimestamp.hpp"
#include <map>


//缓冲区最小单元大小
#ifndef RECV_BUFF_SIZE
#define RECV_BUFF_SIZE 10240 * 5
#define SEND_BUFF_SIZE RECV_BUFF_SIZE
#endif

//客户端数据类型
class ClientSocket
{
public:
	ClientSocket(SOCKET sockfd = INVALID_SOCKET) 
	{
		/*适合更老的c++编译器*/
		_sockfd = sockfd;
		memset(_szMsgBuf, 0, sizeof(RECV_BUFF_SIZE));
		_lastPos = 0;

		memset(_szSendBuf, 0, sizeof(SEND_BUFF_SIZE));
		_lastSendPos = 0;
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

	//发送指定sock数据
	int SendData(DataHeader* header)
	{
		int ret = SOCKET_ERROR;
		//要发送的长度
		int nSendLen = header->dataLength;
		//要发送的数据
		const char* pSendData = (const char*)header;
		while (true)
		{
			/******************************定量发送数据**********************************/
			if ((_lastSendPos + nSendLen) >= SEND_BUFF_SIZE)
			{
				//计算可以拷贝的数据长度
				int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//计算剩余数据位置
				pSendData += nCopyLen;
				//计算剩余数据长度
				nSendLen -= nSendLen;
				//发送数据
				ret = send(_sockfd, _szSendBuf, SEND_BUFF_SIZE, 0);
				//数据尾部位置为0
				_lastSendPos = 0;
				//发送错误 直接返回
				if (SOCKET_ERROR == ret)
				{
					return ret;
				}
			}
			else
			{
				//将要发送的数据 拷贝到发送缓冲区尾部
				memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
				//数据尾部位置置为
				_lastSendPos += nSendLen;
				break;
			}
			/****************************************************************************/
		}
		return ret;
		//
		//if (header)
		//{
		//	//定量发送数据
		//	return send(_sockfd, (const char*)header, header->dataLength, 0);
		//}	
	}
private:
	SOCKET _sockfd;//fdset文件描述符 file 
	//第二缓冲区 消息缓冲区
	char _szMsgBuf[RECV_BUFF_SIZE];
	//消息缓冲区的数据尾部位置
	int _lastPos;

	//第二缓冲区 发送缓冲区
	char _szSendBuf[RECV_BUFF_SIZE];
	//消息缓冲区的数据尾部位置
	int _lastSendPos;
};

//网络事件接口
class INetEvent
{
public:
	//纯虚函数
	//客户端加入事件
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	//客户端离开事件
	virtual void Onleave(ClientSocket* pClient) = 0;//纯虚函数
	//客户端消息事件
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header) = 0;
	//recv事件
	virtual void OnNetRecv(ClientSocket* pClient) = 0;
	//virtual void OnNetMsg(ClientSocket* pClient) = 0;
private:
};

//服务器
class CellServer
{
public:
	CellServer(SOCKET sock = INVALID_SOCKET)
	{
		_sock = sock;
		_pNetEvent = nullptr;
	}
	
	~CellServer()
	{
		Close();
		_sock = INVALID_SOCKET;
	}

	void setEventObj(INetEvent* event)
	{
		_pNetEvent = event;
	}
	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			for (auto iter : _clients)
			{
				closesocket(iter.second->sockfd());
				delete iter.second;
			}
			//关闭套节字
			closesocket(_sock);
#else
			for (auto iter : _clients)
			{
				close(iter.second->sockfd());
				delete iter.second;
			}
			//关闭套节字
			close(_sock);
#endif
			//清理一下
			_clients.clear();
		}
	}
	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//int _nCount = 0;
	//处理网络消息
	//备份客户socket fd_set
	fd_set _fdRead_bak;//备份
	bool _clients_change;
	SOCKET _maxSock;
	bool OnRun()
	{
		_clients_change = true;
		while (isRun())
		{
			if (_clientsBuff.size() > 0)
			{			//从缓冲区队列里取出客户数据
				std::lock_guard<std::mutex> lock(_mutex);
				//c++11中比较流行的遍历方法
				for (auto pClient : _clientsBuff)
				{
					_clients[pClient->sockfd()] = pClient;
					//_clients.push_back(pClient);
				}
				_clientsBuff.clear();
				_clients_change = true;
			}
			//如果没有需要处理的客户端就跳过
			if (_clients.empty())
			{
				//程序在此处暂停一毫秒
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			fd_set fdRead;//描述符(socket)集合 
			FD_ZERO(&fdRead);//清空fd_set集合类型的数据 其实就是将fd_count 置为0;(这个步骤必须要有，否则会发生不可预料的后果)
			//FD_SET(_sock, &fdRead);//将描述符加入集合中(在集合中新加入一个文件描述符)

			if (_clients_change)
			{
				_clients_change = false;
				//将描述符(socket)加入集合
				//_maxSock = _clients[0]->sockfd();
				_maxSock = _clients.begin()->second->sockfd();
				for (auto iter : _clients)
				{
					//将客户端SOCKET加入可读集合中
					/*优化该处 解决网络瓶颈*/
					FD_SET(iter.second->sockfd(), &fdRead);
					//FD_SET(_clients[n]->sockfd(), &fdRead);
					if (_maxSock < iter.second->sockfd())
					{
						//表示客户端的SOCKET 获得客户端SOCKET
						_maxSock = iter.second->sockfd();
					}
				}
				memcpy(&_fdRead_bak, &fdRead, sizeof(fd_set));
			}
			else
			{
				memcpy(&fdRead, &_fdRead_bak, sizeof(fd_set));
			}
			int ret = select(_maxSock + 1, &fdRead, nullptr, nullptr, nullptr);	 /*以上方式为阻塞方式，如果没有客户端进入将阻塞在此处*/
			if (ret < 0)
			{
				printf("<socket = %d>任务结束。\n", _sock);
				Close();
				return false;//表示出错 跳出循环
			}
			else if(ret == 0)
			{
				continue;
			}
			
#ifdef _WIN32
			for (int n = 0; n < fdRead.fd_count; n++)
			{
				auto iter = _clients.find(fdRead.fd_array[n]);
				if (iter != _clients.end())
				{
					if (-1 == RecvData(iter->second))
					{
						if (_pNetEvent)
						{
							_pNetEvent->Onleave(iter->second);
						}
						_clients_change = true;
						_clients.erase(iter->first);
					}
				}else{
					printf("error.if (iter !=_clients.end() )");
				}
			}
#else
			std::vector <ClientSocket *> temp;
			for (auto iter : _clients)
			{
				if (FD_ISSET(iter.second->sockfd(), &fdRead))
				{
					if (-1 == RecvData(iter.second))
					{
						if (_pNetEvent)
						{
							_pNetEvent->Onleave(iter.second);
						}
						_clients_change = false;
						temp.push_back(iter.second);
					}
				}
			}
			for (auto pClient : temp)
			{
				_clients.erase(pClient->sockfd());
				delete pClient;
			}
#endif
		}
	}

	//接收数据 处理粘包
	int RecvData(ClientSocket* pClient)
	{
		//5 接收客户端请求数据
		/*建立缓冲区 去掉后面的拷贝操作 减少系统资源开销*/
		char* szRecv = pClient->msgBuf() + pClient->getLastPos();
		//int nLen = (int)recv(pClient->sockfd(), _szRecv, RECV_BUFF_SIZE, 0);
		int nLen = (int)recv(pClient->sockfd(), szRecv, (RECV_BUFF_SIZE) - pClient->getLastPos(), 0);
		_pNetEvent->OnNetRecv(pClient);
		if (nLen <= 0)
		{
			printf("客户端<Socket = %d>已经退出, 任务结束。\n", pClient->sockfd());
			return -1;
		}
		//将收取的数据拷贝到消息缓冲区
		//memcpy(pClient->msgBuf() + pClient->getLastPos(), _szRecv, nLen);
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
				onNetMsg(pClient, header);
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
	virtual void onNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		_pNetEvent->OnNetMsg(pClient, header);
	}

	void addClient(ClientSocket* pClient)
	{
		//添加锁作用域
		//添加自解锁 - 加上命名空间比较准确
		std::lock_guard<std::mutex> lock(_mutex);
		_clientsBuff.push_back(pClient);
	}

	void Start()
	{
		//mem_fun更加安全的转换 mem_fn更灵活
		_pThread = std::thread(std::mem_fn(&CellServer::OnRun), this);
	}

	size_t getClientCount()
	{
		//已经取得的 和 缓冲队列中尚未取得的
		return _clients.size() + _clientsBuff.size();
	}

private:
	SOCKET _sock;
	//正式客户队列
	std::map<SOCKET, ClientSocket*> _clients;
	//缓冲客户队列 我们在程序执行的过程中逐步加入客户线程
	std::vector<ClientSocket*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	std::thread _pThread;//方便进行管理
	//网络事件对象  依赖关系
	INetEvent* _pNetEvent;
};

//new 堆内存(内存条)   一般变量则是放在栈空间中的
class EasyTcpServer : public INetEvent
{
private:
	SOCKET _sock;
	//消息处理对象 内部会创建线程
	std::vector<CellServer*> _cellServers;
	//创建一个计时器 每秒
	CELLTimestamp _tTime;
protected:
	//SOCKET recv计数
	std::atomic_int _recvCount;
	//收到消息
	std::atomic_int _msgCount;
	//客户端计数
	std::atomic_int _clientCount;
public:
	int getSock() {
		return this->_sock;
	}

public:
	EasyTcpServer()
	{
		_sock = INVALID_SOCKET;
		_recvCount = 0;
		_msgCount = 0;
		_clientCount = 0;
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
		//由于域作用符 和using namespace std的作用域相冲突
		int ret = ::bind(_sock, (sockaddr*)&_sin, sizeof(_sin));

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
			//将新客户端分配给客户端最小的cellserver
			addClientToCellServer(new ClientSocket(cSock));
			//获取IP地址 inet_ntoa(clientAddr.sin_addr)
		}
		return cSock;
	}

	void addClientToCellServer(ClientSocket* pClient)
	{
		//查找客户数量最少的CellServer消息处理线程对象
		auto pMinServer = _cellServers[0];
		for (auto pCellServer : _cellServers)
		{
			if (pMinServer->getClientCount() > pCellServer->getClientCount())
			{
				pMinServer = pCellServer;
			}
		}
		pMinServer->addClient(pClient);
		OnNetJoin(pClient);
	}

	void Start(int nCellServer)
	{
		for (int n = 0; n < nCellServer; n ++)
		{
			auto ser = new CellServer(_sock);
			_cellServers.push_back(ser);
			//注册网络事件
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}
	}

	//关闭Socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32
			closesocket(_sock);
			_sock = INVALID_SOCKET;
			WSACleanup();
#else
			close(_sock);

#endif
		}
	}

	//int _nCount = 0;
	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			time4msg();
			//cout << isRun() << endl;
			fd_set fdRead;//描述符(socket)集合
			FD_ZERO(&fdRead);//清空fd_set集合类型的数据 其实就是将fd_count 置为0
			FD_SET(_sock, &fdRead);//将描述符加入集合中
			SOCKET maxSock = _sock;
			timeval t = { 0, 10 };//时间变量 &t 最大为1秒
			int ret = select(maxSock + 1, &fdRead, 0, 0, &t);//NULL阻塞类型
			if (ret < 0)
			{
				printf("Accept Select任务结束。\n");
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
			return true;
		}
		return false;
	}

	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}

	//计算并输出每秒收到的网络信息
	virtual void time4msg()
	{
		auto t1 = _tTime.getElapsedSecond();
		//当达到一定的时间输出
		if (t1 >= 1.0)
		{
			printf("thread<%d>, time<%lf>, socket<%d>, clients<%d>, recv<%d>, msg<%d>\n", _cellServers.size(), t1, _sock, (int)_clientCount,(int)(_recvCount / t1), (int)(_msgCount / t1));
			_recvCount = 0;
			_msgCount = 0;
			_tTime.update();
		}
	}

	//只会被一个线程调用 安全
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
	}

	//cellServer 4 多个线程触发 不安全 如果只开启一个cellServer 就是安全的
	virtual void Onleave(ClientSocket* pClient)
	{
		_clientCount--;
	}

	//cellServer 4 多个线程触发 不安全
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		_recvCount++;
	}
};
#endif