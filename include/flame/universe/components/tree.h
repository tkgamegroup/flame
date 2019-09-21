#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cElement;
	struct cText;
	struct cEventReceiver;
	struct cStyleBackgroundColor;
	struct cTree;

	struct cTreeLeaf : Component
	{
		cStyleBackgroundColor* style;
		cEventReceiver* event_receiver;
		cTree* tree;

		Vec4c unselected_color_normal;
		Vec4c unselected_color_hovering;
		Vec4c unselected_color_active;
		Vec4c selected_color_normal;
		Vec4c selected_color_hovering;
		Vec4c selected_color_active;

		cTreeLeaf() :
			Component("TreeLeaf")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cTreeLeaf* create();
	};

	struct cTreeNode : Component
	{
		cStyleBackgroundColor* title_style;
		cEventReceiver* title_event_receiver;
		cText* arrow_text;
		cEventReceiver* arrow_event_receiver;
		cTree* tree;

		Vec4c unselected_color_normal;
		Vec4c unselected_color_hovering;
		Vec4c unselected_color_active;
		Vec4c selected_color_normal;
		Vec4c selected_color_hovering;
		Vec4c selected_color_active;

		cTreeNode() :
			Component("TreeNode")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cTreeNode* create();
	};

	struct cTree : Component
	{
		Entity* selected;

		cTree() :
			Component("Tree")
		{
		}

		FLAME_UNIVERSE_EXPORTS void* add_selected_changed_listener(void (*listener)(void* c, Entity* e), const Mail<>& capture);

		FLAME_UNIVERSE_EXPORTS void remove_selected_changed_listener(void* ret_by_add);

		FLAME_UNIVERSE_EXPORTS void set_selected(Entity* e, bool trigger_changed = true);

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cTree* create();
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_tree(bool size_fit_parent);
	FLAME_UNIVERSE_EXPORTS Entity* create_standard_tree_node(graphics::FontAtlas* font_atlas, const std::wstring& name);
	FLAME_UNIVERSE_EXPORTS Entity* create_standard_tree_leaf(graphics::FontAtlas* font_atlas, const std::wstring& name);
}
