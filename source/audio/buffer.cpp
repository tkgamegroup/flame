#include "buffer_private.h"

namespace flame
{
	namespace audio
	{
		std::vector<std::unique_ptr<BufferT>> loaded_buffers;

		BufferPrivate::~BufferPrivate()
		{
			alDeleteBuffers(1, &al_buf);
		}

		struct BufferGet : Buffer::Get
		{
			BufferPtr operator()(const std::filesystem::path& _filename) override
			{
				auto filename = Path::get(_filename);

				for (auto& b : loaded_buffers)
				{
					if (b->filename == filename)
					{
						b->ref++;
						return b.get();
					}
				}

				if (!std::filesystem::exists(filename))
				{
					wprintf(L"cannot find audio: %s\n", _filename.c_str());
					return nullptr;
				}

				auto ext = filename.extension();
				if (ext == L".wav")
				{
					enum FILE_TYPE
					{
						WF_EX,
						WF_EXT
					};
					uint file_type = 0;

					struct WAV_HEADER
					{
						char szRIFF[4];
						uint ulRIFFSize;
						char szWAVE[4];
					}header;

					struct RIFF_CHUNK
					{
						char szChunkName[4];
						uint ulChunkSize;
					}riff_chunk;

					struct GUID
					{
						uint  Data1;
						ushort Data2;
						ushort Data3;
						uchar  Data4[8];
					};

					struct WAV_FMT
					{
						ushort usFormatTag;
						ushort usChannels;
						uint ulSamplesPerSec;
						uint ulAvgBytesPerSec;
						ushort usBlockAlign;
						ushort usBitsPerSample;
						ushort usSize;
						ushort usReserved;
						uint ulChannelMask;
						GUID guidSubFormat;
					}fmt;

					std::string pcm_data;

					auto file = _wfopen(filename.c_str(), L"rb");
					fread(&header, 1, sizeof(WAV_HEADER), file);
					if (strncmp(header.szRIFF, "RIFF", 4) == 0 && strncmp(header.szWAVE, "WAVE", 4) == 0)
					{
						while (fread(&riff_chunk, 1, sizeof(RIFF_CHUNK), file) == sizeof(RIFF_CHUNK))
						{
							if (strncmp(riff_chunk.szChunkName, "fmt ", 4) == 0)
							{
								if (riff_chunk.ulChunkSize <= sizeof(WAV_FMT))
								{
									fread(&fmt, 1, riff_chunk.ulChunkSize, file);
									if (fmt.usFormatTag == 1 /*PCM*/)
									{
										file_type = WF_EX;
									}
									else if (fmt.usFormatTag == 0xFFFE /*extensible*/)
									{
										file_type = WF_EXT;
										// TODO
										return nullptr;
									}
								}
								else
									fseek(file, riff_chunk.ulChunkSize, SEEK_CUR);
							}
							else if (strncmp(riff_chunk.szChunkName, "data", 4) == 0)
							{
								pcm_data.resize(riff_chunk.ulChunkSize);
								fread(pcm_data.data(), 1, pcm_data.size(), file);
							}
							else
								fseek(file, riff_chunk.ulChunkSize, SEEK_CUR);

							// Ensure that we are correctly aligned for next chunk
							if (riff_chunk.ulChunkSize & 1)
								fseek(file, 1, SEEK_CUR);
						}
					}
					else
					{
						wprintf(L"cannot open wav: %s\n", _filename.c_str());
						return nullptr;
					}
					fclose(file);

					uint al_fmt = 0;
					switch (fmt.usChannels)
					{
					case 1:
						switch (fmt.usBitsPerSample)
						{
						case 8:
							al_fmt = AL_FORMAT_MONO8;
							break;
						case 16:
							al_fmt = AL_FORMAT_MONO16;
							break;
						}
						break;
					case 2:
						switch (fmt.usBitsPerSample)
						{
						case 8:
							al_fmt = AL_FORMAT_STEREO8;
							break;
						case 16:
							al_fmt = AL_FORMAT_STEREO16;
							break;
						case 24:
						{
							al_fmt = AL_FORMAT_STEREO16;

							auto samples = (uint)pcm_data.size() / 3;
							std::string new_pcm_data;
							new_pcm_data.resize(samples * 2);

							for (auto i = 0; i < samples; i++)
							{
								union
								{
									uchar b[4];
									uint  i;
								}d;
								d.b[0] = pcm_data[i * 3 + 0];
								d.b[1] = pcm_data[i * 3 + 1];
								d.b[2] = pcm_data[i * 3 + 2];
								d.b[3] = 0;

								union
								{
									uchar b[2];
									ushort s;
								}v;
								v.s = ((double)d.i / (0x00ffffff) * 0xffff);
								new_pcm_data[i * 2 + 0] = v.b[0];
								new_pcm_data[i * 2 + 1] = v.b[1];
							}

							pcm_data = std::move(new_pcm_data);
						}
							break;
						}
						break;
					}
					if (al_fmt == 0)
					{
						wprintf(L"unknown wav format: %s\n", _filename.c_str());
						return nullptr;
					}

					auto ret = new BufferPrivate;
					alGenBuffers(1, &ret->al_buf);
					alBufferData(ret->al_buf, al_fmt, pcm_data.data(), pcm_data.size(), fmt.ulSamplesPerSec);
					return ret;
				}
				//else if (ext == L".flac") TODO
				{

				}

				wprintf(L"cannot open audio: %s, unknown format\n", _filename.c_str());

				return nullptr;
			}
		}Buffer_get;
		Buffer::Get& Buffer::get = Buffer_get;

		struct BufferRelease : Buffer::Release
		{
			void operator()(BufferPtr buffer) override
			{
				if (buffer->ref == 1)
				{
					std::erase_if(loaded_buffers, [&](const auto& b) {
						return b.get() == buffer;
					});
				}
				else
					buffer->ref--;
			}
		}Buffer_release;
		Buffer::Release& Buffer::release = Buffer_release;
	}
}
