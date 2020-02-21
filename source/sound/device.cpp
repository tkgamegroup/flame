#include "device_private.h"

namespace flame
{
	namespace sound
	{
		DevicePrivate::DevicePrivate()
		{
			al_dev = alcOpenDevice(nullptr);
		}

		DevicePrivate::DevicePrivate(uint frequency, bool stereo, bool _16bit, float duration)
		{
			al_dev = alcCaptureOpenDevice(nullptr, frequency, to_backend(stereo, _16bit), sound_size(frequency, stereo, _16bit, duration));
		}

		DevicePrivate::~DevicePrivate()
		{
			alcCloseDevice(al_dev);
		}

		void DevicePrivate::start_record()
		{
			alcCaptureStart(al_dev);
		}

		void DevicePrivate::stop_record(void* dst)
		{
			alcCaptureStop(al_dev);
			ALint samples;
			alcGetIntegerv(al_dev, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &samples);
			alcCaptureSamples(al_dev, (ALCvoid*)dst, samples);
		}

		void Device::start_record()
		{
			((DevicePrivate*)this)->start_record();
		}

		void Device::stop_record(void* dst)
		{
			((DevicePrivate*)this)->stop_record(dst);
		}

		Device* Device::create_player()
		{
			return new DevicePrivate();
		}

		Device* Device::create_recorder(uint frequency, bool stereo, bool _16bit, float duration)
		{
			return new DevicePrivate(frequency, stereo, _16bit, duration);
		}

		void Device::destroy(Device *d)
		{
			delete (DevicePrivate*)d;
		}
	}
}
