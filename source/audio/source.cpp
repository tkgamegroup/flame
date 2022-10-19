#include "source_private.h"
#include "buffer_private.h"

namespace flame
{
	namespace audio
	{
		void SourcePrivate::set_pos(const vec3& _pos)
		{
			if (pos == _pos)
				return;
			pos = _pos;
			alSourcefv(al_src, AL_POSITION, &pos[0]);
		}

		void SourcePrivate::play()
		{
			alSourcePlay(al_src);
		}

		void SourcePrivate::stop()
		{
			alSourceStop(al_src);
		}

		struct SourceGet : Source::Create
		{
			SourcePtr operator()(BufferPtr buffer) override
			{
				auto ret = new SourcePrivate;
				alGenSources(1, &ret->al_src);
				alSourceQueueBuffers(ret->al_src, 1, &buffer->al_buf);
				return ret;
			}
		};
	}
}
