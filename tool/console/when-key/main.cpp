#include <flame/foundation/foundation.h>

using namespace flame;

int main(int argc, char **args)
{
	if (argc != 3)
	{
		printf(
			   "usage:\n"
			   "  key \"command\"\n"
			   "note:\n"
			   "  key can be combine with modifier i.e. Shift+Ctrl+C\n"
			   "  modifiers are: Shift, Ctrl, Alt\n"
			   "  both keys and modifiers aren't case sensitive\n"
			   "  command must be wraped in \"\"\n"
		);
		system("pause");
		return 0;
	}

	printf("key=\"%s\" cmd=\"%s\"\n", args[1], args[2]);

	auto keys = string_split(std::string(args[1]), '+');
	auto key = Key_Null;
	auto shift = false;
	auto ctrl = false;
	auto alt = false;
	for (auto &t : keys)
	{
		string_to_lower$(t);
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
		const char* key;
		const char* command;
	};

	Capture capture;
	capture.key = args[1];
	capture.command = args[2];

	add_global_key_listener(key, shift, ctrl, alt, Function<void(void* c, KeyState action)>(
	[](void* _c, KeyState action){
		if (action == KeyStateDown)
		{
			auto c = (Capture*)_c;

			printf("key down: %s\n", c->key);
			printf("run: %s\n", c->command);

			system(c->command);

			printf("key=[ %s ] cmd=[ %s ]\n", c->key, c->command);
		}
	}, sizeof(Capture), &capture));

	do_simple_dispatch_loop();

	return 0;
}
