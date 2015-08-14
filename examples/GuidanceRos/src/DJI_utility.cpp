#include "DJI_utility.h"

#ifdef WIN32

DJI_lock::DJI_lock()
{
	InitializeCriticalSection( &m_critical_section );
}

DJI_lock::~DJI_lock()
{
}

void DJI_lock::enter()
{
	EnterCriticalSection( &m_critical_section );
}

void DJI_lock::leave()
{
	LeaveCriticalSection( &m_critical_section );
}

void   sleep( int microsecond )
{
	Sleep( ( microsecond + 999 ) / 1000 );
}
DJI_event::DJI_event()
{
	SECURITY_ATTRIBUTES   sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	CreatePipe( &m_pipe_read, &m_pipe_write, &sa, 0 );
}

DJI_event::~DJI_event()
{
	CloseHandle( m_pipe_read );
	CloseHandle( m_pipe_write );
}

int DJI_event::set_event()
{
	char buffer[1] = {0};
	int count = 0;
	int ret = WriteFile( m_pipe_write, (LPWORD)buffer, 1, (LPDWORD)&count, NULL );
	return ret;
}

int DJI_event::wait_event()
{
	char buffer[1] = {0};
	int count = 0;
	int ret = ReadFile( m_pipe_read, (LPWORD)buffer, 1, (LPDWORD)&count, NULL );
	return ret;
}

#else

DJI_lock::DJI_lock()
{
	pthread_mutex_init( &m_lock, NULL );
}

DJI_lock::~DJI_lock()
{
}

void DJI_lock::enter()
{
	pthread_mutex_lock( &m_lock );
}

void DJI_lock::leave()
{
	pthread_mutex_unlock( &m_lock );
}

DJI_event::DJI_event()
{
	sem_init( &m_sem, 0, 0 );
}

DJI_event::~DJI_event()
{
}

int DJI_event::set_event()
{
	int ret = sem_post( &m_sem );
	return ret;
}

int DJI_event::wait_event()
{
	int ret = sem_wait( &m_sem );
	return ret;
}

#endif
