#include "device_private.h"

namespace flame
{
	namespace sound
	{
		DevicePrivate* default_device = nullptr;

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

		Device* Device::create(bool as_default)
		{
			auto device = new DevicePrivate;
			if (as_default)
				default_device = device;
			return device;
		}

		Device* Device::get()
		{
			return default_device;
		}

		RecorderPrivate* default_recorder = nullptr;

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

		Recorder* Recorder::create(uint frequency, bool stereo, bool _16bit, float duration, bool as_default)
		{
			auto recorder = new RecorderPrivate(frequency, stereo, _16bit, duration);
			if (as_default)
				default_recorder = recorder;
			return recorder;
		}
	}
}
