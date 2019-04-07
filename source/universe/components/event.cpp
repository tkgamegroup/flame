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
		cElement$* element_;
		
		cEventPrivate(void* data)
		{
			blackhole = false;
			want_key = false;
			clicked = 0;
		}

		void on_attach()
		{
			element_ = (cElement$*)(entity->component(cH("Element")));
			assert(element_);
		}

		void update(float delta_time)
		{
		}

		bool can_receive(const Vec2& mpos) const
		{
			return blackhole || (Rect::b(element_->pos_(), element_->size_()).contains(mpos));
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

	bool cEvent$::can_receive(const Vec2& mpos) const
	{
		return ((cEventPrivate*)this)->can_receive(mpos);
	}

	cEvent$* cEvent$::create$(void* data)
	{
		return new cEventPrivate(data);
	}
}
