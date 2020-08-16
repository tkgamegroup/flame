#include <flame/universe/components/tree.h>

namespace flame
{
	struct cTreePrivate : cTree // R ~ on_*
	{
		Entity* selected = nullptr;

		Entity* get_selected() const override { return selected; }
		void set_selected(Entity* e) override;

		void expand_to_selected() override;
	};

	struct cTreeLeafPrivate : cTreeLeaf // R ~ on_*
	{
	};

	struct cTreeNodePrivate : cTreeNode // R ~ on_*
	{
	};
}
