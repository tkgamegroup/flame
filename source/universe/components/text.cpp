#include <flame/graphics/canvas.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/default_style.h>

namespace flame
{
	struct cText$Private : cText$
	{
		cElement$* element;
		std::wstring text;

		cText$Private(void* data) :
			element(nullptr)
		{
			if (!data)
			{
				font_atlas_index = -1;
				color = default_style.text_color_normal;
				sdf_scale = default_style.sdf_scale;
			}
			else
			{
				auto src = (cTextArchive$*)data;
				font_atlas_index = src->font_atlas_index$;
				color = src->color$;
				sdf_scale = src->sdf_scale$;
			}
		}

		void on_attach()
		{
			element = (cElement$*)(entity->component(cH("Element")));
			assert(element);
		}

		void update(float delta_time)
		{
			element->canvas()->add_text(font_atlas_index, Vec2f(element->global_x, element->global_y) + 
				Vec2f(element->inner_padding[0], element->inner_padding[1]) * element->global_scale, 
				Vec4c(color, element->alpha), text.c_str(), sdf_scale * element->global_scale);
		}
	};

	cText$::~cText$()
	{
	}

	const char* cText$::type_name() const
	{
		return "Text";
	}

	uint cText$::type_hash() const
	{
		return cH("Text");
	}

	void cText$::on_attach()
	{
		((cText$Private*)this)->on_attach();
	}

	void cText$::update(float delta_time)
	{
		((cText$Private*)this)->update(delta_time);
	}

	const std::wstring& cText$::text() const
	{
		return ((cText$Private*)this)->text;
	}

	void cText$::set_text(const std::wstring& text)
	{
		((cText$Private*)this)->text = text;
	}

	cText$* cText$::create$(void* data)
	{
		return new cText$Private(data);
	}
}
