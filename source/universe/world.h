#pragma once

#include "system.h"

namespace flame
{
	struct World
	{
		virtual ~World() {}

		std::unordered_map<uint, std::unique_ptr<System>> systems;
		std::vector<System*> system_list;

		std::unique_ptr<EntityT> root;
		EntityPtr first_element = nullptr;
		EntityPtr first_node = nullptr;

		inline System* get_system(uint type_hash) const
		{
			auto it = systems.find(type_hash);
			if (it != systems.end())
				return it->second.get();
			return nullptr;
		}

		template<typename T> inline T* get_system_t() const { return (T*)get_system(T::type_hash); }

		inline System* find_system(std::string_view _name) const
		{
			System* ret = nullptr;
			for (auto& s : system_list)
			{
				if (s->type_name == _name)
					return s;
			}
			auto name = "flame::" + std::string(_name);
			for (auto& s : system_list)
			{
				if (s->type_name == name)
					return s;
			}
			return nullptr;
		}

		virtual void add_system(System* s) = 0;
		virtual void remove_system(System* s, bool destroy = true) = 0;

		virtual void update() = 0;

		FLAME_UNIVERSE_EXPORTS static World* create();
	};
}
