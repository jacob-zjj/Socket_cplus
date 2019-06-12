//支持c++ 11 且跨平台
/*高精度时间计时器*/
#pragma once
#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

#include <chrono>
using namespace std::chrono;

class CELLTimestamp
{
public:
	CELLTimestamp()
	{
		update();
	}
	~CELLTimestamp()
	{

	}
	void update()
	{
		_begin = high_resolution_clock::now();
	}
	/*获取当前秒*/
	double getElapsedSecond()
	{
		return this->getElapsedTimeInMicrosec() * 0.000001;
	}
	/*获取当前毫秒*/
	double getElapsedTimeInMillSec()
	{
		return this->getElapsedTimeInMicrosec() * 0.001;
	}
	/*获取当前微秒*/
	long long getElapsedTimeInMicrosec()
	{
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}
protected:
	//high_resolution_clock 高精度计时器
	time_point<high_resolution_clock> _begin;
};
#endif