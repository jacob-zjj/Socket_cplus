#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include "iostream"
using namespace std;
enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
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
	while (true)
	{
		//3 输入请求命令
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);

		//4 处理请求命令
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("收到exit命令，任务结束\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "zjj");
			strcpy(login.PassWord, "969513");
			//DataHeader dh = {sizeof(login), CMD_LOGIN};
			//login.userName = "lyd";
			//login.PassWord = "lydmm";
			//5 向服务器发送请求命令
			//send(_sock, (const char*)&dh, sizeof(dh), 0);
			send(_sock, (const char*)&login, sizeof(login), 0);
			//接收服务器返回的数据
			//DataHeader retHeader = {};
			LoginResult loginRet;
			//recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&loginRet, sizeof(loginRet), 0);
			printf("LoginResult: %d\n", loginRet.result);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			LoginOut logout;
			strcpy(logout.userName, "zjj");
			//DataHeader dh = {sizeof(logout), CMD_LOGINOUT };
			//5 向服务器发送请求命令
			//send(_sock, (const char*)&dh, sizeof(dh), 0);
			send(_sock, (const char*)&logout, sizeof(logout), 0);
			//接收服务器返回的数据
			//DataHeader retHeader = {};
			LoginOutResult loginoutRet;
			//recv(_sock, (char*)&retHeader, sizeof(retHeader), 0);
			recv(_sock, (char*)&loginoutRet, sizeof(loginoutRet), 0);
			printf("LoginResult: %d\n", loginoutRet.result);
		}
		else
		{
			printf("不支持的命令，请重新输入。\n");
		}
	}

	//7 关闭套节字closesocket
	closesocket(_sock);
	//---------------------------
	WSACleanup();
	printf("已退出.\n");
	getchar();
	return 0;
}