#include "../../graphics/image.h"
#include "../../graphics/canvas.h"
#include "element_private.h"
#include "image_private.h"

namespace flame
{
	cImagePrivate::~cImagePrivate()
	{
		element->drawers.remove("image"_h);

		if (!image_name.empty() && !image_name.native().starts_with(L"0x"))
		{
			AssetManagemant::release(Path::get(image_name));
			if (image)
				graphics::Image::release(image);
		}
	}

	void cImagePrivate::on_init()
	{
		element->drawers.add([this](graphics::CanvasPtr canvas) {
			if (image)
			{
				if (angle == 0.f)
					canvas->draw_image(image->get_view(), element->global_pos0(), element->global_pos1(), vec4(0.f, 0.f, 1.f, 1.f), tint_col);
				else
					canvas->draw_image_rotated(image->get_view(), element->global_pos0(), element->global_pos1(), vec4(0.f, 0.f, 1.f, 1.f), tint_col, angle);
			}
		}, "image"_h);
	}

	void cImagePrivate::set_auto_size(bool v)
	{
		if (auto_size == v)
			return;
		auto_size = v;
		if (auto_size && image)
			element->set_ext(image->extent);
		data_changed("auto_size"_h);
	}

	void cImagePrivate::set_image_name(const std::filesystem::path& name)
	{
		if (image_name == name)
			return;

		auto old_one = image;
		auto old_raw = !image_name.empty() && image_name.native().starts_with(L"0x");
		if (!image_name.empty())
		{
			if (!old_raw)
				AssetManagemant::release(Path::get(image_name));
		}
		image_name = name;
		image = nullptr;
		if (!image_name.empty())
		{
			if (!image_name.native().starts_with(L"0x"))
			{
				AssetManagemant::get(Path::get(image_name));
				image = !image_name.empty() ? graphics::Image::get(image_name) : nullptr;
			}
			else
				image = (graphics::ImagePtr)s2u_hex<uint64>(image_name.string());
		}

		if (image != old_one)
		{
			element->mark_drawing_dirty();
			if (auto_size && image)
				element->set_ext(image->extent);
		}

		if (!old_raw && old_one)
			graphics::Image::release(old_one);
		data_changed("image_name"_h);
	}

	void cImagePrivate::set_tint_col(const cvec4& col)
	{
		if (tint_col == col)
			return;
		tint_col = col;
		element->mark_drawing_dirty();
		data_changed("tint_col"_h);
	}

	void cImagePrivate::set_angle(float v)
	{
		if (angle == v)
			return;
		angle = v;
		element->mark_drawing_dirty();
		data_changed("angle"_h);
	}

	struct cImageCreate : cImage::Create
	{
		cImagePtr operator()(EntityPtr) override
		{
			return new cImagePrivate();
		}
	}cImage_create;
	cImage::Create& cImage::create = cImage_create;
}
