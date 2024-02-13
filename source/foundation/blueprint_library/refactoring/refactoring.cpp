#include "../../blueprint_private.h"

namespace flame
{
	static bool is_preview = false;
	static std::string* out_log = nullptr;

	void set_blueprint_refactoring_environment(bool _is_preview, std::string* _out_log)
	{
		is_preview = _is_preview;
		out_log = _out_log;
	}

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

		library->add_template("RF: Set Input Value", "", BlueprintNodeFlagNone,
			{
				{
					.name = "Node",
					.allowed_types = { TypeInfo::get<BlueprintNodePtr>() }
				},
				{
					.name = "Name",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "Value",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto node = *(BlueprintNodePtr*)inputs[0].data;
				auto& name = *(std::string*)inputs[1].data;
				auto& value = *(std::string*)inputs[2].data;
				if (node)
				{
					if (!name.empty())
					{
						if (auto i = node->find_input(name); i)
						{
							if (i->type->tag != TagU)
							{
								if (is_preview)
								{
									if (out_log)
									{
										if (!out_log->empty())
											*out_log += "\n";
										*out_log += std::format("Node({})'s {}={}", node->object_id, i->name, value);
									}
								}
								else
									i->type->unserialize(value, i->data);
							}
						}
					}
				}
			}
		);

		library->add_template("RF: Add Link", "", BlueprintNodeFlagNone,
			{
				{
					.name = "From Node",
					.allowed_types = { TypeInfo::get<BlueprintNodePtr>() }
				},
				{
					.name = "From Slot Name",
					.allowed_types = { TypeInfo::get<std::string>() }
				},
				{
					.name = "To Node",
					.allowed_types = { TypeInfo::get<BlueprintNodePtr>() }
				},
				{
					.name = "To Slot Name",
					.allowed_types = { TypeInfo::get<std::string>() }
				}
			},
			{
			},
			[](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto from_node = *(BlueprintNodePtr*)inputs[0].data;
				auto& from_slot_name = *(std::string*)inputs[1].data;
				auto to_node = *(BlueprintNodePtr*)inputs[2].data;
				auto& to_slot_name = *(std::string*)inputs[3].data;
				if (from_node && to_node)
				{
					auto bp = from_node->group->blueprint;
					auto from_slot = from_node->find_output(from_slot_name);
					auto to_slot = to_node->find_input(to_slot_name);
					if (from_slot && to_slot)
					{
						if (is_preview)
						{
							if (out_log)
							{
								if (!out_log->empty())
									*out_log += "\n";
								*out_log += std::format("From: {} {}({}), To: {} {}({})", from_node->object_id, from_slot->name, from_slot->name_hash, to_node->object_id, to_slot->name, to_slot->name_hash);
							}
						}
						else
							bp->add_link(from_slot, to_slot);
					}
				}
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
					if (is_preview)
					{
						if (out_log)
						{
							if (!out_log->empty())
								*out_log += "\n";
							*out_log += std::format("Join Block: {} -> {}", src_block->object_id, dst_block->object_id);
						}
					}
					else
					{
						bp->set_nodes_parent(src_block->children, dst_block);
						bp->remove_node(src_block, true);
					}
				}
			}
		);
	}
}
