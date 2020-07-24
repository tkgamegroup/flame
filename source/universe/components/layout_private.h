#pragma once

#include <flame/universe/components/layout.h>

namespace flame
{
	struct cElementPrivate;
	struct cAlignerPrivate;

	struct sTypeSettingPrivate;

	struct cLayoutPrivate : cLayout
	{
		LayoutType type = LayoutFree;
		bool pending_layouting = false;
		bool updating = false;

		cElementPrivate* element = nullptr;
		cAlignerPrivate* aligner = nullptr;
		sTypeSettingPrivate* type_setting = nullptr;

	//	cLayoutPrivate(LayoutType _type);
	//	~cLayoutPrivate();
	//	void set_content_size(const Vec2f& s);
		void apply_h_free_layout(cElementPrivate* e, cAlignerPrivate* a, bool free);
		void apply_v_free_layout(cElementPrivate* e, cAlignerPrivate* a, bool free);
	//	void use_children_width(float w);
	//	void use_children_height(float h);
	//	void on_event(EntityEvent e, void* t) override;
	//	void on_sibling_data_changed(Component* t, uint hash, void* sender) override;
	//	void on_child_data_changed(Component* t, uint hash, void* sender) override;
		void update();

		LayoutType get_type() const override { return type; }
		void set_type(LayoutType t) override;

		void mark_dirty() override;

		void on_added() override;
		void on_entered_world() override;
		void on_left_world() override;
	};
}
