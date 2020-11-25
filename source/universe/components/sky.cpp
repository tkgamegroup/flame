#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "node_private.h"
#include "sky_private.h"

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

	void cSkyPrivate::on_gain_canvas()
	{
		{
			auto isfile = false;
			auto fn = std::filesystem::path(box_texture_name);
			if (!fn.extension().empty())
			{
				isfile = true;
				if (!fn.is_absolute())
					fn = entity->get_closest_filename().parent_path() / fn;
			}
			box_texture_id = canvas->find_texture_resource(fn.string().c_str());
			if (box_texture_id == -1 && isfile)
			{
				auto t = graphics::Image::create(graphics::Device::get_default(), fn.c_str(), false, graphics::ImageUsageTransferSrc);
				box_texture_id = canvas->set_texture_resource(-1, t->get_view(0), nullptr, fn.string().c_str());
			}
		}
		{
			auto isfile = false;
			auto fn = std::filesystem::path(irr_texture_name);
			if (!fn.extension().empty())
			{
				isfile = true;
				if (!fn.is_absolute())
					fn = entity->get_closest_filename().parent_path() / fn;
			}
			irr_texture_id = canvas->find_texture_resource(fn.string().c_str());
			if (irr_texture_id == -1 && isfile)
			{
				auto t = graphics::Image::create(graphics::Device::get_default(), fn.c_str(), false, graphics::ImageUsageTransferSrc);
				irr_texture_id = canvas->set_texture_resource(-1, t->get_view(0), nullptr, fn.string().c_str());
			}
		}
		{
			auto isfile = false;
			auto fn = std::filesystem::path(rad_texture_name);
			if (!fn.extension().empty())
			{
				isfile = true;
				if (!fn.is_absolute())
					fn = entity->get_closest_filename().parent_path() / fn;
			}
			rad_texture_id = canvas->find_texture_resource(fn.string().c_str());
			if (rad_texture_id == -1 && isfile)
			{
				auto t = graphics::Image::create(graphics::Device::get_default(), fn.c_str(), false, graphics::ImageUsageTransferSrc);
				rad_texture_id = canvas->set_texture_resource(-1, t->get_view(0), nullptr, fn.string().c_str());
			}
		}
	}

	void cSkyPrivate::draw(graphics::Canvas* canvas)
	{
		if (box_texture_id != -1)
			canvas->set_sky(box_texture_id, irr_texture_id, rad_texture_id);
	}

	cSky* cSky::create()
	{
		return f_new<cSkyPrivate>();
	}
}
