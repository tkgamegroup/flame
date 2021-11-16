#include "device_private.h"

namespace flame
{
	namespace sound
	{
		DevicePrivate* default_device = nullptr;

		DevicePrivate::~DevicePrivate()
		{
			alcDestroyContext(al_ctx);
			alcCloseDevice(al_dev);
		}

		DevicePtr Device::create()
		{
			auto ret = new DevicePrivate;
			ret->al_dev = alcOpenDevice(nullptr);
			ret->al_ctx = alcCreateContext(ret->al_dev, nullptr);
			alcMakeContextCurrent(ret->al_ctx);
			if (!default_device)
				default_device = ret;
			return ret;
		}

		RecorderPrivate::~RecorderPrivate()
		{
			alcCloseDevice(al_dev);
		}

		void RecorderPrivate::start_record()
		{
			alcCaptureStart(al_dev);
		}

		void RecorderPrivate::stop_record(void* dst)
		{
			alcCaptureStop(al_dev);
			ALint samples;
			alcGetIntegerv(al_dev, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &samples);
			alcCaptureSamples(al_dev, (ALCvoid*)dst, samples);
		}

		RecorderPtr Recorder::create(uint frequency, bool stereo, bool _16bit, float duration)
		{
			auto ret = new RecorderPrivate;
			ret->al_dev = alcCaptureOpenDevice(nullptr, frequency, to_backend(stereo, _16bit), get_size(duration, frequency, stereo, _16bit));
			return ret;
		}
	}
}
