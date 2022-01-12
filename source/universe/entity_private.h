#pragma once

#include "entity.h"

namespace flame
{
	struct EntityPrivate : Entity
	{
		Guid guid;

		int created_frame;
		int created_id;
#ifdef FLAME_UNIVERSE_DEBUG
		std::vector<StackFrameInfo> created_stack;
#endif
		uint created_location;

		EntityPrivate();
		~EntityPrivate();

		void update_enable();
		void set_enable(bool v) override;

		Component* add_component(uint hash) override;
		void remove_component(uint hash, bool destroy = true) override;

		void add_child(EntityPtr e, int position = -1) override;
		void on_child_removed(EntityPrivate* e) const;
		void remove_child(EntityPtr e, bool destroy = true) override;
		void remove_all_children(bool destroy = true) override;

		EntityPtr copy() override;
		bool load(const std::filesystem::path& filename) override;
		bool save(const std::filesystem::path& filename) override;
	};
}
