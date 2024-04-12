#include "blueprint.h"

namespace flame
{
	struct BlueprintSlotPrivate : BlueprintSlot
	{
		std::vector<BlueprintSlotPtr> linked_slots;
		
		~BlueprintSlotPrivate();
		uint get_linked_count() const override;
		BlueprintSlotPtr get_linked(uint idx) const override;
	};

	struct BlueprintNodePrivate : BlueprintNode
	{

	};

	struct BlueprintLinkPrivate : BlueprintLink
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

		void					set_super(const std::filesystem::path& filename) override;

		BlueprintEnum*			add_enum(const std::string& name, const std::vector<BlueprintEnumItem>& items) override;
		void					remove_enum(uint name) override;
		void					alter_enum(uint old_name, const std::string& new_name, const std::vector<BlueprintEnumItem>& new_items) override;
		BlueprintStruct*		add_struct(const std::string& name, const std::vector<BlueprintStructVariable>& variables) override;
		void					remove_struct(uint name) override;
		void					alter_struct(uint old_name, const std::string& new_name, const std::vector<BlueprintStructVariable>& new_variables) override;

		BlueprintVariable*		add_variable(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) override;
		void					remove_variable(BlueprintGroupPtr group, uint name) override;
		void					alter_variable(BlueprintGroupPtr group, uint old_name, const std::string& new_name, TypeInfo* new_type) override;

		BlueprintSlotPtr		create_slot(BlueprintNodePtr n, const BlueprintSlotDesc& desc, int pos = -1);

		BlueprintNodePtr		add_node(BlueprintGroupPtr group, BlueprintNodePtr parent, const std::string& name, BlueprintNodeFlags flags, const std::string& display_name,
			const std::vector<BlueprintSlotDesc>& inputs, const std::vector<BlueprintSlotDesc>& outputs,
			BlueprintNodeFunction function, BlueprintNodeLoopFunction loop_function,
			BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
			BlueprintNodeChangeStructureCallback change_structure_callback, BlueprintNodePreviewProvider preview_provider, 
			bool is_block = false, BlueprintNodeBeginBlockFunction begin_block_function = nullptr, BlueprintNodeEndBlockFunction end_block_function = nullptr) override;
		BlueprintNodePtr		add_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint name_hash) override;
		BlueprintNodePtr		add_block(BlueprintGroupPtr group, BlueprintNodePtr parent) override;
		BlueprintNodePtr		add_variable_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint variable_name, uint type, uint location_name, uint property_name) override;
		BlueprintNodePtr		add_call_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint group_name, uint location_name) override;
		void					remove_node(BlueprintNodePtr node, bool recursively) override;
		void					set_nodes_parent(const std::vector<BlueprintNodePtr> nodes, BlueprintNodePtr new_parent) override;
		bool					change_node_structure(BlueprintNodePtr node, const std::string& new_template_string, const std::vector<TypeInfo*>& new_input_types) override;
		bool					change_references(BlueprintGroupPtr group, uint old_name, uint old_location, uint old_property, uint new_name, uint new_location, uint new_property) override;
		BlueprintLinkPtr		add_link(BlueprintSlotPtr from_slot, BlueprintSlotPtr to_slot) override;
		void					remove_link(BlueprintLinkPtr link) override;
		BlueprintGroupPtr		add_group(const std::string& name) override;
		void					remove_group(BlueprintGroupPtr group) override;
		void					alter_group(uint old_name, const std::string& new_name) override;
		void					add_group_input(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) override;
		void					remove_group_input(BlueprintGroupPtr group, uint name) override;
		void					alter_group_input(BlueprintGroupPtr group, uint old_name, const std::string& new_name, TypeInfo* new_type) override;
		void					add_group_output(BlueprintGroupPtr group, const std::string& name, TypeInfo* type) override;
		void					remove_group_output(BlueprintGroupPtr group, uint name) override;
		void					alter_group_output(BlueprintGroupPtr group, uint old_name, const std::string& new_name, TypeInfo* new_type) override;

		void					load(const std::filesystem::path& path, bool load_typeinfos) override;
		void					save(const std::filesystem::path& path) override;
	};

	struct BlueprintNodeLibraryPrivate : BlueprintNodeLibrary
	{
		void add_template(const std::string& name, const std::string& display_name, BlueprintNodeFlags flags = BlueprintNodeFlagNone,
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			BlueprintNodeFunction function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeChangeStructureCallback change_structure_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr) override;
		void add_template(const std::string& name, const std::string& display_name, BlueprintNodeFlags flags = BlueprintNodeFlagNone,
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			BlueprintNodeLoopFunction loop_function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeChangeStructureCallback change_structure_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr) override;
		void add_template(const std::string& name, const std::string& display_name, BlueprintNodeFlags flags = BlueprintNodeFlagNone,
			const std::vector<BlueprintSlotDesc>& inputs = {}, const std::vector<BlueprintSlotDesc>& outputs = {},
			bool is_block = true, BlueprintNodeBeginBlockFunction begin_block_function = nullptr, BlueprintNodeEndBlockFunction end_block_function = nullptr, 
			BlueprintNodeLoopFunction loop_function = nullptr, BlueprintNodeConstructor constructor = nullptr, BlueprintNodeDestructor destructor = nullptr,
			BlueprintNodeChangeStructureCallback change_structure_callback = nullptr, BlueprintNodePreviewProvider preview_provider = nullptr) override;
	};

	struct BlueprintInstancePrivate : BlueprintInstance
	{
		BlueprintInstancePrivate(BlueprintPtr blueprint);
		~BlueprintInstancePrivate(); 

		void build() override;
		void prepare_executing(BlueprintInstanceGroup* group) override;
		void run(BlueprintInstanceGroup* group) override;
		BlueprintInstanceNode* step(BlueprintInstanceGroup* group) override;
		void stop(BlueprintInstanceGroup* group) override;
		void call(BlueprintInstanceGroup* group, void** inputs, void** outputs) override;
		void call(BlueprintInstanceGroup* group, const std::vector<std::pair<uint, void*>>& named_inputs, const std::vector<std::pair<uint, void*>>& named_outputs) override;
	};

	extern std::map<uint, std::vector<BlueprintInstanceGroup*>> message_receivers;

	struct BlueprintDebuggerPrivate : BlueprintDebugger
	{
		void add_break_node(BlueprintNodePtr node, BlueprintBreakpointOption option) override;
		void remove_break_node(BlueprintNodePtr node) override;
	};
}
