#include <flame/graphics/canvas.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/default_style.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		cElement* element;
		std::wstring text;

		cTextPrivate(Entity* e) :
			cText(e),
			element(nullptr)
		{
			font_atlas = nullptr;
			color = default_style.text_color_normal;
			sdf_scale = default_style.sdf_scale;

			element = (cElement*)(e->find_component(cH("Element")));
			assert(element);
		}

		void update()
		{
			element->canvas->add_text(font_atlas, Vec2f(element->global_x, element->global_y) + 
				Vec2f(element->inner_padding[0], element->inner_padding[1]) * element->global_scale, 
				Vec4c(Vec3c(color), color.w() * element->alpha), text.c_str(), sdf_scale * element->global_scale);
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
