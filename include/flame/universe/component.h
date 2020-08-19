#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	struct Component
	{
		const char* type_name;
		const uint64 type_hash;

		Entity* entity = nullptr;
		void* aux = nullptr;

		Component(const char* name, uint64 hash) :
			type_name(name),
			type_hash(hash)
		{
		}

		virtual ~Component() {}

		virtual bool check_refs() { return true; }
		virtual void on_added() {}
		virtual void on_removed() {}
		virtual void on_local_message(Message msg, void* p = nullptr) {}
		virtual void on_child_message(Message msg, void* p = nullptr) {}
		virtual void on_local_data_changed(Component* c, uint64 data_name_hash) {}
		virtual void on_child_data_changed(Component* c, uint64 data_name_hash) {}
	};
}
