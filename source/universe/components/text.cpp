#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/default_style.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		std::wstring text;

		cTextPrivate(Entity* e) :
			cText(e)
		{
			element = (cElement*)(e->find_component(cH("Element")));
			assert(element);

			font_atlas = nullptr;
			color = default_style.text_color_normal;
			sdf_scale = default_style.sdf_scale;
		}

		void update()
		{
			auto rect = element->canvas->add_text(font_atlas, Vec2f(element->global_x, element->global_y) + 
				Vec2f(element->inner_padding[0], element->inner_padding[1]) * element->global_scale, 
				alpha_mul(color, element->alpha), text.c_str(), sdf_scale * element->global_scale);
			element->width = rect.x();
			element->height = rect.y();
		}
	};

	cText::cText(Entity* e) :
		Component("Text", e)
	{
	}

	cText::~cText()
	{
	}

	const std::wstring& cText::text() const
	{
		return ((cTextPrivate*)this)->text;
	}

	void cText::set_text(const std::wstring& text)
	{
		((cTextPrivate*)this)->text = text;
	}

	void cText::update()
	{
		((cTextPrivate*)this)->update();
	}

	cText* cText::create(Entity* e)
	{
		return new cTextPrivate(e);
	}

	struct cTextA$
	{
		uint font_atlas_index$;
		Vec4c color$;
		float sdf_scale$;

		FLAME_UNIVERSE_EXPORTS static cText* create()
		{
			//font_atlas = nullptr; // TODO
			//color = src->color$;
			//sdf_scale = src->sdf_scale$;
			return nullptr;
		}

		FLAME_UNIVERSE_EXPORTS static void save(cText* t)
		{

		}
	};
}
