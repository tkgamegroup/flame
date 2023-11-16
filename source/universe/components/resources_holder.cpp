#include "../../graphics/image.h"
#include "../../audio/buffer.h"
#include "resources_holder_private.h"

namespace flame
{
	cResourcesHolderPrivate::~cResourcesHolderPrivate()
	{
		for (auto& kv : graphics_images)
			graphics::Image::release(kv.second);
		for (auto& kv : audio_buffers)
			audio::Buffer::release(kv.second);
	}

	void cResourcesHolderPrivate::hold(const std::filesystem::path& path, uint name)
	{
		auto ext = path.extension();
		if (is_image_file(ext))
		{
			if (graphics_images.find(name) == graphics_images.end())
			{
				if (auto img = graphics::Image::get(path); img)
					graphics_images[name] = img;
			}
		}
		else if (is_audio_file(ext))
		{
			if (audio_buffers.find(name) == audio_buffers.end())
			{
				if (auto buf = audio::Buffer::get(path); buf)
					audio_buffers[name] = buf;
			}
		}
	}

	graphics::ImagePtr cResourcesHolderPrivate::get_graphics_image(uint name)
	{
		if (auto it = graphics_images.find(name); it != graphics_images.end())
			return it->second;
		return nullptr;
	}

	audio::BufferPtr cResourcesHolderPrivate::get_audio_buffer(uint name)
	{
		if (auto it = audio_buffers.find(name); it != audio_buffers.end())
			return it->second;
		return nullptr;
	}

	struct cResourcesHolderCreate : cResourcesHolder::Create
	{
		cResourcesHolderPtr operator()(EntityPtr) override
		{
			return new cResourcesHolderPrivate();
		}
	}cResourcesHolder_create;
	cResourcesHolder::Create& cResourcesHolder::create = cResourcesHolder_create;
}
