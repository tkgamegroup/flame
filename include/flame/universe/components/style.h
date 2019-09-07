#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cText;
	struct cEventReceiver;

	struct cStyleBackgroundColor : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		Vec4c color_normal;
		Vec4c color_hovering;
		Vec4c color_active;

		cStyleBackgroundColor() :
			Component("StyleBackgroundColor")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cStyleBackgroundColor* create(const Vec4c& color_normal = Vec4c(0), const Vec4c& color_hovering = Vec4c(0), const Vec4c& color_active = Vec4c(0));
	};

	struct cStyleTextColor : Component
	{
		cText* text;
		cEventReceiver* event_receiver;

		Vec4c color_normal;
		Vec4c color_else;

		cStyleTextColor() :
			Component("StyleTextColor")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cStyleTextColor* create(const Vec4c& color_normal = Vec4c(0), const Vec4c& color_else = Vec4c(0));
	};
}
