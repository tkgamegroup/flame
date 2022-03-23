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

			std::string node_name;
			std::vector<PositionKey> position_keys;
			std::vector<RotationKey> rotation_keys;
		};

		struct Animation
		{
			float duration;
			std::vector<Channel> channels;

			std::filesystem::path filename;

			virtual ~Animation() {}

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
