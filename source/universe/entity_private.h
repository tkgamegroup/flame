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

		Component* add_component_h(uint hash) override;
		void add_component_p(Component* comp) override;
		bool remove_component_h(uint hash) override;
		void remove_all_components() override;
		bool reposition_component(uint comp_index, int new_index) override;

		void add_child(EntityPtr e, int position = -1) override;
		void on_child_removed(EntityPrivate* e) const;
		void remove_child(EntityPtr e, bool destroy = true) override;
		void remove_all_children(bool destroy = true) override;

		EntityPtr duplicate(EntityPtr dst = nullptr) override;

		ModificationType parse_modification_target(const std::string& target, ModificationParsedData& out, voidptr& obj) override;
		bool load(const std::filesystem::path& filename, bool only_root = false) override;
		bool save(const std::filesystem::path& filename, bool only_root = false) override;
	};
}
