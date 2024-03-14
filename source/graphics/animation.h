#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Channel
		{
			struct PositionKey
			{
				float t;
				vec3 p;
			};

			struct RotationKey
			{
				float t;
				quat q;
			};

			struct ScalingKey
			{
				float t;
				vec3 s;
			};

			std::string node_name;
			std::vector<PositionKey> position_keys;
			std::vector<RotationKey> rotation_keys;
			std::vector<ScalingKey> scaling_keys;
		};

		struct EventKey
		{
			std::string name;
			uint name_hash;
			float t;
		};

		struct Animation
		{
			float duration;
			std::vector<Channel> channels;
			std::vector<EventKey> events;

			std::filesystem::path filename;
			uint ref = 0;

			inline Channel* find_channel(const std::string& node_name)
			{
				for (auto& c : channels)
				{
					if (c.node_name == node_name)
						return &c;
				}
				return nullptr;
			}

			virtual ~Animation() {}

			virtual void save(const std::filesystem::path& filename) = 0;

			struct Get
			{
				virtual AnimationPtr operator()(const std::filesystem::path& filename) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Release
			{
				virtual void operator()(AnimationPtr animation) = 0;
			};
			FLAME_GRAPHICS_API static Release& release;
		};
	}
}
