#include "lwip/opt.h"
#include <Windows.h>

//u32_t sys_now(void)
//{
//	return (u32_t)GetTickCount();
//}

LARGE_INTEGER freq, sys_start_time;

void sys_init_timing()
{
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&sys_start_time);
}

static LONGLONG sys_get_ms_longlong()
{
	LONGLONG ret;
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	ret = now.QuadPart-sys_start_time.QuadPart;
	return (u32_t)(((ret)*1000)/freq.QuadPart);
}

u32_t sys_jiffies()
{
	return (u32_t)sys_get_ms_longlong();
}

u32_t sys_now()
{
	return (u32_t)sys_get_ms_longlong();
}

CRITICAL_SECTION critSec;

void InitSysArchProtect()
{
	InitializeCriticalSection(&critSec);
}
u32_t sys_arch_protect()
{
	EnterCriticalSection(&critSec);
	return 0;
}
void sys_arch_unprotect(u32_t pval)
{
	LWIP_UNUSED_ARG(pval);
	LeaveCriticalSection(&critSec);
}

void msvc_sys_init()
{
	sys_init_timing();
	InitSysArchProtect();
}

void sys_init()
{
	msvc_sys_init();
}
