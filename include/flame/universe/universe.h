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

#pragma once

#ifdef FLAME_UNIVERSE_MODULE
#define FLAME_UNIVERSE_EXPORTS __declspec(dllexport)
#else
#define FLAME_UNIVERSE_EXPORTS __declspec(dllimport)
#endif

#include <flame/foundation/foundation.h>

namespace flame
{
	struct Entity;

	FLAME_UNIVERSE_EXPORTS int get_world_frame();
	FLAME_UNIVERSE_EXPORTS void reset_world_frame();
	FLAME_UNIVERSE_EXPORTS void update_world(Entity* root_node, float delta_time);
	FLAME_UNIVERSE_EXPORTS void traverse_forward(Entity* node, const Function<void(void* c, Entity* e)>& callback);
	FLAME_UNIVERSE_EXPORTS void traverse_backward(Entity* node, const Function<void(void* c, Entity* e)>& callback);

	struct ATTRIBUTE_BOOL
	{
		bool val;
		int frame;

		ATTRIBUTE_BOOL() :
			frame(-1)
		{
		}

		operator bool() const
		{
			return val;
		}

		void operator=(bool rhs)
		{
			val = rhs;
			frame = get_world_frame();
		}

		void operator&=(bool rhs)
		{
			val &= rhs;
			frame = get_world_frame();
		}

		void operator|=(bool rhs)
		{
			val |= rhs;
			frame = get_world_frame();
		}
	};

	template <class T>
	struct ATTRIBUTE_NUMBER
	{
		T val;
		int frame;

		ATTRIBUTE_NUMBER() :
			frame(-1)
		{
		}

		operator T() const
		{
			return val;
		}

		void operator=(const T& rhs)
		{
			val = rhs;
			frame = get_world_frame();
		}

		void operator+=(const T& rhs)
		{
			val += rhs;
			frame = get_world_frame();
		}

		void operator-=(const T& rhs)
		{
			val -= rhs;
			frame = get_world_frame();
		}

		void operator*=(const T& rhs)
		{
			val *= rhs;
			frame = get_world_frame();
		}

		void operator/=(const T& rhs)
		{
			val /= rhs;
			frame = get_world_frame();
		}
	};
}

