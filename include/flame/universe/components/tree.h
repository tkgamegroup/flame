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
	struct cStyleColor2;
	struct cTree;

	struct cTreeLeaf : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor2* style;
		cTree* tree;

		cTreeLeaf() :
			Component("cTreeLeaf")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cTreeLeaf* create();
	};

	struct cTreeNode : Component
	{
		cTree* tree;

		cTreeNode() :
			Component("cTreeNode")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cTreeNode* create();
	};

	struct cTreeNodeTitle : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor2* style;
		cTree* tree;

		cTreeNodeTitle() :
			Component("cTreeNodeTitle")
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
			Component("cTreeNodeArrow")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cTreeNodeArrow* create();
	};

	struct cTree : Component
	{
		cEventReceiver* event_receiver;

		Entity* selected;

		cTree() :
			Component("cTree")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_selected(Entity* e, bool trigger_changed = true);

		FLAME_UNIVERSE_EXPORTS static cTree* create();
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_tree(bool size_fit_parent);
	FLAME_UNIVERSE_EXPORTS Entity* create_standard_tree_node(graphics::FontAtlas* font_atlas, const wchar_t* name);
	FLAME_UNIVERSE_EXPORTS Entity* create_standard_tree_leaf(graphics::FontAtlas* font_atlas, const wchar_t* name);
}
