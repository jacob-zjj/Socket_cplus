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

//一个客户端同时链接多个服务器 以另外一种方式替换多线程方式
//客户端自己向服务器发数据
int main()
{
	/*创建连接对象*/
	EasyTcpClient client;
	//就本机来说该ip地址应该填 192.168.242.1
	client.Connect("192.168.242.1", 4567);
	EasyTcpClient client2;
	//就本机来说该ip地址应该填 192.168.242.1
	client2.Connect("127.0.0.1", 4568);
	EasyTcpClient client3;
	//就本机来说该ip地址应该填 192.168.242.1
	client3.Connect("192.168.242.128", 4567);


	///*创建启动线程 通过线程来向服务器发送数据*/
	////启动UI线程
	//thread t1(cmdThread, &client);
	//t1.detach();//将其与主线程进行分离
	////启动UI线程
	//thread t2(cmdThread, &client2);
	//t2.detach();//将其与主线程进行分离
	////启动UI线程
	//thread t3(cmdThread, &client3);
	//t3.detach();//将其与主线程进行分离

	Login login;
	strcpy(login.userName, "zjj");
	strcpy(login.PassWord, "969513");
	while (client.isRun() || client2.isRun() || client3.isRun())
	{
		client.OnRun();
		client2.OnRun();
		client3.OnRun();
		//链接时客户端自己发送消息
		client.SendData(&login);
		client2.SendData(&login);
		client3.SendData(&login);
	}
	client.Close();
	client2.Close();
	client3.Close();
	printf("已退出.\n");
	getchar();
	return 0;
}