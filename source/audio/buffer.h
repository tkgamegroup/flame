#pragma once

#include "audio.h"

namespace flame
{
	namespace audio
	{
		struct Buffer
		{
			std::filesystem::path filename;
			uint ref = 0;

			virtual ~Buffer() {}

			struct Get
			{
				virtual BufferPtr operator()(const std::filesystem::path& filename) = 0;
			};
			FLAME_AUDIO_API static Get& get;

			struct Release
			{
				virtual void operator()(BufferPtr buffer) = 0;
			};
			FLAME_AUDIO_API static Release& release;
		};
	}
}

