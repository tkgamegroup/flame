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
