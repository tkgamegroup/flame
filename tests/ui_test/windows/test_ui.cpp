//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include <flame/UI/instance.h>

namespace flame
{
	namespace UI
	{
		void show(void *i)
		{
			auto ui = (Instance*)i;
			ui->begin_window("Hey", Vec2(get_inf()), Vec2(get_inf()));
			static ValueInterpolater show = {0.f, 0.f, 2.f, 0.f, 2.f};
			show.step(ui->elapsed_time);
			if (show.v > 1.f)
				ui->text("hello");
			ui->end_window();
		}
	}
}

extern "C" {
	_declspec(dllexport) void show(void *i)
	{
		flame::UI::show(i); 
	}
}
