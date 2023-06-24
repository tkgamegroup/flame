#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cInputField : Component
	{
		// Reflect requires
		cReceiverPtr receiver = nullptr;

		// Reflect
		GUID text_component;
		// Reflect
		virtual void set_text_component(const GUID& guid) = 0;

		struct Create
		{
			virtual cInputFieldPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
