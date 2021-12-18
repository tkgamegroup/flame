#pragma once

#include "entity.h"

namespace flame
{
	struct EntityPrivate : Entity
	{
		int created_frame;
		int created_id;
#ifdef FLAME_UNIVERSE_DEBUG
		std::vector<StackFrameInfo> created_stack;
#endif
		uint created_location;

		EntityPrivate();
		~EntityPrivate();

		void update_visibility();
		void set_visible(bool v) override;

		void set_state(StateFlags state) override;

		void add_component(Component* c);
		void remove_component(Component* c, bool destroy = true);

		void add_child(EntityPtr e, int position = -1) override;
		void on_child_removed(EntityPrivate* e) const;
		void remove_child(EntityPtr e, bool destroy = true) override;
		void remove_all_children(bool destroy = true) override;

		void on_entered_world(WorldPrivate* world);
		void on_left_world();

		EntityPtr copy() override;
		bool load(const std::filesystem::path& filename) override;
		bool save(const std::filesystem::path& filename) override;
	};
}
