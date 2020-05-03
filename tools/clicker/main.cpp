#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

static bool click = false;

int main(int argc, char **args)
{
	add_global_key_listener(Key_P, false, true, false, [](Capture&, KeyStateFlags action) {
		if (action & KeyStateDown)
		{
			click = !click;
			printf("%d\n", (int)click);
		}
	}, Capture());

	do_simple_dispatch_loop([](Capture&) {
		sleep(100);
		if (click)
		{
			send_global_mouse_event(KeyStateDown, Mouse_Left);
			send_global_mouse_event(KeyStateUp, Mouse_Left);
		}
	}, Capture());

	return 0;
}
