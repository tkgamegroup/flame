#include <flame/universe/systems/type_setting.h>

namespace flame
{
	struct Window;

	struct EntityPrivate;
	struct cLayoutPrivate;

	struct sTypeSettingBridge : sTypeSetting
	{
		void add_to_sizing_list(AutoSizer* s, Entity* e) override;
		void add_to_layouting_list(cLayout* l) override;
		void remove_from_layouting_list(cLayout* l) override;
	};

	struct sTypeSettingPrivate : sTypeSettingBridge
	{
		Window* window = nullptr;;

		std::deque<std::pair<AutoSizer*, EntityPrivate*>> sizing_list;
		std::deque<cLayoutPrivate*> layouting_list;

		void add_to_sizing_list(AutoSizer* s, EntityPrivate* e);
		void remove_from_sizing_list(AutoSizer* s) override;
		void add_to_layouting_list(cLayoutPrivate* l);
		void remove_from_layouting_list(cLayoutPrivate* l);

		void on_added() override;
		void update() override;
	};

	inline void sTypeSettingBridge::add_to_sizing_list(sTypeSetting::AutoSizer* s, Entity* e)
	{
		((sTypeSettingPrivate*)this)->add_to_sizing_list(s, (EntityPrivate*)e);
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
