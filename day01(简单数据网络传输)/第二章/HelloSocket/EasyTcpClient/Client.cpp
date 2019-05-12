#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include "iostream"
using namespace std;
//加入windows的静态链接库--在windows里面支持这种写法
//最好还是在属性设置中去加入ws2_32.lib
//#pragma comment(lib, "ws2_32.lib")
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
	_sin.sin_family = AF_INET;
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
			printf("收到exit命令，任务结束");
			break;
		}
		else
		{
			/*send(
			_In_ SOCKET s,
			_In_reads_bytes_(len) const char FAR * buf,
			_In_ int len,
			_In_ int flags
			);*/
			//5 向服务器发送请求命令
			send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
		}

		//6 接收服务器信息 recv
		char recvBuf[128] = {};
		int nlen = recv(_sock, recvBuf, 128, 0);
		if (nlen > 0)
		{
			printf("接收到数据: %s\n", recvBuf);
		}
	}

	//7 关闭套节字closesocket
	closesocket(_sock);

	//---------------------------
	WSACleanup();
	printf("已退出.");
	getchar();
	return 0;
}