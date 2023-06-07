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
				auto b = border * image_scale;
				if (p1.x - p0.x > b.x + b.z && p1.y - p0.y > b.y + b.w)
				{
					auto img_ext = vec2(image->extent) * image_scale;
					auto iv = image->get_view();
					// top border
					canvas->add_image(iv, vec2(p0.x + b.x, p0.y),		vec2(p1.x - b.z, p0.y + b.y),	vec4(b.x / img_ext.x, 0.f, 1.f - b.z / img_ext.x, b.y / img_ext.y));
					// bottom border
					canvas->add_image(iv, vec2(p0.x + b.x, p1.y - b.w),	vec2(p1.x - b.z, p1.y),			vec4(b.x / img_ext.x, 1.f - b.w / img_ext.y, 1.f - b.w / img_ext.x, 1.f));
					// left border
					canvas->add_image(iv, vec2(p0.x, p0.y + b.y),		vec2(p0.x + b.x, p1.y - b.w),	vec4(0.f, b.y / img_ext.y, b.x / img_ext.x, 1.f - b.w / img_ext.y));
					// right border
					canvas->add_image(iv, vec2(p1.x - b.z, p0.y + b.y), vec2(p1.x, p1.y - b.w),			vec4(1.f - b.w / img_ext.x, b.y / img_ext.y, 1.f, 1.f - b.w / img_ext.y));
					// left-top corner
					canvas->add_image(iv, vec2(p0.x, p0.y),				vec2(p0.x + b.x, p0.y + b.y),	vec4(0.f, 0.f, b.x / img_ext.x, b.y / img_ext.y));
					// right-top corner
					canvas->add_image(iv, vec2(p1.x - b.z, p0.y),		vec2(p1.x, p0.y + b.y),			vec4(1.f - b.w / img_ext.x, 0.f, 1.f, b.y / img_ext.y));
					// left-bottom corner
					canvas->add_image(iv, vec2(p0.x, p1.y - b.w),		vec2(p0.x + b.x, p1.y),			vec4(0.f, 1.f - b.w / img_ext.y, b.x / img_ext.x, 1.f));
					// right-bottom corner
					canvas->add_image(iv, vec2(p1.x - b.z, p1.y - b.w), vec2(p1.x, p1.y),				vec4(1.f - b.w / img_ext.x, 1.f - b.w / img_ext.y, 1.f, 1.f));
					// middle
					canvas->add_image(iv, vec2(p0.x + b.x, p0.y + b.y), vec2(p1.x - b.z, p1.y - b.w),	vec4(vec2(b.x, b.y) / img_ext, 1.f - vec2(b.z, b.w) / img_ext));
				}
			}
		}, "stretched_image"_h);
	}

	void cStretchedImagePrivate::set_image_name(const std::filesystem::path& name)
	{
		if (image_name == name)
			return;

		auto old_one = image;
		if (!image_name.empty())
		{
			if (!image_name.native().starts_with(L"0x"))
				AssetManagemant::release(Path::get(image_name));
			else
				old_one = nullptr;
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

		if (old_one)
			graphics::Image::release(old_one);
		data_changed("image_name"_h);
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
