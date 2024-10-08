#pragma once

#include "Common.h"
#include "SharedQueue.h"

namespace cz
{

class AsyncCommandQueue
{
public:
	AsyncCommandQueue() {}
	virtual ~AsyncCommandQueue() {}
	AsyncCommandQueue(const AsyncCommandQueue&) = delete;
	AsyncCommandQueue& operator=(const AsyncCommandQueue&) = delete; 

	// To be called by any thread that wishes to send commands to the queue
	void send( std::function<void()>&& f);

	WorkQueue& getQueue()
	{
		return m_queue;
	}

protected:
	/*!
	 * \param wait If there are no commands available, it will block and wait
	 */
	void tickImpl(bool wait);

	WorkQueue m_queue;
};


/*
Command queue that needs explicit ticking.
This is useful when the queue is to be part of some system that already has a thread where the commands should be
executed
*/
class AsyncCommandQueueExplicit : public AsyncCommandQueue
{
public:
	void tick(bool wait);
};

/*
Command queue that creates a thread to execute the commands, thus being completely independent
*/
class AsyncCommandQueueAutomatic : public AsyncCommandQueue
{
public:
	AsyncCommandQueueAutomatic();
	virtual ~AsyncCommandQueueAutomatic();
private:
	void run();
	std::thread m_thread;
	bool m_finish= false;
};

} // namespace cz

