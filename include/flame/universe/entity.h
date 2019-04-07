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

#include <flame/foundation/foundation.h>
#include <flame/universe/universe.h>

namespace flame
{
	struct Component;

	struct Entity
	{
		ATTRIBUTE_BOOL visible;
		ATTRIBUTE_BOOL global_visible;

		FLAME_UNIVERSE_EXPORTS const char* name() const;
		FLAME_UNIVERSE_EXPORTS void set_name(const char* name) const;

		FLAME_UNIVERSE_EXPORTS int component_count() const;
		FLAME_UNIVERSE_EXPORTS Component* component(uint type_hash) const;
		FLAME_UNIVERSE_EXPORTS Array<Component*> components(uint type_hash /* 0 to get all components */ ) const;
		FLAME_UNIVERSE_EXPORTS void add_component(Component* c);

		FLAME_UNIVERSE_EXPORTS Entity* parent() const;
		FLAME_UNIVERSE_EXPORTS int children_count() const;
		FLAME_UNIVERSE_EXPORTS Entity* child(int index) const;
		FLAME_UNIVERSE_EXPORTS void add_child(Entity* e);

		FLAME_UNIVERSE_EXPORTS void update(float delta_time);

		FLAME_UNIVERSE_EXPORTS void load(const wchar_t* filename);
		FLAME_UNIVERSE_EXPORTS void save(const wchar_t* filename);

		FLAME_UNIVERSE_EXPORTS static Entity* create();
		FLAME_UNIVERSE_EXPORTS static void destroy(Entity* w);
	};
}
