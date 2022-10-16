#pragma once

#include "audio_source.h"

namespace flame
{
	struct cAudioSourcePrivate : cAudioSource
	{
		void set_buffer_name(const std::filesystem::path& buffer_name) override;
	};
}
