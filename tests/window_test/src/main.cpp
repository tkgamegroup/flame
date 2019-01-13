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

#include <flame/foundation/window.h>

using namespace flame;

FLAME_PACKAGE_BEGIN(WindowClickC)
	FLAME_PACKAGE_ITEM(WindowPtr, w, p)
FLAME_PACKAGE_END

int main(int argc, char **args)
{
	auto app = Application::create();
	auto w = Window::create(app, "Hello",  Ivec2(1280, 720), WindowFrame);

	w->add_mouse_listener(Function<Window::MouseListenerParm>([](Window::MouseListenerParm &p) {
		auto c = p.get_capture<WindowClickC>();
		if (p.is_down())
			c.w()->close();

	}, { w }));

	app->run(Function<>([](Package &p){
	}));

	return 0;
}
