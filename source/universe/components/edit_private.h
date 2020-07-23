#pragma once

#include <flame/universe/components/edit.h>
#include "element_private.h"

namespace flame
{
	struct cEditPrivate : cEdit, cElement::Drawer
	{
		void on_added() override;
		void on_removed() override;
		void on_left_world() override;
		void on_entity_visibility_changed() override;

		void draw(graphics::Canvas* canvas) override;

		static cEditPrivate* create();
	};
}
