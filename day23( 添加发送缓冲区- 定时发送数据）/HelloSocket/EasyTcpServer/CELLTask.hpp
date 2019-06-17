#ifndef _CELL_TASK_H_
#include <thread>
#include <mutex>
//链表来进行存储数据 能快速进行增加删除等操作
#include <list>
//任务类型-基类
//执行任务的服务类型
/*基本任务类 管理任务类*/
class CellTask
{
public:
	CellTask()
	{

	};
	//虚析构函数
	virtual ~CellTask() 
	{
		
	};
	//执行任务
	virtual void doTask()
	{

	}
private:

};

//执行任务的服务类型
class CellTaskServer
{
private:
	//任务数据
	std::list<CellTask*> _tasks;
	//任务数据缓冲区
	std::list<CellTask*> _tasksBuff;
	//改变数据缓冲区时需要加锁
	std::mutex _mutex;
	//线程
	//std::thread _thread;
public:
	//添加任务
	void addTask(CellTask* task)
	{
		//在整个作用域中都可以进行加锁
		std::lock_guard<std::mutex> lock(_mutex);
		_tasksBuff.push_back(task);
	}
	//启动工作线程
	void Start()
	{
		//线程
		std::thread t(std::mem_fn(&CellTaskServer::OnRun), this);
		t.detach();
	}
protected:
	//工作函数
	void OnRun()
	{
		while (true)
		{
			//从缓冲区取出数据
			if (!_tasksBuff.empty())
			{
				std::lock_guard<std::mutex> lock(_mutex);
				for (auto pTask : _tasksBuff)
				{
					_tasks.push_back(pTask);
				}
				_tasksBuff.clear();/*清空缓冲*/
			}
			//如果没有任务
			if (_tasks.empty())
			{
				//没有任务的时候等待一毫秒 避免cpu占用过多资源
				std::chrono::milliseconds t(1);
				std::this_thread::sleep_for(t);
				continue;
			}
			//处理任务
			for (auto pTask : _tasks)
			{
				pTask->doTask();
				delete pTask;
			}
			//清空任务
			_tasks.clear();
		}
		
	}
private:
};
#endif