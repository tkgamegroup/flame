#pragma once

#include <flame/universe/components/layout.h>

namespace flame
{
	struct cElementPrivate;
	struct cAlignerPrivate;

	struct sTypeSettingPrivate;

	struct cLayoutPrivate : cLayout // R ~ on_*
	{
		LayoutType type = LayoutBasic;
		float gap = 0.f;
		bool auto_width = true;
		bool auto_height = true;

		float scrollx = 0.f;
		float scrolly = 0.f;

		cElementPrivate* element = nullptr;
		cAlignerPrivate* aligner = nullptr;
		sTypeSettingPrivate* type_setting = nullptr;

		bool pending_layouting = false;
		bool updating = false;

		LayoutType get_type() const override { return type; }
		void set_type(LayoutType t) override;

		float get_gap() const override { return gap; }
		void set_gap(float g) override;

		bool get_auto_width() const override { return auto_width; }
		void set_auto_width(bool a) override;
		bool get_auto_height() const override { return auto_height; }
		void set_auto_height(bool a) override;

		void apply_basic_h(cElementPrivate* e, cAlignerPrivate* a, bool free);
		void apply_basic_v(cElementPrivate* e, cAlignerPrivate* a, bool free);
		void judge_width(float w);
		void judge_height(float h);
		void update();

		void mark_layout_dirty() override;

		void on_added() override;
		void on_entered_world() override;
		void on_left_world() override;
		void on_entity_component_removed(Component* c) override;
		void on_entity_component_added(Component* c) override;
		void on_entity_component_data_changed(Component* c, uint data_name_hash) override;
		void on_entity_child_visibility_changed(Entity* e) override;
		void on_entity_child_position_changed(Entity* e) override;
		void on_entity_child_component_added(Component* c) override;
		void on_entity_child_component_removed(Component* c) override;
		void on_entity_child_component_data_changed(Component* c, uint data_name_hash) override;
	};
}
