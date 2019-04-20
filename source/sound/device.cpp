// MIT License
// 
// Copyright (c) 2019 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "device_private.h"

namespace flame
{
	namespace sound
	{
		inline DevicePrivate::DevicePrivate(DeviceType t) :
			type(t),
			al_dev(nullptr)
		{
			if (type == DevicePlay)
				al_dev = alcOpenDevice(nullptr);
			else if (type == DeviceRecord)
				al_dev = alcCaptureOpenDevice(nullptr, 44100, AL_FORMAT_STEREO16, 44100 * 4 /* one second */);
		}

		inline DevicePrivate::~DevicePrivate()
		{
			alcCloseDevice(al_dev);
		}

		inline void DevicePrivate::start_record()
		{
			alcCaptureStart(al_dev);
		}

		inline int DevicePrivate::get_recorded_samples()
		{
			ALint samples;
			alcGetIntegerv(al_dev, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &samples);
			return samples;
		}

		inline void DevicePrivate::get_recorded_data(void* dst, int samples)
		{
			alcCaptureSamples(al_dev, (ALCvoid*)dst, samples);
		}

		inline void DevicePrivate::stop_record()
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
