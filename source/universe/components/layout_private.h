#pragma once

#include <flame/universe/components/layout.h>

namespace flame
{
	struct cText;

	struct cLayoutPrivate : cLayout
	{
		sLayoutManagement* management;
		bool pending_update;

		bool als_dirty;
		std::vector<std::tuple<cElement*, cAligner*, cText*>> als;

		cLayoutPrivate(LayoutType _type);
		void apply_h_free_layout(cElement* element, cAligner* aligner, bool lock);
		void apply_v_free_layout(cElement* element, cAligner* aligner, bool lock);
		void use_children_width(float w);
		void use_children_height(float h);
		void on_into_world() override;
		void on_component_added(Component* c) override;
		void on_child_visibility_changed() override;
		void on_child_component_added(Component* c) override;
		void on_child_component_removed(Component* c) override;
		void update();
		Component* copy() override;
	};
}
