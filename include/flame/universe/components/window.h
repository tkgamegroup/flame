#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cElement;
	struct cEventReceiver;
	struct cAligner;
	struct cLayout;
	struct cListItem;
	struct cList;

	struct cMoveable : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		cMoveable() :
			Component("cMoveable")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cMoveable* create();
	};

	struct cBringToFront : Component
	{
		cEventReceiver* event_receiver;

		cBringToFront() :
			Component("cBringToFront")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cBringToFront* create();
	};

	struct cSizeDragger : Component 
	{
		cEventReceiver* event_receiver;
		cElement* p_element;

		cSizeDragger() :
			Component("cSizeDragger")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cSizeDragger* create();
	};

	struct cDockerTab : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;
		cListItem* list_item;

		Entity* root;

		bool floating;
		Entity* page;
		cElement* page_element;

		cDockerTab() :
			Component("cDockerTab")
		{

		}

		FLAME_UNIVERSE_EXPORTS void take_away(bool close);

		FLAME_UNIVERSE_EXPORTS static cDockerTab* create();
	};

	struct cDockerTabbar : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;
		cList* list;

		cDockerTabbar() :
			Component("cDockerTabbar")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cDockerTabbar* create();
	};

	struct cDockerPages : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		cDockerPages() :
			Component("cDockerPages")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cDockerPages* create();
	};

	struct cDockerStaticContainer : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		cDockerStaticContainer() :
			Component("cDockerStaticContainer")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cDockerStaticContainer* create();
	};
}
