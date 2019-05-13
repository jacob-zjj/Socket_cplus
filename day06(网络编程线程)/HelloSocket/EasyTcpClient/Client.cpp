#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <thread>
#include "iostream"
using namespace std;
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
		printf("与服务器断开连接, 任务结束。\n", _cSocket);
		return -1;
	}
	//printf("收到命令: %d 数据长度：%d\n", header.cmd, header.dataLength);
	/*判断所收到的数据*/ //多客户端进行收发数据的情况下使用
				 //if (nLen > sizeof(DataHeader))
				 //{
				 //}
	switch (header->cmd)
	{
		case CMD_LOGIN_RESULT:
		{
			recv(_cSocket, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LoginResult* login = (LoginResult*)szRecv;
			printf("收到服务端消息:CMD_LOGIN_RESULT 数据长度：%d \n", login->dataLength);
		}
		break;
		case CMD_LOGINOUT_RESULT:
		{
			recv(_cSocket, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			LoginOutResult* loginout = (LoginOutResult*)szRecv;
			printf("收到服务端消息:CMD_LOGINOUT_RESULT 数据长度：%d \n", loginout->dataLength);
		}
		break;
		case CMD_NEW_USER_JOIN:
		{
			recv(_cSocket, szRecv + sizeof(DataHeader), header->dataLength - sizeof(DataHeader), 0);
			NewUserJoin* userJoin = (NewUserJoin*)szRecv;
			printf("收到服务端消息:CMD_NEW_USER_JOIN 数据长度：%d \n", userJoin->dataLength);
		}
		break;
	}
}
bool g_bRun = true;
//将输入命令分离出来
void cmdThread(SOCKET sock) {
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
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "zjj");
			strcpy(login.PassWord, "969513");
			send(sock, (const char*)&login, sizeof(login), 0);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			LoginOut logout;
			strcpy(logout.userName, "zjj");
			send(sock, (const char*)&logout, sizeof(logout), 0);
		}
		else
		{
			printf("不支持的命令\n");
		}
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
	//用Socket API建立简易TCP客户端
	//1、建立一个socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == _sock)
	{
		printf("错误，建立Socket失败...\n");
	}
	else
	{
		printf("建立socket成功...\n");
	}

	//2、链接服务器 connect
	sockaddr_in _sin = {};//将结构体初始化
	_sin.sin_family = AF_INET;//ipv4
	_sin.sin_port = htons(4567);//将网络转换为成无符号类型
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in)); //使用sizeof(sockaddr_in)类型更安全
	if (SOCKET_ERROR == ret)
	{
		printf("错误，链接服务器失败...\n");
	}
	else
	{
		printf("链接服务器成功...\n");
	}

	//启动线程
	thread t1(cmdThread, _sock);
	t1.detach();//将其与主线程进行分离

	while (g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = { 1, 0 };
		int ret = select(_sock, &fdReads, 0, 0, &t);
		if (ret < 0)
		{
			printf("select任务结束！！！\n");
			break;
		}
		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (processor(_sock) == -1)
			{
				printf("select任务结束2\n");
				break;
			}
		}
		//线程thread 如果在程序执行的过程中加入scanf那么程序将会阻塞
		//printf("空闲时间处理其他业务\n");
		//Sleep(1000);
	}

	//7 关闭套节字closesocket
	closesocket(_sock);
	//---------------------------
	WSACleanup();
	printf("已退出.\n");
	getchar();
	return 0;
}