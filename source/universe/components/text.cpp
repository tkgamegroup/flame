#include <flame/graphics/canvas.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/default_style.h>

namespace flame
{
	struct cTextPrivate : cText
	{
		cElement* element;
		std::wstring text;

		cTextPrivate() :
			element(nullptr)
		{
			font_atlas = nullptr;
			color = default_style.text_color_normal;
			sdf_scale = default_style.sdf_scale;
		}

		void on_attach()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
		}

		void update()
		{
			element->canvas()->add_text(font_atlas, Vec2f(element->global_x.v, element->global_y.v) + 
				Vec2f(element->inner_padding[0], element->inner_padding[1]) * element->global_scale.v, 
				Vec4c(Vec3c(color), color.w() * element->alpha), text.c_str(), sdf_scale * element->global_scale.v);
		}
	};

	cText::~cText()
	{
	}

#define NAME "Text"
	const char* cText::type_name() const
	{
		return NAME;
	}

	uint cText::type_hash() const
	{
		return cH(NAME);
	}
#undef NAME

	void cText::on_attach()
	{
		((cTextPrivate*)this)->on_attach();
	}

	void cText::update()
	{
		((cTextPrivate*)this)->update();
	}

	const std::wstring& cText::text() const
	{
		return ((cTextPrivate*)this)->text;
	}

	void cText::set_text(const std::wstring& text)
	{
		((cTextPrivate*)this)->text = text;
	}

	cText* cText::create()
	{
		return new cTextPrivate();
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
