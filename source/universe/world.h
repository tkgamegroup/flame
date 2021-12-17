#pragma once

#include "universe.h"

namespace flame
{
	struct World
	{
		virtual ~World() {}

		std::vector<std::unique_ptr<System>> systems;

		std::unique_ptr<EntityT> root;
		EntityPtr first_element = nullptr;
		EntityPtr first_node = nullptr;

		inline System* get_system(uint type_hash) const
		{
			for (auto& s : systems)
			{
				if (s->type_hash == type_hash)
					return s.get();
			}
			return nullptr;
		}

		template<typename T> inline T* get_system_t() const { return (T*)get_system(T::type_hash); }

		inline System* find_system(std::string_view _name) const
		{
			System* ret = nullptr;
			for (auto& s : systems)
			{
				if (s->type_name == _name)
				{
					ret = s.get();
					break;
				}
			}
			auto name = "flame::" + std::string(_name);
			for (auto& s : systems)
			{
				if (s->type_name == name)
				{
					ret = s.get();
					break;
				}
			}
			return ret;
		}

		virtual void add_system(System* s) = 0;
		virtual void remove_system(System* s) = 0;

		virtual void update() = 0;

		FLAME_UNIVERSE_EXPORTS static World* create();
	};
}
