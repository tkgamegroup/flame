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

#include <flame/universe/universe.h>
#include <flame/universe/entity.h>

namespace flame
{
	static int world_frame = 0;

	int get_world_frame()
	{
		return world_frame;
	}

	void reset_world_frame()
	{
		world_frame = 0;
	}

	void update_world(Entity* root_node, float delta_time)
	{
		root_node->update(delta_time);
		world_frame++;
	}

	void traverse_forward_RE(Entity* node, Function<void(void* c, Entity* e)>& callback)
	{
		callback(node);
		for (auto i = 0; i < node->children_count(); i++)
			traverse_forward_RE(node->child(i), callback);
	}

	void traverse_forward(Entity* node, const Function<void(void* c, Entity* e)>& callback)
	{
		auto callback_ = callback;
		traverse_forward_RE(node, callback_);
	}

	void traverse_backward_RE(Entity* node, Function<void(void* c, Entity* e)>& callback)
	{
		for (auto i = 0; i < node->children_count(); i++)
			traverse_forward_RE(node->child(i), callback);
		callback(node);
	}

	void traverse_backward(Entity* node, const Function<void(void* c, Entity* e)>& callback)
	{
		auto callback_ = callback;
		traverse_backward_RE(node, callback_);
	}
}
