#pragma once

#include "../component.h"
namespace flame
{
	// Reflect ctor
	struct cScrollView : Component
	{
		// Reflect requires
		cElementPtr element = nullptr;

		// Reflect
		GUID content_container;
		// Reflect
		virtual void set_content_container(const GUID& guid) = 0;
		// Reflect
		GUID content_viewport;
		// Reflect
		virtual void set_content_viewport(const GUID& guid) = 0;
		// Reflect
		GUID vertical_scroller;
		// Reflect
		virtual void set_vertical_scroller(const GUID& guid) = 0;
		// Reflect
		GUID vertical_scroller_slider;
		// Reflect
		virtual void set_vertical_scroller_slider(const GUID& guid) = 0;

		EntityPtr e_content_container = nullptr;
		EntityPtr e_content_viewport = nullptr;
		EntityPtr e_vertical_scroller = nullptr;
		EntityPtr e_vertical_scroller_slider = nullptr;

		struct Create
		{
			virtual cScrollViewPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
