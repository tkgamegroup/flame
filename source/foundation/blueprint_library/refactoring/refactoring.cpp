#include "../../blueprint_private.h"

namespace flame
{
	void add_refactoring_node_templates(BlueprintNodeLibraryPtr library)
	{
		library->add_template("RF: Find Node", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Group",
					.allowed_types = { TypeInfo::get<BlueprintGroupPtr>() }
				},
				{
					.name = "ID",
					.allowed_types = { TypeInfo::get<uint>() }
				}
			},
			{
				{
					.name = "V",
					.allowed_types = { TypeInfo::get<BlueprintNodePtr>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto g = *(BlueprintGroupPtr*)inputs[0].data;
				if (g)
				{
					auto id = *(uint*)inputs[1].data;
					if (id)
						*(BlueprintNodePtr*)outputs[0].data = g->find_node_by_id(id);
					else
						*(BlueprintNodePtr*)outputs[0].data = nullptr;
				}
				else
					*(BlueprintNodePtr*)outputs[0].data = nullptr;
			}
		);

		library->add_template("RF: Get Node Name", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Node",
					.allowed_types = { TypeInfo::get<BlueprintNodePtr>() }
				}
			},
			{
				{
					.name = "Name",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto node = *(BlueprintNodePtr*)inputs[0].data;
				if (node)
				{
					auto name = node->display_name.empty() ? node->name : node->display_name;
					if (!node->template_string.empty())
						name += '#' + node->template_string;
					*(std::string*)outputs[0].data = name;
				}
				else
					*(std::string*)outputs[0].data = "";
			}
		);

		library->add_template("RF: Join Block", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Src Block",
					.allowed_types = { TypeInfo::get<BlueprintNodePtr>() }
				},
				{
					.name = "Dst Block",
					.allowed_types = { TypeInfo::get<BlueprintNodePtr>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto src_block = *(BlueprintNodePtr*)inputs[0].data;
				auto dst_block = *(BlueprintNodePtr*)inputs[1].data;
				if (src_block && dst_block && src_block->is_block && dst_block->is_block)
				{
					auto bp = dst_block->group->blueprint;
					bp->set_nodes_parent(src_block->children, dst_block);
					bp->remove_node(src_block, true);
				}
			}
		);
	}
}
