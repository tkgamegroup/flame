#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>

#include <iostream>

using namespace flame;

enum WavaType
{
	WavaSin,
	WavaSquare,
	WavaTriangle,
	WavaNoise
};

auto freq = 4000U;
auto stereo = false;
auto _16bit = false;
auto wave = WavaSin;
auto pitch = 220U;
auto dura = 0.1f;

const auto fade_time = 0.1f;

float get_sin_wave(float t)
{
	return cos(t * M_PI * 2.f);
}

float get_square_wave(float t)
{
	float ret;
	if (t < 0.5f)
		ret = 0.9f;
	else
		ret = 0.1f;
	return ret;
}

float get_triangle_wave(float t)
{
	float ret;
	if (t < 0.5f)
		ret = t / 0.5f;
	else
		ret = 1.f - (t - 0.5f) / 0.5f;
	return ret;
}

float get_noise_wave(float t)
{
	return (float)rand() / RAND_MAX;
}

float get_wave(WavaType w, float t)
{
	auto pt = fract(t * pitch);
	float v;
	switch (w)
	{
	case WavaSin:
		v = get_sin_wave(pt);
		break;
	case WavaSquare:
		v = get_square_wave(pt);
		break;
	case WavaTriangle:
		v = get_triangle_wave(pt);
		break;
	case WavaNoise:
		v = get_noise_wave(pt);
		break;
	}
	auto power = 1.f;
	if (t > (dura - fade_time))
		power = (dura - t) / fade_time;
	return v * power;
}

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
				wave = WavaSin;
			else if (t == "sqr")
				wave = WavaSquare;
			else if (t == "tri")
				wave = WavaTriangle;
			else if (t == "noi")
				wave = WavaNoise;
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

		auto samples = sound::get_samples(dura, freq);
		auto size = sound::get_size(samples, stereo, _16bit);
		auto data = new uchar[size];
		if (stereo)
		{
			if (_16bit)
			{
				for (auto i = 0; i < samples; i++)
				{
					int v = get_wave(wave, (float)i / freq) * 32767;
					((short*)data)[i * 2 + 0] = v;
					((short*)data)[i * 2 + 1] = v;
				}
			}
			else
			{
				for (auto i = 0; i < samples; i++)
				{
					int v = get_wave(wave, (float)i / freq) * 127;
					data[i * 2 + 0] = v;
					data[i * 2 + 1] = v;
				}
			}
		}
		else
		{
			if (_16bit)
			{
				for (auto i = 0; i < samples; i++)
				{
					int v = get_wave(wave, (float)i / freq) * 32767;
					((ushort*)data)[i] = v;
				}
			}
			else
			{
				for (auto i = 0; i < samples; i++)
				{
					int v = (get_wave(wave, (float)i / freq) * 0.5f + 0.5f) * 255;
					data[i] = v;
				}
			}
		}

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
