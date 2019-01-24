// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/foundation/foundation.h>

using namespace flame;

FLAME_PACKAGE_BEGIN(GlobalKeyC)
	FLAME_PACKAGE_ITEM(charptr, key, p)
	FLAME_PACKAGE_ITEM(charptr, command, p)
FLAME_PACKAGE_END

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

	printf("run [ %s ] when [ %s ] pressed\n", args[2], args[1]);

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
	add_global_key_listener(key, shift, ctrl, alt, Function<GlobalKeyParm>([](GlobalKeyParm &p){
		if (p.action() == KeyStateDown)
		{
			auto c = p.get_capture<GlobalKeyC>();

			printf("key down: %s\n", c.key());
			printf("run: %s\n", c.command());

			system(c.command());

			printf("run [ %s ] when [ %s ] pressed\n", c.command(), c.key());
		}
	}, { args[1], args[2] }));

	do_simple_dispatch_loop();

	return 0;
}
