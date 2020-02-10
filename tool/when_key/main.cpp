#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char **args)
{
	if (argc != 3)
	{
		printf("usage:\n  key \"command\"\nnote:\n  key can be combine with modifier i.e. Shift+Ctrl+C\n  modifiers are: Shift, Ctrl, Alt\n  both keys and modifiers aren't case sensitive\n  command must be wraped in \"\"\n");
		system("pause");
		return 0;
	}

	printf("key=\"%s\" cmd=\"%s\"\n", args[1], args[2]);

	auto keys = SUS::split(args[1], '+');
	auto key = KeyNull;
	auto shift = false;
	auto ctrl = false;
	auto alt = false;
	for (auto &t : keys)
	{
		std::transform(t.begin(), t.end(), t.begin(), ::tolower);
		if (t == "shift")
			shift = true;
		else if (t == "ctrl")
			ctrl = true;
		else if (t == "alt")
			alt = true;
		else if (t.size() == 1)
		{
			auto ch = t[0];
			if (ch >= 'a' && ch <= 'z')
				key = Key(Key_A + ch - 'a');
		}
	}

	struct Capture
	{
		std::string c;
	}capture;
	capture.c = args[2];
	add_global_key_listener(key, shift, ctrl, alt, [](void* c, KeyState action){
		auto& capture = *(Capture*)c;

		if (action == KeyStateDown)
		{
			printf("pressed\n");

			system(capture.c.c_str());
		}
	}, new_mail(&capture));

	do_simple_dispatch_loop();

	return 0;
}
