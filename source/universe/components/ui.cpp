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
#include <flame/universe/entity.h>
#include <flame/universe/components/event.h>
#include <flame/universe/components/ui.h>

namespace flame
{
	struct cUIPrivate : cUI$
	{
		graphics::Canvas* canvas_;
		Window* window_;

		Ivec2 mouse_pos_, mouse_pos_prev_, mouse_disp_;
		int mouse_scroll_;
		int mouse_buttons_[3];

		bool f_all_done_;
		bool f_mljustdown_, f_mljustup_, f_mrjustdown_, f_mrjustup_;
		Vec2 f_mpos_, f_mdisp_;
		int f_mscroll_;

		cUIPrivate(void* data) :
			canvas_(nullptr)
		{
			focus_ = nullptr;
		}

		void update(float delta_time)
		{
			mouse_disp_ = mouse_pos_ - mouse_pos_prev_;

			if (focus_)
			{
				if (!focus_->entity->visible_())
					focus_ = nullptr;
			}

			f_all_done_ = false;
			f_mljustdown_ = is_mouse_down((KeyState)mouse_buttons_[Mouse_Left], Mouse_Left, true);
			f_mljustup_ = is_mouse_up((KeyState)mouse_buttons_[Mouse_Left], Mouse_Left, true);
			f_mrjustdown_ = is_mouse_down((KeyState)mouse_buttons_[Mouse_Right], Mouse_Right, true);
			f_mrjustup_ = is_mouse_up((KeyState)mouse_buttons_[Mouse_Right], Mouse_Right, true);
			f_mpos_ = mouse_pos_;
			f_mdisp_ = mouse_disp_;
			f_mscroll_ = mouse_scroll_;

			if (f_mljustdown_)
				focus_ = nullptr;

			traverse_backward(entity, Function<void(void*, Entity*)>(
				[](void* c, Entity* e) {
					auto thiz = *((cUIPrivate**)c);
					if (thiz->f_all_done_)
						return;

					auto ev = (cEvent$*)e->component(cH("Event"));
					if (ev)
					{
						thiz->f_all_done_ = true;
						if (thiz->f_mljustdown_)
						{
							if (ev->can_receive(thiz->f_mpos_))
							{
								thiz->f_mljustdown_ = false;
								thiz->focus_ = ev;
							}
							else
								thiz->f_all_done_ = false;
						}
					}
				}));

			for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons_); i++)
				mouse_buttons_[i] &= ~KeyStateJust;

			mouse_pos_prev_ = mouse_pos_;
			mouse_scroll_ = 0;
		}

		void setup(graphics::Canvas* canvas, Window* window)
		{
			canvas_ = canvas;
			window_ = window;

			mouse_pos_ = Ivec2(0);
			mouse_pos_prev_ = Ivec2(0);
			mouse_disp_ = Ivec2(0);
			mouse_scroll_ = 0;

			for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons_); i++)
				mouse_buttons_[i] = KeyStateUp;

			auto thiz = this;

			if (window_)
			{
				window_->add_mouse_listener(Function<void(void*, KeyState, MouseKey, const Ivec2&)>(
					[](void* c, KeyState action, MouseKey key, const Ivec2 & pos) {
						auto thiz = *((cUIPrivate**)c);

						if (action == KeyStateNull)
						{
							if (key == Mouse_Middle)
								thiz->mouse_scroll_ = pos.x;
							else if (key == Mouse_Null)
								thiz->mouse_pos_ = pos;
						}
						else
						{
							thiz->mouse_buttons_[key] = action | KeyStateJust;
							thiz->mouse_pos_ = pos;
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
		return ((cUIPrivate*)this)->canvas_;
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
