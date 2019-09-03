#ifndef _Lock_H  
#define _Lock_H  

#include <windows.h>  


#ifdef LANDIS40CORE_EXPORTS
#define DLLCLASS   __declspec( dllexport )
#else
#define DLLCLASS   __declspec( dllimport )
#endif


//lock and unlock
class DLLCLASS Mutex
{
public:
	Mutex();
	~Mutex();

	void Lock() const;
	void Unlock() const;

private:
	HANDLE m_mutex;
};


#endif  

