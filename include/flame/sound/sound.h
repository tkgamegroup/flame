#pragma once

#ifdef FLAME_SOUND_MODULE
#define FLAME_SOUND_EXPORTS __declspec(dllexport)
#else
#define FLAME_SOUND_EXPORTS __declspec(dllimport)
#endif

#include <flame/math.h>

namespace flame
{
	namespace sound
	{
		inline uint get_sample_count(float duration, uint frequency)
		{
			return frequency * duration;
		}

		inline uint get_size(float duration, uint frequency, bool stereo, bool _16bit)
		{
			return get_sample_count(duration, frequency) * (stereo ? 2 : 1) * (_16bit ? 2 : 1);
		}

		inline uint get_size(uint samples, bool stereo, bool _16bit)
		{
			return samples * (stereo ? 2 : 1) * (_16bit ? 2 : 1);
		}

		enum WavaType
		{
			WavaSin,
			WavaSquare,
			WavaTriangle,
			WavaNoise
		};

		inline void wave(std::vector<float>& samples, uint frequency, WavaType type, uint pitch)
		{
			for (auto i = 0; i < samples.size(); i++)
			{
				auto t = fract((float)i / (float)frequency * (float)pitch);
				switch (type)
				{
				case WavaSin:
					samples[i] += cos(t * M_PI * 2.f);
					break;
				case WavaSquare:
					samples[i] += t < 0.5f ? 1.f : 0.f;
					break;
				case WavaTriangle:
					samples[i] += t < 0.5f ? t / 0.5f : 1.f - (t - 0.5f) / 0.5f;
					break;
				case WavaNoise:
					samples[i] += (float)::rand() / (float)RAND_MAX;
					break;
				}
			}
		}

		inline void fill_data(void* dst, uint frequency, bool stereo, bool _16bit, const std::vector<float>& samples, float fade_time = 0.1f)
		{
			auto total_time = (float)samples.size() / (float)frequency;
			for (auto i = 0; i < samples.size(); i++)
			{
				auto t = (float)i / (float)frequency;
				auto v = samples[i] * (t > (total_time - fade_time) ? (total_time - t) / fade_time : 1.f);
				if (stereo)
				{
					if (_16bit)
					{
						v *= 32767;
						((short*)dst)[i * 2 + 0] = v;
						((short*)dst)[i * 2 + 1] = v;
					}
					else
					{
						v = (v * 0.5f + 0.5f) * 255.f;
						((uchar*)dst)[i * 2 + 0] = v;
						((uchar*)dst)[i * 2 + 1] = v;
					}
				}
				else
				{
					if (_16bit)
					{
						v *= 32767;
						((short*)dst)[i] = v;
					}
					else
					{
						v = (v * 0.5f + 0.5f) * 255.f;
						((uchar*)dst)[i] = v;
					}
				}
			}
		}
	}
}
