#include "EasyTcpClient.hpp"
#include <thread>
#include "iostream"
using namespace std;

//将输入命令分离出来
void cmdThread(EasyTcpClient* client) {
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			client->Close();
			printf("退出cmdThread线程\n");
			break;
		}
		else if (0 == strcmp(cmdBuf, "login"))
		{
			Login login;
			strcpy(login.userName, "zjj");
			strcpy(login.PassWord, "969513");
			client->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout"))
		{
			LoginOut logout;
			strcpy(logout.userName, "zjj");
			client->SendData(&logout);
		}
		else
		{
			printf("不支持的命令\n");
		}
	}
}
int main()
{
	EasyTcpClient client;
	client.Connect("127.0.0.1", 4567);

	EasyTcpClient client2;
	client2.Connect("127.0.0.1", 4568);

	//启动UI线程
	thread t1(cmdThread, &client);
	t1.detach();//将其与主线程进行分离

	thread t2(cmdThread, &client2);
	t2.detach();//将其与主线程进行分离

	while (client.isRun() || client2.isRun())
	{
		client.OnRun();
		client2.OnRun();
	}
	client.Close();
	client2.Close();
	printf("已退出.\n");
	getchar();
	return 0;
}