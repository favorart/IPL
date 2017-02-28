
#ifdef _WIN32
#define WINDOWS
#elif  _WIN64
#define WINDOWS
#endif

#ifdef WINDOWS
#include <Windows.h>
#endif

#include <stdlib.h>
void 	SetTitle (char* title)
{
#ifdef WINDOWS
		wchar_t t[257]; *t=0;
		MultiByteToWideChar (1251,0,title,-1,t,2*(strlen(title)+1));
		SetConsoleTitleW (t);
#endif
}
