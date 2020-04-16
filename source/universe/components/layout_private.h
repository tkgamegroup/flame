#pragma once

#include <flame/universe/components/layout.h>

namespace flame
{
	struct cText;

	struct cLayoutPrivate : cLayout
	{
		sLayoutManagement* management;
		bool pending_update;

		void* element_data_listener;
		
		struct Aligner
		{
			cElement* element;
			cAligner* aligner;
			void* element_listener;
			void* aligner_listener;
		};
		std::vector<Aligner> als;
		bool als_dirty;

		cLayoutPrivate(LayoutType _type);
		~cLayoutPrivate();
		void set_content_size(const Vec2f& s);
		void apply_h_free_layout(const Aligner& a, bool free);
		void apply_v_free_layout(const Aligner& a, bool free);
		void use_children_width(float w);
		void use_children_height(float h);
		void on_entered_world() override;
		void on_left_world() override;
		void on_component_added(Component* c) override;
		void on_visibility_changed() override;
		void on_child_visibility_changed() override;
		void on_child_component_added(Component* c) override;
		void on_child_component_removed(Component* c) override;
		void on_child_position_changed() override;
		void update();
	};
}
