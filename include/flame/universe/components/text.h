#pragma once

#include <flame/math.h>
#include <flame/universe/component.h>

namespace flame
{
	struct cText$ : Component // requires: Element
	{
		int font_atlas_index;
		Vec4c color;
		float sdf_scale;

		FLAME_UNIVERSE_EXPORTS virtual ~cText$() override;

		FLAME_UNIVERSE_EXPORTS virtual const char* type_name() const override;
		FLAME_UNIVERSE_EXPORTS virtual uint type_hash() const override;

		FLAME_UNIVERSE_EXPORTS virtual void on_attach() override;

		FLAME_UNIVERSE_EXPORTS virtual void update(float delta_time) override;

		FLAME_UNIVERSE_EXPORTS const wchar_t* text() const;
		FLAME_UNIVERSE_EXPORTS void set_text(const wchar_t* text);

		FLAME_UNIVERSE_EXPORTS static cText$* create$(void* data);
	};

	struct cTextArchive$
	{
		int font_atlas_index$;
		Vec4c color$;
		float sdf_scale$;
	};
}
