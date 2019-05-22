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

int main()
{
	/*添加功能 -> 加入线程使得服务器可以像 客户端一样通过输入exit退出*/
	EasyTcpServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
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