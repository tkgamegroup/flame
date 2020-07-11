#include <flame/universe/systems/type_setting.h>

namespace flame
{
	struct EntityPrivate;
	struct cElementPrivate;
	struct cLayoutPrivate;

	struct sTypeSettingBridge : sTypeSetting
	{
		void add_to_sizing_list(AutoSizer* s, Entity* e) override;
		void add_to_layouting_list(cLayout* l) override;
		void remove_from_layouting_list(cLayout* l) override;
	};

	struct sTypeSettingPrivate : sTypeSettingBridge
	{
		std::deque<std::tuple<AutoSizer*, EntityPrivate*, cElementPrivate*>> sizing_list;
		std::deque<cLayoutPrivate*> layouting_list;

		void add_to_sizing_list(AutoSizer* s, EntityPrivate* e);
		void remove_from_sizing_list(AutoSizer* s) override;
		void add_to_layouting_list(cLayoutPrivate* l);
		void remove_from_layouting_list(cLayoutPrivate* l);

		void update() override;

		static sTypeSettingPrivate* create();
	};
}
