#include "audio_private.h"
#if USE_AUDIO_MODULE
#include "../../audio/device.h"
#include "../../audio/buffer.h"
#include "../../audio/source.h"
#endif

namespace flame
{
	sAudioPrivate::sAudioPrivate()
	{
#if USE_AUDIO_MODULE
		audio::Device::create();
#endif
	}

	sAudioPrivate::sAudioPrivate(int)
	{
	}

	sAudioPrivate::~sAudioPrivate()
	{
	}

	void sAudioPrivate::play_once(const std::filesystem::path& path)
	{
		if (auto buf = audio::Buffer::get(path); buf)
		{
			auto src = audio::Source::create();
			src->add_buffer(buf);
			src->play();

			add_event([buf, src]() {
				if (src->is_player())
					return true;
				delete src;
				audio::Buffer::release(buf);
				return false;
			});
		}
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
				return new sAudioPrivate(0);

			assert(!_instance);
			_instance = new sAudioPrivate();
			return _instance;
		}
	}sAudio_create;
	sAudio::Create& sAudio::create = sAudio_create;
}
