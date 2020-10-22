#include "device_private.h"

namespace flame
{
	namespace sound
	{
		thread_local DevicePrivate* default_device = nullptr;

		DevicePrivate::DevicePrivate()
		{
			al_dev = alcOpenDevice(nullptr);
			al_ctx = alcCreateContext(al_dev, nullptr);
			alcMakeContextCurrent(al_ctx);
		}

		DevicePrivate::~DevicePrivate()
		{
			alcDestroyContext(al_ctx);
			alcCloseDevice(al_dev);
		}

		Device* Device::create()
		{
			return new DevicePrivate;
		}

		Device* Device::get_default()
		{
			return default_device;
		}

		void Device::set_default(Device* device)
		{
			default_device = (DevicePrivate*)device;
		}

		RecorderPrivate::RecorderPrivate(uint frequency, bool stereo, bool _16bit, float duration)
		{
			al_dev = alcCaptureOpenDevice(nullptr, frequency, to_backend(stereo, _16bit), get_size(duration, frequency, stereo, _16bit));
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

		Recorder* Recorder::create(uint frequency, bool stereo, bool _16bit, float duration)
		{
			return new RecorderPrivate(frequency, stereo, _16bit, duration);
		}
	}
}
