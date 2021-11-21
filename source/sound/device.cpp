#include "device_private.h"

namespace flame
{
	namespace sound
	{
		DevicePtr current_device = nullptr;

		DevicePrivate::~DevicePrivate()
		{
			alcDestroyContext(al_ctx);
			alcCloseDevice(al_dev);
		}

		struct DeviceCreatePrivate : Device::Create
		{
			DevicePtr operator()() override
			{
				auto ret = new DevicePrivate;
				ret->al_dev = alcOpenDevice(nullptr);
				ret->al_ctx = alcCreateContext(ret->al_dev, nullptr);
				alcMakeContextCurrent(ret->al_ctx);
				if (!current_device)
					current_device = ret;
				return ret;
			}
		}device_create_private;
		Device::Create& Device::create = device_create_private;

		struct DeviceCurrentPrivate : Device::Current
		{
			DevicePtr& operator()() override
			{
				return current_device;
			}
		}device_current_private;
		Device::Current& Device::current = device_current_private;

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

		struct RecorderCreatePrivate : Recorder::Create
		{
			RecorderPtr operator()(uint frequency, bool stereo, bool _16bit, float duration) override
			{
				auto ret = new RecorderPrivate;
				ret->al_dev = alcCaptureOpenDevice(nullptr, frequency, to_backend(stereo, _16bit), get_size(duration, frequency, stereo, _16bit));
				return ret;
			}
		}recorder_create_private;
		Recorder::Create& Recorder::create = recorder_create_private;
	}
}
