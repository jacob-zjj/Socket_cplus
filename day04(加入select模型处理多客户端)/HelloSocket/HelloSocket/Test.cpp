#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <WinSock2.h>
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
	/*
	用Socket API建立简易TCP客户端
	1、建立一个socket
	2、链接服务器 connect
	3、接收服务器信息 recv
	4、关闭套节字closesocket
	//--用Socket API建立简易TCP服务端
	1、建立一个socket
	2、bind 绑定用于接收客户端链接的网络端口
	3、listen 监听网络端口
	4、accept 等待接收客户端链接
	5、send 向客户端发送一条数据
	6、关闭套接字closesocket
	*/
	//---------------------------
	WSACleanup();
	cout << "hello..." << endl;
	return 0;
}