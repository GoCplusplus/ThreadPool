#ifndef __CTHREADPOOLTASK__H
#define __CTHREADPOOLTASK__H

#include <functional>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <vector>

// �̳߳�ģ��

struct Task // ����
{
	std::function<void(void*)> func;
	void* arg; // ����
};

class CThreadPoolTask
{
public:
	CThreadPoolTask(int nThreadsNum);
	~CThreadPoolTask();

public:
	void AddTask(struct Task Tasklhs);
	void AddTask(struct Task&& TaskRhs);
	void Quit();
private:
	std::queue<struct Task> m_queueTask;
	std::mutex              m_queueLock;

	int                      m_nThreadsNum;
	std::vector<std::thread> m_vectThreads;
	std::atomic<bool>        m_bExit;
	std::atomic<int>         m_nReady; // ׼�����˵��߳�

	std::condition_variable  m_cond; // ��������
	std::mutex               m_condMutex;
};

static CThreadPoolTask g_ThreadPool(8);

#endif