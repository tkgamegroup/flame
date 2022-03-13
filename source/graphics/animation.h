#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Channel
		{
			struct Key
			{
				vec3 p;
				quat q;
			};

			std::string node_name;
			std::vector<Key> keys;
		};

		struct Animation
		{
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
