#include "EasyTcpServer.hpp"
#include <thread>
#include "iostream"
using namespace std;

bool g_bRun = true;
//将输入命令分离出来
void cmdThread1() {
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else
		{
			printf("不支持的命令\n");
		}
	}
}

class MyServer : public EasyTcpServer
{
public:
	//只会被一个线程调用 安全
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
		//printf("client<%d> join\n", pClient->sockfd());
	}

	//cellServer 4 多个线程触发 不安全 如果只开启一个cellServer 就是安全的
	virtual void Onleave(ClientSocket* pClient)
	{
		_clientCount--;
		//printf("client<%d> leave\n", pClient->sockfd());
	}

	//cellServer 4 多个线程触发 不安全
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		_msgCount++;
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{
			Login* login = (Login*)header;
			//printf("收到客户端<Socket = %d>请求:CMD_LOGIN 数据长度：%d userName = %s passWord = %s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//LoginResult ret;
			//send(cSock, (char*)&ret, sizeof(LoginResult), 0);
			//pClient->SendData(&ret);
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
			printf("<socket = %d>收到未定义消息 数据长度：%d \n", pClient->sockfd(), header->dataLength);
			//DataHeader ret;
			//send(cSock, (char*)&header, sizeof(DataHeader), 0);
			//SendData(cSock, &ret);
		}
		break;
		}
	}
	virtual void OnNetRecv(ClientSocket* pClient)
	{
		_recvCount++;
		//printf("client<%d> leave\n", pClient->sockfd());
	}
private:
};

int main()
{
	/*添加功能 -> 加入线程使得服务器可以像 客户端一样通过输入exit退出*/
	//EasyTcpServer server;
	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(4);
	thread t(cmdThread1);
	t.detach();

	/*EasyTcpServer server1;
	server1.InitSocket();
	server1.Bind(nullptr, 4568);
	server1.Listen(5);
	thread t1(cmdThread1, &server1);
	t1.detach();*/

	/*while (server.isRun() || server1.isRun())*/
	while (g_bRun)
	{
		server.OnRun();
		//server1.OnRun();
		//printf("空闲时间处理其他业务...\n");
	}
	server.Close();
	//server1.Close();
	printf("已退出。\n");
	getchar();
	return 0;
}