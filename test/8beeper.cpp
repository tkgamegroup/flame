#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>

#include <iostream>

using namespace flame;

auto freq = 4000U;
auto stereo = false;
auto _16bit = false;
auto wave_type = sound::WavaSin;
auto pitch = 220U;
auto dura = 0.1f;

const auto fade_time = 0.1f;

int main(int argc, char** args)
{
	auto device = sound::Device::create_player();
	auto context = sound::Context::create(device);
	context->make_current();

	while (true)
	{
		std::string line;
		std::getline(std::cin, line);

		auto sp = SUS::split(line);

		for (auto& t : sp)
		{
			if (t == "mono")
				stereo = false;
			else if (t == "stereo")
				stereo = true;
			else if (t == "8bit")
				_16bit = false;
			else if (t == "16bit")
				_16bit = true;
			else if (t == "sin")
				wave_type = sound::WavaSin;
			else if (t == "sqr")
				wave_type = sound::WavaSquare;
			else if (t == "tri")
				wave_type = sound::WavaTriangle;
			else if (t == "noi")
				wave_type = sound::WavaNoise;
			else
			{
				auto sp = SUS::split(t, ':');
				if (sp[0] == "freq")
					freq = std::stoi(sp[1]);
				else if (sp[0] == "dura")
					dura = std::stof(sp[1]);
				else if (sp[0] == "pit")
					pitch = std::stoi(sp[1]);
			}
		}

		if (dura < fade_time)
			dura = fade_time;

		std::vector<float> samples;
		samples.resize(sound::get_sample_count(dura, freq));
		auto size = sound::get_size(samples.size(), stereo, _16bit);
		auto data = new uchar[size];
		sound::wave(samples, freq, wave_type, pitch);
		sound::fill_data(data, freq, stereo, _16bit, samples, fade_time);

		auto buffer = sound::Buffer::create_from_data(data, freq, stereo, _16bit, dura);
		auto source = sound::Source::create(buffer);
		source->play();
		sleep(dura * 1000);
		source->stop();
		delete[]data;

		sound::Source::destroy(source);
		sound::Buffer::destroy(buffer);
	}

	return 0;
}
