// MIT License
// 
// Copyright (c) 2019 wjs
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

#include <flame/foundation/window.h>

using namespace flame;

int main(int argc, char** args)
{
	auto app = Application::create();
	auto w = Window::create(app, "Window Test", Ivec2(1280, 720), WindowFrame);

	w->add_mouse_listener(Function<void(void*, KeyState, MouseKey, const Ivec2&)>(
		[](void* c, KeyState action, MouseKey key, const Ivec2 & pos) {
			if (is_mouse_down(action, key))
				(*((WindowPtr*)c))->close();
		}, sizeof(void*), &w));

	app->run(Function<void(void* c)>(
		[](void* c) {
		}));

	return 0;
}
