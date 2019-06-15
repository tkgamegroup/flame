#include "device_private.h"

namespace flame
{
	namespace sound
	{
		DevicePrivate::DevicePrivate(DeviceType t) :
			type(t),
			al_dev(nullptr)
		{
			if (type == DevicePlay)
				al_dev = alcOpenDevice(nullptr);
			else if (type == DeviceRecord)
				al_dev = alcCaptureOpenDevice(nullptr, 44100, AL_FORMAT_STEREO16, 44100 * 4 /* one second */);
		}

		DevicePrivate::~DevicePrivate()
		{
			alcCloseDevice(al_dev);
		}

		void DevicePrivate::start_record()
		{
			alcCaptureStart(al_dev);
		}

		int DevicePrivate::get_recorded_samples()
		{
			ALint samples;
			alcGetIntegerv(al_dev, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &samples);
			return samples;
		}

		void DevicePrivate::get_recorded_data(void* dst, int samples)
		{
			alcCaptureSamples(al_dev, (ALCvoid*)dst, samples);
		}

		void DevicePrivate::stop_record()
		{
			alcCaptureStop(al_dev);
		}

		void Device::start_record()
		{
			((DevicePrivate*)this)->start_record();
		}

		int Device::get_recorded_samples()
		{
			return ((DevicePrivate*)this)->get_recorded_samples();
		}

		void Device::get_recorded_data(void* dst, int samples)
		{
			((DevicePrivate*)this)->get_recorded_data(dst, samples);
		}

		void Device::stop_record()
		{
			((DevicePrivate*)this)->stop_record();
		}

		Device* Device::create(DeviceType t)
		{
			return new DevicePrivate(t);
		}

		void Device::destroy(Device *d)
		{
			delete (DevicePrivate*)d;
		}
	}
}
