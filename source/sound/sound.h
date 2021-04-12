#pragma once

#ifdef FLAME_SOUND_MODULE
#define FLAME_SOUND_EXPORTS __declspec(dllexport)
template<class T, class U>
struct FlameSoundTypeSelector
{
	typedef U result;
};
#else
#define FLAME_SOUND_EXPORTS __declspec(dllimport)
template<class T, class U>
struct FlameSoundTypeSelector
{
	typedef T result;
};
#endif

#define FLAME_SOUND_TYPE(name) struct name; struct name##Private; \
	typedef FlameSoundTypeSelector<name*, name##Private*>::result name##Ptr;

#include "../foundation/foundation.h"

namespace flame
{
	namespace sound
	{
		FLAME_SOUND_TYPE(Device);
		FLAME_SOUND_TYPE(Recorder);
		FLAME_SOUND_TYPE(Buffer);
		FLAME_SOUND_TYPE(Source);

		inline uint get_samples_count(float duration, uint frequency)
		{
			return frequency * duration;
		}

		inline uint get_size(float duration, uint frequency, bool stereo, bool _16bit)
		{
			return get_samples_count(duration, frequency) * (stereo ? 2 : 1) * (_16bit ? 2 : 1);
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

		inline void wave(std::vector<float>& samples, uint frequency, WavaType type, uint pitch, float power = 1.f)
		{
			for (auto i = 0; i < samples.size(); i++)
			{
				auto t = fract((float)i / (float)frequency * (float)pitch);
				switch (type)
				{
				case WavaSin:
					samples[i] += cos(t * pi<float>() * 2.f) * power;
					break;
				case WavaSquare:
					samples[i] += (t < 0.5f ? 1.f : 0.f) * power;
					break;
				case WavaTriangle:
					samples[i] += (t < 0.5f ? t / 0.5f : 1.f - (t - 0.5f) / 0.5f) * power;
					break;
				case WavaNoise:
					samples[i] += ((float)rand() / (float)RAND_MAX) * power;
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
