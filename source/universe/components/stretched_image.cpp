#include "../../graphics/image.h"
#include "../../graphics/canvas.h"
#include "element_private.h"
#include "stretched_image_private.h"

namespace flame
{
	cStretchedImagePrivate::~cStretchedImagePrivate()
	{
		element->drawers.remove("stretched_image"_h);

		if (!image_name.empty() && !image_name.native().starts_with(L"0x"))
		{
			AssetManagemant::release(Path::get(image_name));
			if (image)
				graphics::Image::release(image);
		}
	}

	void cStretchedImagePrivate::on_init()
	{
		element->drawers.add([this](graphics::CanvasPtr canvas) {
			if (image)
			{
				auto p0 = element->global_pos0();
				auto p1 = element->global_pos1();
				canvas->add_image_stretched(image->get_view(), p0, p1, vec4(0.f, 0.f, 1.f, 1.f), border, image_scale, tint_col);
			}
		}, "stretched_image"_h);
	}

	void cStretchedImagePrivate::set_image_name(const std::filesystem::path& name)
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
			element->mark_drawing_dirty();

		if (!old_raw && old_one)
			graphics::Image::release(old_one);
		data_changed("image_name"_h);
	}

	void cStretchedImagePrivate::set_tint_col(const cvec4& col)
	{
		if (tint_col == col)
			return;
		tint_col = col;
		element->mark_drawing_dirty();
		data_changed("tint_col"_h);
	}

	void cStretchedImagePrivate::set_border(const vec4& _border)
	{
		if (border == _border)
			return;
		border = _border;
		element->mark_drawing_dirty();
		data_changed("border"_h);
	}

	void cStretchedImagePrivate::set_image_scale(float scale)
	{
		if (image_scale == scale)
			return;
		image_scale = scale;
		element->mark_drawing_dirty();
		data_changed("image_scale"_h);
	}

	struct cStretchedImageCreate : cStretchedImage::Create
	{
		cStretchedImagePtr operator()(EntityPtr) override
		{
			return new cStretchedImagePrivate();
		}
	}cStretchedImage_create;
	cStretchedImage::Create& cStretchedImage::create = cStretchedImage_create;
}
