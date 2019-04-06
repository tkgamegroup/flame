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

#include <flame/foundation/foundation.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>

using namespace flame;

int main(int argc, char **args)
{
	//auto recorder = sound::Device::create(sound::DeviceRecord);
	//recorder->start_record();
	//sleep(1000);
	//recorder->stop_record();
	//auto samples = recorder->get_recorded_samples();
	//auto data = new ushort[samples * 2];
	//recorder->get_recorded_data(data, samples);

	//auto file = fopen("d:/record.txt", "w");
	//for (auto i = 0; i < samples * 2; i++)
	//	fprintf(file, "%d\n", data[i]);
	//fclose(file);
	auto samples = 44100 *2;
	auto data = new ushort[samples * 2];
	for (auto i = 0; i < samples * 2; i++)
		data[i] = 30000;
		//data[i] = sin(i) * 65535;

	auto device = sound::Device::create(sound::DevicePlay);
	auto context = sound::Context::create(device);
	context->make_current();
	auto buffer = sound::Buffer::create_from_data(samples * 4, data);
	auto source = sound::Source::create(buffer);
	source->play();

	sleep(2000);

	return 0;
}
