#include "buffer_private.h"

#include <flame/foundation/foundation.h>

namespace flame
{
	namespace sound
	{
		BufferPrivate::BufferPrivate(FILE* f, bool reverse) :
			al_buf(0)
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
				if (strncmp(riff_chunk.name, "fmt", 3) == 0)
					fread(&fmt, riff_chunk.size, 1, f);
				else if (strncmp(riff_chunk.name, "data", 4) == 0)
				{
					data_size = riff_chunk.size;
					data_offset = ftell(f);
					fseek(f, riff_chunk.size, SEEK_CUR);
				}
				else
					fseek(f, riff_chunk.size, SEEK_CUR);
			}

			alGenBuffers(1, &al_buf);

			auto data = new char[data_size];
			fseek(f, data_offset, SEEK_SET);
			fread(data, data_size, 1, f);
			if (reverse)
			{
				if (fmt.bits_per_sample == 8)
					std::reverse(data, data + data_size);
				else
					std::reverse((short*)data, (short*)(data + data_size));
			}

			int al_fmt;
			if (fmt.channel == 2)
			{
				if (fmt.bits_per_sample == 8)
					al_fmt = AL_FORMAT_STEREO8;
				else
					al_fmt = AL_FORMAT_STEREO16;
			}
			else
			{
				if (fmt.bits_per_sample == 8)
					al_fmt = AL_FORMAT_MONO8;
				else
					al_fmt = AL_FORMAT_MONO16;
			}

			alBufferData(al_buf, al_fmt, data, data_size, fmt.samples_per_sec);
			delete[]data;
		}

		BufferPrivate::BufferPrivate(int size, void* data) :
			al_buf(0)
		{
			alGenBuffers(1, &al_buf);
			alBufferData(al_buf, AL_FORMAT_STEREO16, data, size, 44100);
		}

		BufferPrivate::~BufferPrivate()
		{
			alDeleteBuffers(1, &al_buf);
		}

		Buffer* Buffer::create_from_data(int size, void* data)
		{
			return new BufferPrivate(size, data);
		}

		Buffer* Buffer::create_from_file(const wchar_t *filename, bool reverse)
		{
			auto file = _wfopen(filename, L"rb");
			if (!file)
				return nullptr;

			auto b = new BufferPrivate(file, reverse);

			fclose(file);

			return b;
		}

		void Buffer::destroy(Buffer *b)
		{
			delete (BufferPrivate*)b;
		}
	}
}
