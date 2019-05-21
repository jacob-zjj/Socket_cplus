#include "EasyTcpClient.hpp"
#include <thread>
#include "iostream"
using namespace std;
bool g_bRun = true;
//将输入命令分离出来
void cmdThread() {
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

//一个客户端同时链接多个服务器 以另外一种方式替换多线程方式
//客户端自己向服务器发数据
int main()
{
	const int cCount = 2000;
	//使用数组方式 已经超出了c++的栈内存 因此将其变成指针类型即可
	EasyTcpClient* client[cCount];
	for (int n = 0; n < cCount; n++)
	{
		if (!g_bRun)
		{
			break;
		}
		client[n] = new EasyTcpClient();
	}
	for (int n = 0; n < cCount; n++)
	{
		if (!g_bRun)
		{
			return 0;
		}
		client[n]->Connect("127.0.0.1", 4567);
		printf("Connect = %d\n", n);
	}
	thread t1(cmdThread);
	t1.detach();
	Login login;
	strcpy(login.userName, "zjj");
	strcpy(login.PassWord, "969513");
	/*while (client.isRun() || client2.isRun())*/
	while (g_bRun)
	{
		for (int n = 0; n < cCount; n++)
		{
			client[n]->SendData(&login);
			//client[n]->OnRun();
		}
	}
	for (int n = 0; n < cCount; n++)
	{
		client[n]->Close();
	}
	printf("已退出.\n");
	return 0;
}