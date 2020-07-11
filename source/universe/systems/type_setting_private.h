#include <flame/universe/systems/type_setting.h>

namespace flame
{
	struct EntityPrivate;
	struct cElementPrivate;

	struct sTypeSettingBridge : sTypeSetting
	{
		void add_to_sizing_list(AutoSizer* s, Entity* e) override;
	};

	struct sTypeSettingPrivate : sTypeSettingBridge
	{
		std::deque<std::tuple<AutoSizer*, EntityPrivate*, cElementPrivate*>> sizing_list;

		void add_to_sizing_list(AutoSizer* s, EntityPrivate* e);
		void remove_from_sizing_list(AutoSizer* s) override;

		void update() override;

		static sTypeSettingPrivate* create();
	};
}
