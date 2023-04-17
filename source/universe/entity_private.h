#pragma once

#include "entity.h"

namespace flame
{
	struct EntityPrivate : Entity
	{
#ifdef FLAME_UNIVERSE_DEBUG
		int created_frame;
		int created_id;
		std::vector<StackFrameInfo> created_stack;
		uint created_location;
#endif

		EntityPrivate();
		~EntityPrivate();

		void update_enable();
		void set_enable(bool v) override;

		Component* add_component(uint hash) override;
		bool remove_component(uint hash) override;
		void remove_all_components() override;
		bool reposition_component(Component* comp) override;

		void add_child(EntityPtr e, int position = -1) override;
		void on_child_removed(EntityPrivate* e) const;
		void remove_child(EntityPtr e, bool destroy = true) override;
		void remove_all_children(bool destroy = true) override;

		EntityPtr copy(EntityPtr dst = nullptr) override;

		bool load(const std::filesystem::path& filename, bool only_root) override;
		bool save(const std::filesystem::path& filename, bool only_root) override;
	};
}
