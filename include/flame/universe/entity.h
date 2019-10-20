#pragma once

#include <flame/foundation/foundation.h>
#include <flame/universe/universe.h>

namespace flame
{
	struct TypeinfoDatabase;

	struct World;
	struct Component;

	struct Entity
	{
		World* world_;

		int created_frame_;
		bool dying_;

		bool visibility_;
		bool global_visibility_;

		FLAME_UNIVERSE_EXPORTS const std::string& name() const;
		FLAME_UNIVERSE_EXPORTS uint name_hash() const;
		FLAME_UNIVERSE_EXPORTS void set_name(const std::string& name) const;

		FLAME_UNIVERSE_EXPORTS void set_visibility(bool v);

		FLAME_UNIVERSE_EXPORTS Component* get_component_plain(uint type_hash) const;

		template<class T>
		T* get_component_t(uint type_hash) const
		{
			return (T*)get_component_plain(type_hash);
		}

#define get_component(T) get_component_t<c##T>(cH(#T))

		FLAME_UNIVERSE_EXPORTS Mail<std::vector<Component*>> get_components() const;
		FLAME_UNIVERSE_EXPORTS void add_component(Component* c);
		FLAME_UNIVERSE_EXPORTS void remove_component(Component* c);

		FLAME_UNIVERSE_EXPORTS Entity* parent() const;
		FLAME_UNIVERSE_EXPORTS uint child_count() const;
		FLAME_UNIVERSE_EXPORTS int child_position(Entity* e) const; // -1 means do not exist
		FLAME_UNIVERSE_EXPORTS Entity* child(uint index) const;
		FLAME_UNIVERSE_EXPORTS Entity* find_child(const std::string& name) const;
		FLAME_UNIVERSE_EXPORTS void add_child(Entity* e, int position = -1); /* -1 is end */
		FLAME_UNIVERSE_EXPORTS void reposition_child(Entity* e, int position); /* -1 is last */
		FLAME_UNIVERSE_EXPORTS void remove_child(Entity* e, bool destroy = true); /* if e==InvalidPointer, then remove all */

		FLAME_UNIVERSE_EXPORTS Entity* copy();

		FLAME_UNIVERSE_EXPORTS static Entity* create();
		FLAME_UNIVERSE_EXPORTS static Entity* create_from_file(Universe* u, const std::vector<TypeinfoDatabase*>& dbs, const std::wstring& filename);
		FLAME_UNIVERSE_EXPORTS static void save_to_file(const std::vector<TypeinfoDatabase*>& dbs, Entity* e, const std::wstring& filename);
		FLAME_UNIVERSE_EXPORTS static void destroy(Entity* w);
	};
}
