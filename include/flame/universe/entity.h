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
