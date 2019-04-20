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
#include <flame/universe/entity.h>
#include <flame/universe/components/widget.h>
#include <flame/universe/components/ui.h>

namespace flame
{
	struct cUIPrivate : cUI$
	{
		graphics::Canvas* canvas;
		Window* window;

		Ivec2 mouse_pos, mouse_pos_prev, mouse_disp;
		int mouse_scroll;
		int mouse_buttons[3];

		bool f_all_done;
		bool f_hovering, f_mljustdown, f_mljustup, f_mrjustdown, f_mrjustup;
		Vec2 f_mpos;
		Ivec2 f_mdisp;
		int f_mscroll;

		cUIPrivate(void* data) :
			canvas(nullptr)
		{
			hovering = nullptr;
			focusing = nullptr;
		}

		void update(float delta_time)
		{
			mouse_disp = mouse_pos - mouse_pos_prev;

			f_mljustdown = is_mouse_down((KeyState)mouse_buttons[Mouse_Left], Mouse_Left, true);
			f_mljustup = is_mouse_up((KeyState)mouse_buttons[Mouse_Left], Mouse_Left, true);
			f_mrjustdown = is_mouse_down((KeyState)mouse_buttons[Mouse_Right], Mouse_Right, true);
			f_mrjustup = is_mouse_up((KeyState)mouse_buttons[Mouse_Right], Mouse_Right, true);
			f_mpos = mouse_pos;
			f_mdisp = mouse_disp;
			f_mscroll = mouse_scroll;

			if (focusing)
			{
				if (!focusing->entity->global_visible)
				{
					focusing->focusing = false;
					focusing->dragging = false;
					focusing = nullptr;
				}
				else if (!is_mouse_down((KeyState)mouse_buttons[Mouse_Left], Mouse_Left))
					focusing->dragging = false;
				else if (focusing->dragging && f_mdisp != 0)
				{
					f_mdisp = Ivec2(0);
				}
			}

			f_all_done = !f_mljustdown && !f_mljustup && !f_mrjustdown && !f_mrjustup && f_mdisp == 0 && f_mscroll == 0;

			traverse_backward(entity, Function<void(void*, Entity*)>(
				[](void* c, Entity* e) {
					auto thiz = *((cUIPrivate**)c);
					if (thiz->f_all_done)
						return;

					auto ev = (cWidget$*)e->component(cH("Event"));
					if (ev)
					{
						auto mhover = ev->contains(thiz->f_mpos);
						if (ev->blackhole || mhover)
						{
							if (thiz->f_mljustdown)
							{
								thiz->f_mljustdown = false;
								thiz->focusing = ev;
								ev->focusing = true;
								if (mhover)
									ev->dragging = true;
							}
							if (thiz->f_mljustup)
							{
								thiz->f_mljustdown = false;
							}
						}

						thiz->f_all_done = !thiz->f_mljustdown && !thiz->f_mljustup && 
							!thiz->f_mrjustdown && !thiz->f_mrjustup && thiz->f_mdisp == 0 && thiz->f_mscroll == 0;
					}
				}));

			if (f_mljustdown)
			{
				focusing->focusing = false;
				focusing->dragging = false;
				focusing = nullptr;
			}

			for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
				mouse_buttons[i] &= ~KeyStateJust;

			mouse_pos_prev = mouse_pos;
			mouse_scroll = 0;
		}

		void setup(graphics::Canvas* canvas, Window* window)
		{
			canvas = canvas;
			window = window;

			mouse_pos = Ivec2(0);
			mouse_pos_prev = Ivec2(0);
			mouse_disp = Ivec2(0);
			mouse_scroll = 0;

			for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
				mouse_buttons[i] = KeyStateUp;

			auto thiz = this;

			if (window)
			{
				window->add_mouse_listener(Function<void(void*, KeyState, MouseKey, const Ivec2&)>(
					[](void* c, KeyState action, MouseKey key, const Ivec2 & pos) {
						auto thiz = *((cUIPrivate**)c);

						if (action == KeyStateNull)
						{
							if (key == Mouse_Middle)
								thiz->mouse_scroll = pos.x;
							else if (key == Mouse_Null)
								thiz->mouse_pos = pos;
						}
						else
						{
							thiz->mouse_buttons[key] = action | KeyStateJust;
							thiz->mouse_pos = pos;
						}
					}, sizeof(void*), &thiz));
			}
		}
	};

	cUI$::~cUI$()
	{
	}

	const char* cUI$::type_name() const
	{
		return "UI";
	}

	uint cUI$::type_hash() const
	{
		return cH("UI");
	}

	void cUI$::update(float delta_time)
	{
		((cUIPrivate*)this)->update(delta_time);
	}

	graphics::Canvas* cUI$::canvas() const
	{
		return ((cUIPrivate*)this)->canvas;
	}

	void cUI$::setup(graphics::Canvas* canvas, Window* window)
	{
		((cUIPrivate*)this)->setup(canvas, window);
	}

	cUI$* cUI$::create$(void* data)
	{
		return new cUIPrivate(data);
	}
}
