/*
 * DQueue.h
 *
 *  Created on: 2018Äê4ÔÂ15ÈÕ
 *      Author: caoyuan9642
 */

#ifndef IOQUEUE_H_
#define IOQUEUE_H_

#include "mbed.h"

#define SIGNAL_QUEUE 0x00000010
#define MAX_THREAD_QUEUE 16

struct ThreadQueue
{
	osThreadId_t threads[MAX_THREAD_QUEUE];
	osThreadId_t *head, *tail;

	ThreadQueue()
	{
		head = tail = threads;
	}

	bool empty()
	{
		return head == tail;
	}

	bool full()
	{
		return (tail - head + 1) % MAX_THREAD_QUEUE == 0;
	}

	osThreadId_t get()
	{
		if (empty())
			return NULL;
		osThreadId_t th = *head;
		if (++head == threads + MAX_THREAD_QUEUE)
			head = threads;
		return th;
	}

	int put(osThreadId_t th)
	{
		if (full())
		{
			return -1;
		}
		*tail = th;
		if (++tail == threads + MAX_THREAD_QUEUE)
			tail = threads;
		return 0;
	}

	/*
	 * Wait on the current thread until awaken by other operations in the queue
	 */
	osStatus qwait(uint32_t wait)
	{
		if (wait == 0)
		{
			return osErrorTimeout;
		}
		core_util_critical_section_exit();
		if (put(Thread::gettid()) != 0)
		{
			// Queue full
			return osErrorTimeout;
		}
		Thread::signal_clr(0x7FFFFFFF);
		Thread::signal_wait(SIGNAL_QUEUE, wait);
		core_util_critical_section_enter();
		return osOK;
	}
};

template<typename T, unsigned int N>
class OutputQueue: private mbed::NonCopyable<OutputQueue<T, N> >
{
public:

	typedef void (*notify_cb)(OutputQueue<T, N> *);

	/** Create and initialize a message Queue.
	 *
	 * @note You cannot call this function from ISR context.
	 */
	OutputQueue()
	{
		memset(buf, 0, sizeof(buf));
		ntf = NULL;
		head = buf;
		tail = buf;
	}
	/** Queue destructor
	 *
	 * @note You cannot call this function from ISR context.
	 */
	virtual ~OutputQueue()
	{
	}

	/** Check if the queue is empty
	 *
	 * @return True if the queue is empty, false if not
	 *
	 * @note You may call this function from ISR context.
	 */
	bool empty() const
	{
		return head == tail;
	}

	/** Check if the queue is full
	 *
	 * @return True if the queue is full, false if not
	 *
	 * @note You may call this function from ISR context.
	 */
	bool full() const
	{
		return (tail - head == -1) || (tail - head == N - 1);
	}

	/** Check if the queue is full
	 *
	 * @return number of empty space
	 *
	 * @note You may call this function from ISR context.
	 */
	int capacity() const
	{
		return N - (tail - head + N) % N;
	}
	/** Check if the queue is full
	 *
	 * @return number of empty space
	 *
	 * @note You may call this function from ISR context.
	 */
	int count() const
	{
		return (tail - head + N) % N;
	}

	/** Put a message in a Queue.
	 @param   data      message pointer.
	 @param   millisec  timeout value or 0 in case of no time-out. (default: osWaitForever)
	 @return  status code that indicates the execution status of the function:
	 @a osOK the message has been put into the queue.
	 @a osErrorTimeout the message could not be put into the queue in the given time.

	 @note You may call this function from ISR context if the millisec parameter is set to 0.
	 */
	osStatus put(const T &data, uint32_t wait = osWaitForever)
	{
		core_util_critical_section_enter();

		// Wait for signal
		while (full())
		{
			if (thq.qwait(wait) == osErrorTimeout)
			{
				core_util_critical_section_exit();
				return osErrorTimeout;
			}
		}

		*tail = data;
		if (++tail == buf + N)
			tail = buf;

		if (ntf)
		{
			ntf(this);
		}
		core_util_critical_section_exit();

		return osOK;
	}

	/** Get a message or Wait for a message from a Queue. Messages are retrieved in a descending priority order or
	 first in first out when the priorities are the same.
	 @param   pdata     pointer for return value
	 @param   millisec  timeout value or 0 in case of no time-out. (default: osWaitForever).
	 @return  status code that indicates the execution status of the function:
	 @a osOK data retrieved in pdata
	 @a osEventTimeout no message has arrived during the given timeout period.

	 @note You may call this function from ISR context if the millisec parameter is set to 0.
	 */
	osStatus get(T *pdata)
	{
		if (empty())
			return osErrorResource;

		core_util_critical_section_enter();

		*pdata = *head;
		if (++head == buf + N)
			head = buf;
		if (!thq.empty())
		{
			osThreadFlagsSet(thq.get(), SIGNAL_QUEUE);
		}

		core_util_critical_section_exit();

		return osOK;
	}

	void notify(notify_cb cb)
	{
		ntf = cb;
	}

protected:
	notify_cb ntf;
	T buf[N];
	T* head;
	T* tail;
	ThreadQueue thq;

};

template<typename T, unsigned int N>
class InputQueue: private mbed::NonCopyable<InputQueue<T, N> >
{
public:

	typedef void (*notify_cb)(InputQueue<T, N> *);

	/** Create and initialize a message Queue.
	 *
	 * @note You cannot call this function from ISR context.
	 */
	InputQueue()
	{
		memset(buf, 0, sizeof(buf));
		ntf = NULL;
		head = buf;
		tail = buf;
	}
	/** Queue destructor
	 *
	 * @note You cannot call this function from ISR context.
	 */
	virtual ~InputQueue()
	{
	}

	/** Check if the queue is empty
	 *
	 * @return True if the queue is empty, false if not
	 *
	 * @note You may call this function from ISR context.
	 */
	bool empty() const
	{
		return head == tail;
	}

	/** Check if the queue is full
	 *
	 * @return True if the queue is full, false if not
	 *
	 * @note You may call this function from ISR context.
	 */
	bool full() const
	{
		return (tail - head == -1) || (tail - head == N - 1);
	}

	/** Check if the queue is full
	 *
	 * @return number of empty space
	 *
	 * @note You may call this function from ISR context.
	 */
	int capacity() const
	{
		return N - (tail - head + N) % N;
	}
	/** Check if the queue is full
	 *
	 * @return number of empty space
	 *
	 * @note You may call this function from ISR context.
	 */
	int count() const
	{
		return (tail - head + N) % N;
	}

	/** Put a message in a Queue.
	 @param   data      message pointer.
	 @param   millisec  timeout value or 0 in case of no time-out. (default: osWaitForever)
	 @return  status code that indicates the execution status of the function:
	 @a osOK the message has been put into the queue.
	 @a osErrorTimeout the message could not be put into the queue in the given time.

	 @note You may call this function from ISR context if the millisec parameter is set to 0.
	 */
	osStatus put(const T &data)
	{
		core_util_critical_section_enter();

		*tail = data;
		if (++tail == buf + N)
			tail = buf;

		if (!thq.empty())
		{
			osThreadFlagsSet(thq.get(), SIGNAL_QUEUE);
		}

		core_util_critical_section_exit();

		return osOK;
	}

	/** Get a message or Wait for a message from a Queue. Messages are retrieved in a descending priority order or
	 first in first out when the priorities are the same.
	 @param   pdata     pointer for return value
	 @param   millisec  timeout value or 0 in case of no time-out. (default: osWaitForever).
	 @return  status code that indicates the execution status of the function:
	 @a osOK data retrieved in pdata
	 @a osEventTimeout no message has arrived during the given timeout period.

	 @note You may call this function from ISR context if the millisec parameter is set to 0.
	 */
	osStatus get(T *pdata, uint32_t wait = osWaitForever)
	{

		core_util_critical_section_enter();

		// Wait for non-empty
		while (empty())
		{
			if (thq.qwait(wait) == osErrorTimeout)
			{
				core_util_critical_section_exit();
				return osErrorTimeout;
			}
		}

		*pdata = *head;
		if (++head == buf + N)
			head = buf;

		if (ntf)
		{
			ntf(this);
		}

		core_util_critical_section_exit();

		return osOK;
	}

	void notify(notify_cb cb)
	{
		ntf = cb;
	}

protected:
	notify_cb ntf;
	T buf[N];
	T* head;
	T* tail;
	ThreadQueue thq;

};
/** @}*/
/** @}*/

#endif
