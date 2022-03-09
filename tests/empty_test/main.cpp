namespace tkTest
{
	struct ABC
	{
		int a;
		char c;
	};
}

int entry()
{
	return 0;
}

#include <Windows.h>

void* __crt_ev = nullptr; 
extern "C" void mainCRTStartup(); 
extern "C" __declspec(dllexport) void __stdcall __init_crt(void* ev) 
{
	new tkTest::ABC;
	__crt_ev = ev; 
	mainCRTStartup(); 
} 

int main(int argc, char** args) 
{ 
	if (__crt_ev) 
	{ 
		SetEvent(__crt_ev); 
		while (true) 
		Sleep(60000); 
	}
	return entry(); 
}

