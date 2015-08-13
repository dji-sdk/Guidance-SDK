#include "DJI_utility.h"

#ifdef WIN32

lock::lock()
{
	InitializeCriticalSection( &m_critical_section );
}

lock::~lock()
{
}

void lock::enter()
{
	EnterCriticalSection( &m_critical_section );
}

void lock::leave()
{
	LeaveCriticalSection( &m_critical_section );
}

void   sleep( int microsecond )
{
	Sleep( ( microsecond + 999 ) / 1000 );
}


event::event()
{
	SECURITY_ATTRIBUTES   sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	CreatePipe( &m_pipe_read, &m_pipe_write, &sa, 0 );
}

event::~event()
{
	CloseHandle( m_pipe_read );
	CloseHandle( m_pipe_write );
}

int event::set_event()
{
	char buffer[1] = {0};
	int count = 0;
	int ret = WriteFile( m_pipe_write, (LPWORD)buffer, 1, (LPDWORD)&count, NULL );
	return ret;
}

int event::wait_event()
{
	char buffer[1] = {0};
	int count = 0;
	int ret = ReadFile( m_pipe_read, (LPWORD)buffer, 1, (LPDWORD)&count, NULL );
	return ret;
}

#else

lock::lock()
{
	pthread_mutex_init( &m_lock, NULL );
}

lock::~lock()
{
}

void lock::enter()
{
	pthread_mutex_lock( &m_lock );
}

void lock::leave()
{
	pthread_mutex_unlock( &m_lock );
}

event::event()
{
	sem_init( &m_sem, 0, 0 );
}

event::~event()
{
}

int event::set_event()
{
	int ret = sem_post( &m_sem );
	return ret;
}

int event::wait_event()
{
	int ret = sem_wait( &m_sem );
	return ret;
}

#endif
