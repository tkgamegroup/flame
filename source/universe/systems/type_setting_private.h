#include <flame/universe/systems/type_setting.h>

namespace flame
{
	struct Window;

	struct cElementPrivate;
	struct cLayoutPrivate;

	struct sTypeSettingBridge : sTypeSetting
	{
		void add_to_sizing_list(cElement* e) override;
		void remove_from_sizing_list(cElement* e) override;
		void add_to_layouting_list(cLayout* l) override;
		void remove_from_layouting_list(cLayout* l) override;
	};

	struct sTypeSettingPrivate : sTypeSettingBridge
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

	inline void sTypeSettingBridge::add_to_sizing_list(cElement* e)
	{
		((sTypeSettingPrivate*)this)->add_to_sizing_list((cElementPrivate*)e);
	}

	inline void sTypeSettingBridge::remove_from_sizing_list(cElement* e)
	{
		((sTypeSettingPrivate*)this)->remove_from_sizing_list((cElementPrivate*)e);
	}

	inline void sTypeSettingBridge::add_to_layouting_list(cLayout* l)
	{
		((sTypeSettingPrivate*)this)->add_to_layouting_list((cLayoutPrivate*)l);
	}

	inline void sTypeSettingBridge::remove_from_layouting_list(cLayout* l)
	{
		((sTypeSettingPrivate*)this)->remove_from_layouting_list((cLayoutPrivate*)l);
	}
}
