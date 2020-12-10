#pragma once

#include <flame/universe/systems/layout.h>

namespace flame
{
	struct Window;

	struct cElementPrivate;
	struct cLayoutPrivate;

	struct sLayoutBridge : sLayout
	{
		void add_to_sizing_list(cElement* e) override;
		void remove_from_sizing_list(cElement* e) override;
		void add_to_layouting_list(cLayout* l) override;
		void remove_from_layouting_list(cLayout* l) override;
	};

	struct sLayoutPrivate : sLayoutBridge
	{
		Window* window = nullptr;;

		std::deque<cElementPrivate*> sizing_list;
		std::deque<cLayoutPrivate*> layouting_list;

		void add_to_sizing_list(cElementPrivate* e);
		void remove_from_sizing_list(cElementPrivate* e);
		void add_to_layouting_list(cLayoutPrivate* l);
		void remove_from_layouting_list(cLayoutPrivate* l);

		void on_added() override;
		void update() override;
	};

	inline void sLayoutBridge::add_to_sizing_list(cElement* e)
	{
		((sLayoutPrivate*)this)->add_to_sizing_list((cElementPrivate*)e);
	}

	inline void sLayoutBridge::remove_from_sizing_list(cElement* e)
	{
		((sLayoutPrivate*)this)->remove_from_sizing_list((cElementPrivate*)e);
	}

	inline void sLayoutBridge::add_to_layouting_list(cLayout* l)
	{
		((sLayoutPrivate*)this)->add_to_layouting_list((cLayoutPrivate*)l);
	}

	inline void sLayoutBridge::remove_from_layouting_list(cLayout* l)
	{
		((sLayoutPrivate*)this)->remove_from_layouting_list((cLayoutPrivate*)l);
	}
}
