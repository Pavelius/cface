#include "thread.h"
#include "win.h"

thread::thread(proc p, void* param)
{
	handle = CreateThread(0, 0, p, param, 0, (unsigned*)&id);
}

void thread::join()
{
	if(joinable())
		WaitForSingleObject(handle, INFINITE);
}

bool thread::joinable() const
{
	if(!handle)
		return false;
	unsigned code = 0;
	GetExitCodeThread(handle, &code);
	return code==STILL_ACTIVE;
}

void thread::run(proc p, void* param)
{
	thread e(p,param);
}

thread::mutex::mutex()
{
	InitializeCriticalSectionAndSpinCount(&data, 0x0400);
}

thread::mutex::~mutex()
{
	DeleteCriticalSection(&data);
}

void thread::mutex::lock()
{
	EnterCriticalSection(&data);
}

bool thread::mutex::try_lock()
{
	return TryEnterCriticalSection(&data)!=0;
}

void thread::mutex::unlock()
{
	LeaveCriticalSection(&data);
}

thread::sync::sync(mutex& e):data(e)
{
	data.lock();
}

thread::sync::~sync()
{
	data.unlock();
}