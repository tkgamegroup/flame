#pragma once

#include "audio_source.h"

namespace flame
{
	struct BoundSource
	{
		audio::BufferPtr buf;
		audio::SourcePtr src;
	};

	struct cAudioSourcePrivate : cAudioSource
	{
		std::unordered_map<uint, BoundSource> sources;

		void set_buffer_names(const std::vector<std::pair<std::filesystem::path, std::string>>& names) override;

		~cAudioSourcePrivate();

		void play(uint name, float volumn) override;

		void update() override;
	};
}
