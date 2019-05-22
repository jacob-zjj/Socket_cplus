#include "EasyTcpClient.hpp"
#include <thread>
#include "iostream"
using namespace std;
bool g_bRun = true;

//客户端数量
const int cCount = 1000;
//发送线程数量
const int tCount = 4;
//客户端数组
EasyTcpClient* client[cCount];

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
void sendThread(int id) //4个线程 1 - 4
{
	//ID 1-4
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;
	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}
	for (int n = begin; n < end; n++)
	{
		client[n]->Connect("127.0.0.1", 4567);
		printf("Connect = %d\n", n);
	}
	Login login;
	strcpy(login.userName, "zjj");
	strcpy(login.PassWord, "969513");
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			client[n]->SendData(&login);
			//client[n]->OnRun();
		}
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
	}
}

//一个客户端同时链接多个服务器 以另外一种方式替换多线程方式
//客户端自己向服务器发数据
int main()
{
	thread t1(cmdThread);
	t1.detach();

	//启动发送线程
	for (int n = 0; n < tCount; n ++)
	{
		thread t1(sendThread, n + 1);
		t1.detach();
	}
	while (g_bRun)
	{
		Sleep(100);
	}
	printf("已退出.\n");
	return 0;
}