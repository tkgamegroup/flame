#include "audio_source_private.h"
#include "node_private.h"
#if USE_AUDIO_MODULE
#include "../../audio/buffer.h"
#endif

namespace flame
{
	void cAudioSourcePrivate::set_buffer_name(const std::filesystem::path& name)
	{
#if USE_AUDIO_MODULE
		if (buffer_name == name)
			return;

		buffer_name = name;

		if (!buffer_name.empty())
			AssetManagemant::release_asset(Path::get(buffer_name));
		if (!name.empty())
			AssetManagemant::get_asset(Path::get(name));

		audio::BufferPtr _buffer = nullptr;
		if (!name.empty())
			_buffer = audio::Buffer::get(name);
		if (buffer != _buffer)
		{
			if (buffer)
				audio::Buffer::release(buffer);
			buffer = _buffer;
		}
		else if (_buffer)
			audio::Buffer::release(_buffer);

		node->mark_transform_dirty();
		data_changed("buffer_name"_h);
#endif
	}
}
