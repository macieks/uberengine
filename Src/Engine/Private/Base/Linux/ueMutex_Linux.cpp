#include "Base/Base.h"
#include "Threading/Mutex.h"

ueMutex::ueMutex()
{
	pthread_mutex_init(&m_handle, NULL);
}

ueMutex::~ueMutex()
{
	pthread_mutex_destroy(&m_handle);
}

void ueMutex::Lock()
{
	pthread_mutex_lock(&m_handle);
}

void ueMutex::Unlock()
{
	pthread_mutex_unlock(&m_handle);
}
