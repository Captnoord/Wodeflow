#ifndef __LOCKMUTEX_HPP
#define __LOCKMUTEX_HPP

class LockMutex
{
	const mutex_t &m_mutex;
public:
	LockMutex(mutex_t &m) : m_mutex(m) { LWP_MutexLock(m_mutex); }
	LockMutex(const mutex_t &m) : m_mutex(m) { LWP_MutexLock(m_mutex); }
	~LockMutex(void) { LWP_MutexUnlock(m_mutex); }
protected:
    // copy ops are private to prevent copying 
    //LockMutex(const NoCopy&); // no implementation 
    LockMutex& operator=(const LockMutex&); // no implementation
	
};

#endif // !defined(__LOCKMUTEX_HPP)
