#pragma once

#include <flame/universe/components/layout.h>

namespace flame
{
	struct cLayoutPrivate : cLayout
	{
		bool pending_update;

		cLayoutPrivate(LayoutType _type);
		~cLayoutPrivate();
		void set_content_size(const Vec2f& s);
		void apply_h_free_layout(cElement* e, cAligner* a, bool free);
		void apply_v_free_layout(cElement* e, cAligner* a, bool free);
		void use_children_width(float w);
		void use_children_height(float h);
		void on_event(EntityEvent e, void* t) override;
		void on_sibling_data_changed(Component* t, uint hash, void* sender) override;
		void on_child_data_changed(Component* t, uint hash, void* sender) override;
		void update();
	};
}
