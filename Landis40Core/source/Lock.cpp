#include "Lock.h"  

Mutex::Mutex()
{
	m_mutex = ::CreateMutex(NULL, FALSE, NULL);
}

Mutex::~Mutex()
{
	::CloseHandle(m_mutex);
}

void Mutex::Lock() const
{
	WaitForSingleObject(m_mutex, INFINITE);
}

void Mutex::Unlock() const
{
	::ReleaseMutex(m_mutex);
}
