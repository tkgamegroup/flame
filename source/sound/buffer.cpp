#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include "buffer_private.h"

namespace flame
{
	namespace sound
	{
		BufferPrivate::BufferPrivate(void* data, uint frequency, bool stereo, bool _16bit, float duration)
		{
			alGenBuffers(1, &al_buf);
			alBufferData(al_buf, to_backend(stereo, _16bit), data, get_size(duration, frequency, stereo, _16bit), frequency);
		}

		BufferPrivate::BufferPrivate(FILE* f)
		{
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

			fread(&wave_header, sizeof(RIFF_Header), 1, f);
			while (fread(&riff_chunk, sizeof(RIFF_Chunk), 1, f) == 1)
			{
				auto chunk_name = std::string(riff_chunk.name);
				if (chunk_name.starts_with("fmt"))
					fread(&fmt, riff_chunk.size, 1, f);
				else if (chunk_name.starts_with("data"))
				{
					data_size = riff_chunk.size;
					data_offset = ftell(f);
					fseek(f, riff_chunk.size, SEEK_CUR);
				}
				else
					fseek(f, riff_chunk.size, SEEK_CUR);
			}

			alGenBuffers(1, &al_buf);

			auto data = new uchar[data_size];
			fseek(f, data_offset, SEEK_SET);
			fread(data, data_size, 1, f);

			alBufferData(al_buf, to_backend(fmt.channel == 2, fmt.bits_per_sample == 16), data, data_size, fmt.samples_per_sec);
			delete[]data;
		}

		BufferPrivate::~BufferPrivate()
		{
			alDeleteBuffers(1, &al_buf);
		}

		Buffer* Buffer::create_from_data(void* data, uint frequency, bool stereo, bool _16bit, float duration)
		{
			return new BufferPrivate(data, frequency, stereo, _16bit, duration);
		}

		Buffer* Buffer::create_from_file(const wchar_t* filename)
		{
			auto file = _wfopen(filename, L"rb");
			if (!file)
				return nullptr;

			auto b = new BufferPrivate(file);

			fclose(file);

			return b;
		}

		void Buffer::destroy(Buffer *b)
		{
			delete (BufferPrivate*)b;
		}
	}
}
