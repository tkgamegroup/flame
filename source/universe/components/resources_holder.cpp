#include "../../graphics/image.h"
#include "../../graphics/material.h"
#include "../../audio/buffer.h"
#include "resources_holder_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cResourcesHolderPrivate::~cResourcesHolderPrivate()
	{
		for (auto& kv : graphics_images)
			graphics::Image::release(kv.second);
		for (auto& kv : graphics_image_atlases)
			graphics::ImageAtlas::release(kv.second);
		for (auto& kv : graphics_materials)
		{
			if (kv.second.second != -1)
				sRenderer::instance()->release_material_res(kv.second.second);
			graphics::Material::release(kv.second.first);
		}
#if USE_AUDIO_MODULE
		for (auto& kv : audio_buffers)
			audio::Buffer::release(kv.second);
#endif
	}

	void cResourcesHolderPrivate::hold(const std::filesystem::path& path, uint name)
	{
		auto ext = path.extension();
		if (is_image_file(ext))
		{
			auto atlas_path = Path::get(path);
			atlas_path += L".ini";
			if (std::filesystem::exists(atlas_path))
			{
				if (graphics_image_atlases.find(name) == graphics_image_atlases.end())
				{
					if (auto atlas = graphics::ImageAtlas::get(path); atlas)
						graphics_image_atlases[name] = atlas;
				}
			}
			else
			{
				if (graphics_images.find(name) == graphics_images.end())
				{
					if (auto img = graphics::Image::get(path); img)
						graphics_images[name] = img;
				}
			}
		}
		else if (ext == L".fmat")
		{
			if (graphics_materials.find(name) == graphics_materials.end())
			{
				if (auto mat = graphics::Material::get(path); mat)
				{
					auto res_id = sRenderer::instance()->get_material_res(mat, -1);
					graphics_materials[name] = { mat, res_id };
				}
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

	graphics::ImageAtlasPtr cResourcesHolderPrivate::get_graphics_image_atlas(uint name)
	{
		if (auto it = graphics_image_atlases.find(name); it != graphics_image_atlases.end())
			return it->second;
		return nullptr;
	}

	graphics::MaterialPtr cResourcesHolderPrivate::get_material(uint name)
	{
		if (auto it = graphics_materials.find(name); it != graphics_materials.end())
			return it->second.first;
		return nullptr;
	}

	int cResourcesHolderPrivate::get_material_res_id(uint name)
	{
		if (auto it = graphics_materials.find(name); it != graphics_materials.end())
			return it->second.second;
		return -1;
	}

#if USE_AUDIO_MODULE
	audio::BufferPtr cResourcesHolderPrivate::get_audio_buffer(uint name)
	{
		if (auto it = audio_buffers.find(name); it != audio_buffers.end())
			return it->second;
		return nullptr;
	}
#endif

	struct cResourcesHolderCreate : cResourcesHolder::Create
	{
		cResourcesHolderPtr operator()(EntityPtr) override
		{
			return new cResourcesHolderPrivate();
		}
	}cResourcesHolder_create;
	cResourcesHolder::Create& cResourcesHolder::create = cResourcesHolder_create;
}
