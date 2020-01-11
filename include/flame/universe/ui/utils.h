#pragma once

#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/image.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/toggle.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/splitter.h>
#include <flame/universe/components/list.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/components/window.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/2d_renderer.h>

namespace flame
{
	namespace ui
	{
		FLAME_UNIVERSE_EXPORTS Entity* current_parent();
		FLAME_UNIVERSE_EXPORTS void push_parent(Entity* parent);
		FLAME_UNIVERSE_EXPORTS void pop_parent();
		FLAME_UNIVERSE_EXPORTS Entity* current_entity();
		FLAME_UNIVERSE_EXPORTS void set_current_entity(Entity* e);
	}
}
