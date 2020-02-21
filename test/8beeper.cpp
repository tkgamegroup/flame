#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>

#include <iostream>

using namespace flame;

int main(int argc, char** args)
{
	auto device = sound::Device::create_player();
	auto context = sound::Context::create(device);
	context->make_current();

	while (true)
	{
		std::string line;
		std::getline(std::cin, line);

		auto freq = 44100U;
		auto dura = 1.f;
		auto stereo = true;
		auto _16bit = true;
		auto sp = SUS::split(line);

		enum WavaType
		{
			WavaSin,
			WavaSquare,
			WavaTriangle
		};
		auto wave = WavaSin;

		auto pitch = 440U;

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
				wave = WavaSin;
			else if (t == "sqr")
				wave = WavaSquare;
			else if (t == "tri")
				wave = WavaTriangle;
			else
			{
				auto sp = SUS::split(t, ':');
				if (sp[0] == "freq")
					freq = std::stoi(sp[1]);
				else if (sp[0] == "dura")
					dura = std::stof(sp[1]);
				else if (sp[0] == "pitch")
					pitch = std::stoi(sp[1]);
			}
		}

		auto size = sound::sound_size(dura, freq, stereo, _16bit);
		auto data = new uchar[size];
		if (stereo)
		{
			if (_16bit)
				;
			else
			{
				for (auto i = 0; i < size; i++)
				{

				}
			}
		}
		else
		{
			if (_16bit)
				;
			else
			{
				for (auto i = 0; i < size; i++)
				{
					auto v = fract(((float)i / freq * pitch));
					if (v < 0.5f)
						data[i] = 255;
					else
						data[i] = 0;
				}
			}
		}
		auto buffer = sound::Buffer::create_from_data(data, freq, stereo, _16bit, dura);
		auto source = sound::Source::create(buffer);
		source->play();
		sleep(dura * 1000);
		delete[]data;
		sound::Buffer::destroy(buffer);
		sound::Source::destroy(source);
	}

	return 0;
}
