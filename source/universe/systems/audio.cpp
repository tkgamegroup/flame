#include "audio_private.h"
#if USE_AUDIO_MODULE
#include "../../audio/device.h"
#endif

namespace flame
{
	sAudioPrivate::sAudioPrivate()
	{
#if USE_AUDIO_MODULE
		audio::Device::create();
#endif
	}

	sAudioPrivate::~sAudioPrivate()
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
