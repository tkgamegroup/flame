#pragma once

#include "entity.h"
#include "component.h"

#include <functional>

namespace flame
{
	struct EntityPrivate : Entity
	{
		std::vector<std::filesystem::path> srcs;
		std::wstring srcs_str;

		int created_frame;
		int created_id;
#ifdef FLAME_UNIVERSE_DEBUG
		std::vector<StackFrameInfo> created_stack;
#endif
		uint created_location;

		std::vector<std::list<std::function<void(uint)>>> components_listeners;
		std::list<std::function<void(uint, void*, void*)>> message_listeners;

		EntityPrivate();
		~EntityPrivate();

		void update_visibility();
		void set_visible(bool v) override;

		void set_state(StateFlags state) override;

		void add_src(const std::filesystem::path& p);
		const wchar_t* get_srcs() const override;
		std::filesystem::path get_src(uint id) const { return srcs[srcs.size() - id - 1]; }

		void add_component(Component* c);
		void remove_component(Component* c, bool destroy = true);

		void add_child(EntityPtr e, int position = -1) override;
		void reposition_child(uint pos1, uint pos2) override;
		void on_child_removed(EntityPrivate* e) const;
		void remove_child(EntityPtr e, bool destroy = true) override;
		void remove_all_children(bool destroy = true) override;

		void on_entered_world(WorldPrivate* world);
		void on_left_world();

		void traversal(const std::function<void(EntityPrivate*)>& callback);

		void* add_message_listener(void (*callback)(Capture& c, uint msg, void* parm1, void* parm2), const Capture& capture) override;
		void remove_message_listener(void* lis) override;

		void component_data_changed(Component* c, uint h) override;
		void* add_component_data_listener(void (*callback)(Capture& c, uint hash), const Capture& capture, Component* c) override;
		void remove_component_data_listener(void* lis, Component* c) override;

		void* add_event(void (*callback)(Capture& c), const Capture& capture, float interval = 0.f) override;
		void remove_event(void* ev) override;

		EntityPtr copy() override;
		bool load(const std::filesystem::path& filename);
		bool load(const wchar_t* filename) override { return load(std::filesystem::path(filename)); }
		bool save(const std::filesystem::path& filename);
		bool save(const wchar_t* filename) override { return save(std::filesystem::path(filename)); }
	};
}
