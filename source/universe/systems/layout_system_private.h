#include <flame/universe/systems/layout_system.h>

namespace flame
{
	struct Window;

	struct cElementPrivate;
	struct cLayoutPrivate;

	struct sLayoutSystemBridge : sLayoutSystem
	{
		void add_to_sizing_list(cElement* e) override;
		void remove_from_sizing_list(cElement* e) override;
		void add_to_layouting_list(cLayout* l) override;
		void remove_from_layouting_list(cLayout* l) override;
	};

	struct sLayoutSystemPrivate : sLayoutSystemBridge
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

	inline void sLayoutSystemBridge::add_to_sizing_list(cElement* e)
	{
		((sLayoutSystemPrivate*)this)->add_to_sizing_list((cElementPrivate*)e);
	}

	inline void sLayoutSystemBridge::remove_from_sizing_list(cElement* e)
	{
		((sLayoutSystemPrivate*)this)->remove_from_sizing_list((cElementPrivate*)e);
	}

	inline void sLayoutSystemBridge::add_to_layouting_list(cLayout* l)
	{
		((sLayoutSystemPrivate*)this)->add_to_layouting_list((cLayoutPrivate*)l);
	}

	inline void sLayoutSystemBridge::remove_from_layouting_list(cLayout* l)
	{
		((sLayoutSystemPrivate*)this)->remove_from_layouting_list((cLayoutPrivate*)l);
	}
}
