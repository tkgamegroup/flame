#pragma once

#include <flame/universe/component.h>

namespace flame
{
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

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cTree* create();
	};
}
