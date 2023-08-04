#include "blueprint.h"

namespace flame
{
	struct BlueprintNodePrivate : BlueprintNode
	{
	};

	struct BlueprintLinkPrivate : BlueprintLink
	{

	};

	struct BlueprintGroupPrivate : BlueprintGroup
	{

	};

	struct BlueprintPrivate : Blueprint
	{
		BlueprintPrivate();

		BlueprintNodePtr add_node(BlueprintGroupPtr group, const std::string& name,
			const std::vector<BlueprintSlot>& inputs = {}, const std::vector<BlueprintSlot>& outputs = {}, BlueprintNodeFunction function = nullptr) override;
		void remove_node(BlueprintNodePtr node) override;
		BlueprintLinkPtr add_link(BlueprintNodePtr from_node, uint from_slot, BlueprintNodePtr to_node, uint to_slot) override;
		void remove_link(BlueprintLinkPtr link) override;
		BlueprintGroupPtr add_group(const std::string& name) override;
		void remove_group(BlueprintGroupPtr group) override;
		void move_to_group(const std::vector<BlueprintNodePtr>& nodes) override;
		void save() override;
	};

	struct BlueprintInstancePrivate : BlueprintInstance
	{
		struct Node
		{
			BlueprintNodePtr original;
			std::vector<BlueprintArgument> inputs;
			std::vector<BlueprintArgument> outputs;
		};

		struct Group
		{
			std::map<const void*, BlueprintArgument> datas;
			std::vector<std::vector<Node>> nodes;

			~Group();
		};

		std::map<uint, Group> groups;
		Group* current_group = nullptr;
		int current_stack = -1;

		uint updated_frame;

		BlueprintInstancePrivate();

		void update();

		void set_group(uint group_name) override;
		void run() override;
		void step() override;
	};

	void init_blueprint();
}
