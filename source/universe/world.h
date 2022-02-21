#pragma once

#include "system.h"

namespace flame
{
	struct World
	{
		bool update_components = true;
		bool update_systems = true;

		virtual ~World() {}

		std::unordered_map<uint, System*> system_map;
		std::vector<std::unique_ptr<System>> systems;

		std::unique_ptr<EntityT> root;

		inline System* get_system(uint type_hash) const
		{
			auto it = system_map.find(type_hash);
			if (it != system_map.end())
				return it->second;
			return nullptr;
		}

		template<typename T> 
		inline T* get_system_t() const 
		{ 
			return (T*)get_system(th<T>()); 
		}

		virtual System* add_system(uint hash) = 0;
		virtual void remove_system(uint hash, bool destroy = true) = 0;

		virtual void update() = 0;

		struct Instance
		{
			virtual WorldPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Instance& instance;

		struct Create
		{
			virtual WorldPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Create& create;
	};
}
