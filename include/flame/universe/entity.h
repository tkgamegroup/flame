#pragma once

#include <flame/foundation/foundation.h>
#include <flame/universe/universe.h>

namespace flame
{
	struct TypeinfoDatabase;

	struct Component;

	struct Entity
	{
		int created_frame;

		bool visible;
		bool global_visible;

		bool first_update;

		FLAME_UNIVERSE_EXPORTS const std::string& name() const;
		FLAME_UNIVERSE_EXPORTS uint name_hash() const;
		FLAME_UNIVERSE_EXPORTS void set_name(const std::string& name) const;

		FLAME_UNIVERSE_EXPORTS uint component_count() const;
		FLAME_UNIVERSE_EXPORTS Component* component(uint index) const;
		FLAME_UNIVERSE_EXPORTS Component* find_component(uint type_hash) const;
		FLAME_UNIVERSE_EXPORTS Mail<std::vector<Component*>> find_components(uint type_hash /* 0 to get all components */ ) const;
		FLAME_UNIVERSE_EXPORTS void add_component(Component* c);

		FLAME_UNIVERSE_EXPORTS Entity* parent() const;
		FLAME_UNIVERSE_EXPORTS uint child_count() const;
		FLAME_UNIVERSE_EXPORTS int child_position(Entity* e) const; // -1 means do not exist
		FLAME_UNIVERSE_EXPORTS Entity* child(uint index) const;
		FLAME_UNIVERSE_EXPORTS Entity* find_child(const std::string& name) const;
		FLAME_UNIVERSE_EXPORTS void add_child(Entity* e, int position = -1); /* -1 is end */
		FLAME_UNIVERSE_EXPORTS void reposition_child(Entity* e, int position); /* -1 is last */
		FLAME_UNIVERSE_EXPORTS void remove_child(Entity* e);
		FLAME_UNIVERSE_EXPORTS void take_child(Entity* e); // no destroy
		FLAME_UNIVERSE_EXPORTS void remove_all_children();
		FLAME_UNIVERSE_EXPORTS void take_all_children();

		FLAME_UNIVERSE_EXPORTS Entity* copy();

		FLAME_UNIVERSE_EXPORTS void traverse_forward(void (*callback)(void* c, Entity* n), const Mail<>& capture);
		FLAME_UNIVERSE_EXPORTS void traverse_backward(void (*callback)(void* c, Entity* n), const Mail<>& capture);

		FLAME_UNIVERSE_EXPORTS void update();

		FLAME_UNIVERSE_EXPORTS static Entity* create();
		FLAME_UNIVERSE_EXPORTS static Entity* create_from_file(const std::vector<TypeinfoDatabase*>& dbs, const std::wstring& filename);
		FLAME_UNIVERSE_EXPORTS static void save_to_file(const std::vector<TypeinfoDatabase*>& dbs, Entity* e, const std::wstring& filename);
		FLAME_UNIVERSE_EXPORTS static void destroy(Entity* w);
	};

	FLAME_UNIVERSE_EXPORTS void* component_alloc(uint size); // for user-define component

	template<class T>
	T* new_component()
	{
		auto c = (T*)component_alloc(sizeof(T));
		new (c) T;
		return c;
	}

	FLAME_UNIVERSE_EXPORTS void universe_serialization_initialize();
	FLAME_UNIVERSE_EXPORTS void universe_serialization_set_data(const std::string& name, void* data);
	FLAME_UNIVERSE_EXPORTS void* universe_serialization_get_data(const std::string& name);
	FLAME_UNIVERSE_EXPORTS const std::string& universe_serialization_find_data(void* data);
}
