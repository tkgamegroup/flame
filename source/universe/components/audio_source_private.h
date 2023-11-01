#pragma once

#include "audio_source.h"

namespace flame
{
	struct BoundSource
	{
#if USE_AUDIO_MODULE
		audio::BufferPtr buf;
		audio::SourcePtr src;
#endif
	};

	struct cAudioSourcePrivate : cAudioSource
	{
		std::unordered_map<uint, BoundSource> sources;

		void set_buffer_names(const std::vector<std::pair<std::filesystem::path, std::string>>& names) override;

		void add_buffer_name(const std::filesystem::path& path, const std::string& name) override;

		~cAudioSourcePrivate();

		void play(uint name, float volumn) override;
		void stop(uint name) override;
		void pause(uint name) override;
		void rewind(uint name) override;

		void update() override;
	};
}
