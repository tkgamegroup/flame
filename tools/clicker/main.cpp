#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

#include <Windows.h>

using namespace flame;

static bool click = false;

int main(int argc, char **args)
{
	add_global_key_listener(Key_P, false, true, false, [](void*, KeyStateFlags action) {
		if (action & KeyStateDown)
		{
			click = !click;
			printf("%d\n", (int)click);
		}
	}, Capture());

	for (;;)
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(100);
		if (click)
		{
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, NULL);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, NULL);
		}
	}

	return 0;
}
