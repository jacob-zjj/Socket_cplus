#include "EasyTcpClient.hpp"
#include "CELLTimestamp.hpp"
#include <thread>
#include "iostream"
#include <atomic>
bool g_bRun = true;

//客户端数量
const int cCount = 8;

//发送线程数量
const int tCount = 4;

//客户端数组
EasyTcpClient* client[cCount];
std::atomic_int sendCount = 0;
std::atomic_int readyCount = 0;

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
	printf("thread<%d>, start\n", id);
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
		client[n]->Connect("101.132.170.175", 3389);
		//client[n]->Connect("127.0.0.1", 4567);
	}
	printf("thread<%d>, Connect<begin=%d, end=%d>\n", id, begin, end);
	
	//四个线程都准备好 就会跳过这个循环一起发数据
	readyCount++;
	while (readyCount < tCount)
	{	
		//等待其他线程准备 发并发发送数据
		std::chrono::milliseconds t(10);
		std::this_thread::sleep_for(t);
	}

	Login login[1];
	for (int n = 0; n < 10; n++)
	{
		strcpy(login[n].userName, "zjj");
		strcpy(login[n].PassWord, "969513");
	}

	const int nLen = sizeof(login);
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			if (client[n]->SendData(login, nLen) != SOCKET_ERROR)
			{
				sendCount++;
			}//client[n]->OnRun();
		}
	}
	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
		delete client[n];
	}
	printf("thread<%d>, exit\n", id);
}

//一个客户端同时链接多个服务器 以另外一种方式替换多线程方式
//客户端自己向服务器发数据
int main()
{
	std::thread t1(cmdThread);
	t1.detach();
	//启动发送线程
	for (int n = 0; n < tCount; n ++)
	{
		std::thread t1(sendThread, n + 1);
		t1.detach();
	}
	CELLTimestamp tTime;
	while (g_bRun)
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 1.0)
		{
			/*解决sendCount计数问题*/
			printf("thread<%d>, clients<%d>, time<%lf>, send<%d>\n", tCount, cCount, t, sendCount);
			sendCount = 0;
			tTime.update();
		}
		Sleep(1);
	}
	printf("已退出.\n");
	return 0;
}