#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "sky_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cSkyPrivate::set_box_texture(const char* name)
	{
		box_texture_name = name;
	}

	void cSkyPrivate::set_irr_texture(const char* name)
	{
		irr_texture_name = name;
	}

	void cSkyPrivate::set_rad_texture(const char* name)
	{
		rad_texture_name = name;
	}

	void cSkyPrivate::set_lut_texture(const char* name)
	{
		lut_texture_name = name;
	}

	void cSkyPrivate::on_entered_world()
	{
		canvas = entity->world->get_system_t<sRendererPrivate>()->canvas;
		fassert(canvas);

		{
			auto fn = std::filesystem::path(box_texture_name);
			if (!fn.extension().empty())
			{
				if (!fn.is_absolute())
				{
					auto& srcs = entity->srcs;
					fn = srcs[srcs.size() - src_id - 1].parent_path() / fn;
				}
			}
			box_texture = graphics::Image::create(graphics::Device::get_default(), fn.c_str(), true, graphics::ImageUsageNone, true);
		}
		{
			auto fn = std::filesystem::path(irr_texture_name);
			if (!fn.extension().empty())
			{
				if (!fn.is_absolute())
				{
					auto& srcs = entity->srcs;
					fn = srcs[srcs.size() - src_id - 1].parent_path() / fn;
				}
			}
			irr_texture = graphics::Image::create(graphics::Device::get_default(), fn.c_str(), true, graphics::ImageUsageNone, true);
		}
		{
			auto fn = std::filesystem::path(rad_texture_name);
			if (!fn.extension().empty())
			{
				if (!fn.is_absolute())
				{
					auto& srcs = entity->srcs;
					fn = srcs[srcs.size() - src_id - 1].parent_path() / fn;
				}
			}
			rad_texture = graphics::Image::create(graphics::Device::get_default(), fn.c_str(), true, graphics::ImageUsageNone, true);
		}
		{
			auto fn = std::filesystem::path(lut_texture_name);
			if (!fn.extension().empty())
			{
				if (!fn.is_absolute())
				{
					auto& srcs = entity->srcs;
					fn = srcs[srcs.size() - src_id - 1].parent_path() / fn;
				}
			}
			lut_texture = graphics::Image::create(graphics::Device::get_default(), fn.c_str(), true, graphics::ImageUsageNone, true);
		}
		canvas->set_sky(box_texture->get_view(box_texture->get_levels()), irr_texture->get_view(irr_texture->get_levels()),
			rad_texture->get_view(rad_texture->get_levels()), lut_texture->get_view());
	}

	void cSkyPrivate::on_left_world()
	{
		canvas = nullptr;
	}

	cSky* cSky::create(void* parms)
	{
		return f_new<cSkyPrivate>();
	}
}
