#include "blueprint.h"

namespace flame
{
	struct BlueprintSlotPrivate : BlueprintSlot
	{
		std::vector<BlueprintSlotPtr> linked_slots;
		
		~BlueprintSlotPrivate();
	};

	struct BlueprintNodePrivate : BlueprintNode
	{

	};

	struct BlueprintLinkPrivate : BlueprintLink
	{

	};

	struct BlueprintBlockPrivate : BlueprintBlock
	{

	};

	struct BlueprintGroupPrivate : BlueprintGroup
	{
		~BlueprintGroupPrivate();
	};

	struct BlueprintPrivate : Blueprint
	{
		uint next_object_id = 1;

		BlueprintPrivate();
		~BlueprintPrivate();

		void*					add_variable(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) override;
		void					remove_variable(BlueprintGroupPtr group, uint name) override;
		void					alter_variable(BlueprintGroupPtr group, uint old_name, const std::string& new_name, TypeInfo* type) override;
		BlueprintNodePtr		add_node(BlueprintGroupPtr group, BlueprintBlockPtr block, const std::string& name, const std::string& display_name,
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			BlueprintNodeFunction function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr) override;
		BlueprintNodePtr		add_variable_node(BlueprintGroupPtr group, BlueprintBlockPtr block, uint variable_name, uint type) override;
		BlueprintNodePtr		add_call_node(BlueprintGroupPtr group, BlueprintBlockPtr block, uint group_name) override;
		void					remove_node(BlueprintNodePtr node) override;
		void					set_node_block(BlueprintNodePtr node, BlueprintBlockPtr new_block) override;
		void					set_input_type(BlueprintSlotPtr slot, TypeInfo* type) override;
		BlueprintLinkPtr		add_link(BlueprintSlotPtr from_slot, BlueprintSlotPtr to_slot) override;
		void					remove_link(BlueprintLinkPtr link) override;
		BlueprintBlockPtr		add_block(BlueprintGroupPtr group, BlueprintBlockPtr parent) override;
		void					remove_block(BlueprintBlockPtr block) override;
		void					set_block_parent(BlueprintBlockPtr block, BlueprintBlockPtr new_parent) override;
		BlueprintGroupPtr		add_group(const std::string& name) override;
		void					remove_group(BlueprintGroupPtr group) override;
		void					add_group_input(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) override;
		void					remove_group_input(BlueprintGroupPtr group, uint name) override;
		void					add_group_output(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) override;
		void					remove_group_output(BlueprintGroupPtr group, uint name) override;
		void					save(const std::filesystem::path& path) override;
	};

	struct BlueprintNodeLibraryPrivate : BlueprintNodeLibrary
	{
		void add_template(const std::string& name, const std::string& display_name, 
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			BlueprintNodeFunction function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeInputSlotChangedCallback input_slot_changed_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr) override;
	};

	struct BlueprintInstancePrivate : BlueprintInstance
	{
		BlueprintInstancePrivate(BlueprintPtr blueprint);
		~BlueprintInstancePrivate();

		void build() override;
		void prepare_executing(Group* group) override;
		void run(Group* group) override;
		void step(Group* group) override;
		void stop(Group* group) override;
		void call(uint group_name, void** inputs, void** outputs) override;
	};

	struct BlueprintDebuggerPrivate : BlueprintDebugger
	{
		void add_break_node(BlueprintNodePtr node) override;
		void remove_break_node(BlueprintNodePtr node) override;
	};
}
