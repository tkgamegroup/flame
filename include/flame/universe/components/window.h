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
			Component("Moveable")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cMoveable* create();
	};

	struct cBringToFront : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		cBringToFront() :
			Component("BringToFront")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cBringToFront* create();
	};

	struct cSizeDragger : Component 
	{
		cEventReceiver* event_receiver;
		cElement* p_element;

		cSizeDragger() :
			Component("SizeDragger")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

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
			Component("DockerTab")
		{

		}

		FLAME_UNIVERSE_EXPORTS void take_away(bool close);

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cDockerTab* create();
	};

	struct cDockerTabbar : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;
		cList* list;

		cDockerTabbar() :
			Component("DockerTabbar")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cDockerTabbar* create();
	};

	struct cDockerPages : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;

		cDockerPages() :
			Component("DockerPages")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cDockerPages* create();
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_docker_tab(graphics::FontAtlas* font_atlas, const std::wstring& title, Entity* root);
	FLAME_UNIVERSE_EXPORTS Entity* get_docker_page_model();
	FLAME_UNIVERSE_EXPORTS Entity* get_docker_model();
	FLAME_UNIVERSE_EXPORTS Entity* get_docker_layout_model();
	FLAME_UNIVERSE_EXPORTS Entity* get_docker_container_model();
}
