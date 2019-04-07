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

#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event.h>

namespace flame
{
	struct cEventPrivate : cEvent$
	{
		cElement$* element;

		Array<Function<FoucusListenerParm>> focus_listeners$;
		Array<Function<KeyListenerParm>> key_listeners$;
		Array<Function<MouseListenerParm>> mouse_listeners$;
		Array<Function<DropListenerParm>> drop_listeners$;
		Array<Function<ChangedListenerParm>> changed_listeners$;
		Array<Function<ChildListenerParm>> child_listeners$;

		cEventPrivate(void* data) :
			element(nullptr)
		{
			blackhole = false;
			want_key = false;

			hovering = false;
			dragging = false;
			focusing = false;
		}

		void on_attach()
		{
			element = (cElement$*)(entity->component(cH("Element")));
			assert(element);
		}

		void update(float delta_time)
		{
		}

		bool contains(const Vec2& pos) const
		{
			return Rect::b(Vec2(element->global_x, element->global_y), Vec2(element->global_width, element->global_height)).contains(pos);
		}
	};

	cEvent$::~cEvent$()
	{
	}

	const char* cEvent$::type_name() const
	{
		return "Event";
	}

	uint cEvent$::type_hash() const
	{
		return cH("Event");
	}

	void cEvent$::on_attach()
	{
		((cEventPrivate*)this)->on_attach();
	}

	void cEvent$::update(float delta_time)
	{
		((cEventPrivate*)this)->update(delta_time);
	}

	bool cEvent$::contains(const Vec2& pos) const
	{
		return ((cEventPrivate*)this)->contains(pos);
	}

	cEvent$* cEvent$::create$(void* data)
	{
		return new cEventPrivate(data);
	}
}
