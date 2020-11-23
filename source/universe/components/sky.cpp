#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "node_private.h"
#include "sky_private.h"

namespace flame
{
	void cSkyPrivate::set_texture_name(const char* name)
	{
		texture_name = name;
	}

	void cSkyPrivate::on_gain_canvas()
	{
		{
			auto isfile = false;
			auto fn = std::filesystem::path(texture_name);
			if (!fn.extension().empty())
			{
				isfile = true;
				if (!fn.is_absolute())
					fn = entity->get_closest_filename().parent_path() / fn;
			}
			texture_id = canvas->find_texture_resource(fn.string().c_str());
			if (texture_id == -1 && isfile)
			{
				auto t = graphics::Image::create(graphics::Device::get_default(), fn.c_str(), false, graphics::ImageUsageTransferSrc);
				texture_id = canvas->set_texture_resource(-1, t->get_view(0), nullptr, fn.string().c_str());
			}
		}
	}

	void cSkyPrivate::draw(graphics::Canvas* canvas)
	{
		if (texture_id != -1)
			canvas->set_sky(texture_id);
	}

	cSky* cSky::create()
	{
		return f_new<cSkyPrivate>();
	}
}
