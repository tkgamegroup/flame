#include "buffer_private.h"

namespace flame
{
	namespace sound
	{
		BufferPrivate::~BufferPrivate()
		{
			alDeleteBuffers(1, &al_buf);
		}

		struct BufferCreate : Buffer::Create
		{
			BufferPtr operator()(void* data, uint frequency, bool stereo, bool _16bit, float duration) override
			{
				auto ret = new BufferPrivate;
				alGenBuffers(1, &ret->al_buf);
				alBufferData(ret->al_buf, to_backend(stereo, _16bit), data, get_size(duration, frequency, stereo, _16bit), frequency);
				return ret;
			}

			BufferPtr operator()(const std::filesystem::path& filename) override
			{
				if (!std::filesystem::exists(filename))
					return nullptr;

				struct RIFF_Header
				{
					char RIFF_name[4];
					int RIFF_size;
					char WAVE_name[4];
				};

				struct RIFF_Chunk
				{
					char name[4];
					int size;
				};

				struct Wave_FMT
				{
					short tag;
					short channel;
					int samples_per_sec;
					int bytes_per_sec;
					short align;
					short bits_per_sample;
				};

				RIFF_Header wave_header;
				RIFF_Chunk riff_chunk;
				Wave_FMT fmt;
				int data_size;
				int data_offset;

				std::ifstream file(filename, std::ios::binary);

				read_t(file, wave_header);
				while (!file.eof())
				{
					read_t(file, riff_chunk);

					auto chunk_name = std::string(riff_chunk.name);
					if (chunk_name.starts_with("fmt"))
						read_t(file, riff_chunk.size);
					else if (chunk_name.starts_with("data"))
					{
						data_size = riff_chunk.size;
						data_offset = file.tellg();
						file.seekg(riff_chunk.size, std::ios::cur);
					}
					else
						file.seekg(riff_chunk.size, std::ios::cur);
				}

				auto data = new char[data_size];
				file.seekg(data_offset);
				file.read(data, data_size);
				auto ret = new BufferPrivate;
				alGenBuffers(1, &ret->al_buf);
				alBufferData(ret->al_buf, to_backend(fmt.channel == 2, fmt.bits_per_sample == 16), data, data_size, fmt.samples_per_sec);
				delete[]data;

				return ret;
			}
		}Buffer_create;
		Buffer::Create& Buffer::create = Buffer_create;
	}
}
