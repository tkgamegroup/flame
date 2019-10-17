#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cText;
	struct cEventReceiver;
	struct cStyleColor;
	struct cTree;

	struct cTreeLeaf : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor* style;
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

		FLAME_UNIVERSE_EXPORTS static cTreeLeaf* create();
	};

	struct cTreeNode : Component
	{
		cTree* tree;

		cTreeNode() :
			Component("TreeNode")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cTreeNode* create();
	};

	struct cTreeNodeTitle : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor* style;
		cTree* tree;

		Vec4c unselected_color_normal;
		Vec4c unselected_color_hovering;
		Vec4c unselected_color_active;
		Vec4c selected_color_normal;
		Vec4c selected_color_hovering;
		Vec4c selected_color_active;

		cTreeNodeTitle() :
			Component("TreeNodeTitle")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cTreeNodeTitle* create();
	};

	struct cTreeNodeArrow : Component
	{
		cEventReceiver* event_receiver;
		cText* text;
		cTree* tree;

		cTreeNodeArrow() :
			Component("TreeNodeArrow")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cTreeNodeArrow* create();
	};

	struct cTree : Component
	{
		cEventReceiver* event_receiver;

		Entity* selected;

		cTree() :
			Component("Tree")
		{
		}

		Listeners<void(void* c, Entity * e)> selected_changed_listeners;

		FLAME_UNIVERSE_EXPORTS void set_selected(Entity* e, bool trigger_changed = true);

		FLAME_UNIVERSE_EXPORTS static cTree* create();
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_tree(bool size_fit_parent);
	FLAME_UNIVERSE_EXPORTS Entity* create_standard_tree_node(graphics::FontAtlas* font_atlas, const std::wstring& name);
	FLAME_UNIVERSE_EXPORTS Entity* create_standard_tree_leaf(graphics::FontAtlas* font_atlas, const std::wstring& name);
}
