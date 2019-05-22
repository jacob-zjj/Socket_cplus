#include <iostream>
#include <thread>
//线程中的锁
#include <mutex>
//原子操作
#include <atomic>
#include "CELLTimestamp.hpp"
using namespace std;
//原子操作(不可分割的操作) 原子 分子
//创建全局锁
mutex m;
//支持标准C++的所有平台
/*线程的入口函数*/
const int tCount = 4;
//atomic<int> sum;
atomic_int sum = 0;//相当于直接给变量加上锁 比锁所消耗的资源小
void workFun(int index)
{
	//抢占式
	for (int n = 0; n < 20000000; n ++)
	{
		//自解锁
		//lock_guard<mutex> lg(m);//构造函数时lock 析构函数时unlock
		//m.lock();//被锁锁住的区域叫做临界区域
		//临界区域的开始
		sum++;
		//临界区域的结束
		//m.unlock();
	}//线程安全 线程不安全
	//cout << index << "Hello, other thread." << n << endl;
}

int main()
{
	thread t[tCount];
	for (int n = 0; n < tCount; n ++)
	{
		t[n] = thread(workFun, n);
	}
	CELLTimestamp tTime;
	for (int n = 0; n < tCount; n++)
	{
		//t[n].detach();
		t[n].join();//在线程执行完之后才会执行主线程
	}
	//t.detach();//线程和主线程相互 分离 各不相干
	//t.join();使用join的作用是 表示线程执行完成后再执行主线程
	cout << tTime.getElapsedTimeInMillSec() <<",sum " <<  sum << endl;
	sum = 0;
	tTime.update();
	for (int n = 0; n < 80000000; n++)
	{
		//m.lock();//被锁锁住的区域叫做临界区域
				 //临界区域的开始
		sum++;
		//临界区域的结束
		//m.unlock();
	}
	cout << tTime.getElapsedTimeInMillSec() << ",sum " << sum << endl;
	cout << "hello, main thread..." << endl;
	getchar();
	return 0;
}