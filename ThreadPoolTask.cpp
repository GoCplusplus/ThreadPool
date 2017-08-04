#include "ThreadPoolTask.h"
#include <iostream>
#include <Windows.h>

CThreadPoolTask::CThreadPoolTask(int nThreadsNum)
{
	m_nThreadsNum = nThreadsNum;
	m_nReady = 0;
	for (int i = 0; i < m_nThreadsNum; i++)
	{
		m_vectThreads.emplace_back(std::thread([this]() {
			while (true)
			{
				std::unique_lock<std::mutex> lock(m_condMutex);
				if (m_nReady < m_nThreadsNum) // Ӧ�������ô��ļ��
					m_nReady.fetch_add(1);
				m_cond.wait(lock);
				if (m_bExit) // �˳�
					return true;
				while (true)
				{
					m_queueLock.lock();
					if (!m_queueTask.empty())
					{
						Task task = std::move(m_queueTask.front());
						m_queueTask.pop();
						m_queueLock.unlock();
						task.func(task.arg);
					}
					else
					{
						m_queueLock.unlock();
						break;
					}
				}
			}
		}));
	}
}

CThreadPoolTask::~CThreadPoolTask()
{
	Quit();
}

void CThreadPoolTask::AddTask(struct Task pTask)
{
	m_queueLock.lock();
	m_queueTask.push(pTask);
	m_queueLock.unlock();
	while (m_nReady < m_nThreadsNum) // �����̶߳���׼������
		;
	m_cond.notify_one();
}

void CThreadPoolTask::AddTask(struct Task&& pTask)
{
	m_queueLock.lock();
	m_queueTask.push(std::move(pTask));
	m_queueLock.unlock();
	while (m_nReady < m_nThreadsNum) // �����̶߳���׼������
		;
	m_cond.notify_one();
}

void CThreadPoolTask::Quit()
{

	// �ȴ���������ִ����� ������ִ��������߳��޷��յ�notify_all����Ϣ�������޷��˳�
	while (true)
	{
		m_queueLock.lock();
		if (m_queueTask.empty())
		{
			m_queueLock.unlock();
			break;
		}
		else
		{
			m_queueLock.unlock();
			//Sleep(100); // 100ms���ٴμ��
			m_cond.notify_all();
			Sleep(100); // 100ms���ٴμ��
		}
	}

	m_bExit = true;
	m_cond.notify_all();
	for (auto & thread : m_vectThreads)
		thread.join();
	m_vectThreads.clear();
	std::queue<Task> queueTask;
	m_queueTask.swap(queueTask);
}
