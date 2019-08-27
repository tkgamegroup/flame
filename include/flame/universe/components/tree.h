#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cText;
	struct cEventReceiver;
	struct cStyleBgCol;
	struct cTree;

	struct cTreeNode : Component
	{
		cText* text;
		cStyleBgCol* style;
		cTree* tree;

		Vec4c unselected_color_normal;
		Vec4c unselected_color_hovering_or_active;
		Vec4c selected_color_normal;
		Vec4c selected_color_hovering_or_active;

		cTreeNode() :
			Component("TreeNode")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cTreeNode() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_added() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cTreeNode* create();
	};

	struct cTree : Component
	{
		cText* text;

		Entity* select;

		cTree() :
			Component("Tree")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cTree() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_added() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cTree* create();
	};
}
