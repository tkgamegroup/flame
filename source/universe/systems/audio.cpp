#include "audio_private.h"

namespace flame
{
	int sAudioPrivate::get_buffer_res(const std::filesystem::path& filename, int id)
	{
		return -1;
	}

	void sAudioPrivate::release_buffer_res(uint id)
	{

	}

	void sAudioPrivate::update()
	{

	}

	static sAudioPtr _instance = nullptr;

	struct sAudioInstance : sAudio::Instance
	{
		sAudioPtr operator()() override
		{
			return _instance;
		}
	}sAudio_instance;
	sAudio::Instance& sAudio::instance = sAudio_instance;

	struct sAudioCreate : sAudio::Create
	{
		sAudioPtr operator()(WorldPtr w) override
		{
			if (!w)
				return nullptr;

			assert(!_instance);
			_instance = new sAudioPrivate();
			return _instance;
		}
	}sAudio_create;
	sAudio::Create& sAudio::create = sAudio_create;
}
