// MIT License
// 
// Copyright (c) 2018 wjs
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

#pragma once

#include <flame/sound/sound.h>

namespace flame
{
	namespace sound
	{
		enum DeviceType
		{
			DevicePlay,
			DeviceRecord
		};

		struct Device
		{
			FLAME_SOUND_EXPORTS void start_record();
			FLAME_SOUND_EXPORTS int get_recorded_samples();
			FLAME_SOUND_EXPORTS void get_recorded_data(void* dst, int samples);
			FLAME_SOUND_EXPORTS void stop_record();

			FLAME_SOUND_EXPORTS static Device* create(DeviceType t);
			FLAME_SOUND_EXPORTS static void destroy(Device* d);
		};
	}
}
