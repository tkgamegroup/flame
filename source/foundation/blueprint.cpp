#include "../xml.h"
#include "typeinfo_serialize.h"
#include "sheet_private.h"
#include "system_private.h"
#include "blueprint_private.h"

namespace flame
{
	std::vector<std::pair<std::string, TypeInfo*>> BlueprintSystem::template_types;

	std::vector<std::unique_ptr<BlueprintT>>						loaded_blueprints;
	std::map<uint, std::pair<BlueprintPtr, BlueprintInstancePtr>>	named_blueprints;
	std::vector<std::unique_ptr<BlueprintNodeLibraryT>>				loaded_libraries;

	std::map<uint, std::vector<BlueprintInstanceGroup*>>			message_receivers;

	struct PointerAndUint
	{
		void* p;
		uint u;
	};

	static void update_depth(BlueprintNodePtr n)
	{
		n->depth = n->parent->depth + 1;
		for (auto& c : n->children)
			update_depth(c);
	}

	static void update_degree(BlueprintNodePtr n)
	{
		auto degree = 0U;
		for (auto& i : n->inputs)
		{
			if (!i->linked_slots.empty())
			{
				auto ls = i->linked_slots[0];
				degree = max(degree, ls->node->degree + 1);
			}
		}
		n->degree = degree;
		for (auto& c : n->children)
			update_degree(c);
	}

	static bool remove_link(BlueprintLinkPtr link)
	{
		auto group = link->from_slot->node->group;
		for (auto it = group->links.begin(); it != group->links.end(); it++)
		{
			if (it->get() == link)
			{
				auto from_slot = link->from_slot;
				auto to_node = link->to_slot->node;
				auto to_slot = link->to_slot;
				std::erase_if(from_slot->linked_slots, [&](const auto& slot) {
					return slot == to_slot;
				});
				std::erase_if(to_slot->linked_slots, [&](const auto& slot) {
					return slot == from_slot;
				});
				group->links.erase(it);
				return true;
			}
		}
		return false;
	}

	static void change_slot_type(BlueprintSlotPtr slot, TypeInfo* new_type)
	{
		if (slot->type == new_type)
			return;
		auto has_data = slot->data != nullptr;
		auto prev_value = slot->data ? slot->type->serialize(slot->data) : "";
		if (slot->data) // is input slot and has data
		{
			slot->type->destroy(slot->data);
			slot->data = nullptr;
		}
		if (!(slot->type && (slot->type == TypeInfo::get<voidptr>() ||
			(slot->type->tag == TagPU && new_type->tag == TagU)))) // if a udt type link to its pointer type, dont change the type
		{
			slot->type = new_type;
			if (new_type && has_data) // is input slot and has data
			{
				slot->data = new_type->create();
				if (!prev_value.empty())
					new_type->unserialize(prev_value, slot->data);
			}
		}
	}

	static void clear_invalid_links(BlueprintGroupPtr group)
	{
		std::vector<BlueprintLinkPtr> to_remove_links;
		auto done = false;
		while (!done)
		{
			done = true;
			for (auto& l : group->links)
			{
				if (!l->from_slot->type || !l->to_slot->type)
				{
					if (!l->to_slot->type && !l->to_slot->allowed_types.empty())
						change_slot_type(l->to_slot, nullptr);
					to_remove_links.push_back(l.get());
					done = false;
				}
				if (!l->from_slot->node->parent->contains(l->to_slot->node))
					to_remove_links.push_back(l.get());
			}
			for (auto l : to_remove_links)
				remove_link(l);
		}
	}

	static void remove_node_links(BlueprintNodePtr n)
	{
		auto group = n->group;
		std::vector<BlueprintLinkPtr> to_remove_links;
		for (auto& l : group->links)
		{
			if (l->from_slot->node == n || l->to_slot->node == n)
				to_remove_links.push_back(l.get());
		}
		for (auto l : to_remove_links)
			remove_link(l);
	}

	BlueprintSlotPrivate::~BlueprintSlotPrivate()
	{
		if (data)
			type->destroy(data);
	}

	uint BlueprintSlotPrivate::get_linked_count() const
	{
		return linked_slots.size();
	}

	BlueprintSlotPtr BlueprintSlotPrivate::get_linked(uint idx) const
	{
		if (idx < linked_slots.size())
			return linked_slots[idx];
		return nullptr;
	}

	BlueprintGroupPrivate::~BlueprintGroupPrivate()
	{
		for (auto& v : variables)
			v.type->destroy(v.data);
	}

	BlueprintPrivate::BlueprintPrivate()
	{
		dirty_frame = frames;
	}

	BlueprintPrivate::~BlueprintPrivate()
	{
		for (auto& e : enums)
			remove_enum(e.name_hash);
		for (auto& s : structs)
			remove_struct(s.name_hash);
		for (auto& v : variables)
			v.type->destroy(v.data);
	}

	void BlueprintPrivate::add_enum(const std::string& name, const std::vector<BlueprintEnumItem>& items)
	{
		for (auto& e : enums)
		{
			if (e.name == name)
			{
				printf("blueprint add_enum: %s enum already existed\n", name.c_str());
				return;
			}
		}

		auto name_hash = sh(name.c_str());

		auto& e = enums.emplace_back();
		e.name = name;
		e.name_hash = sh(name.c_str());
		e.items = items;

		if (auto ei = find_enum(name_hash, tidb); ei)
		{
			// extent the cpp enum
			for (auto& i : items)
			{
				if (auto ii = (EnumItemInfo*)ei->find_item(i.name_hash); ii)
					assert(0); // cpp enum already has this item
				else
					ei->items.push_back({ ei, i.name, i.name_hash, i.value });
			}
		}
		else
		{
			auto& new_ei = tidb.enums[name_hash];
			new_ei.db = &tidb;
			new_ei.name = name;
			new_ei.name_hash = name_hash;
			new_ei.is_flags = name.ends_with("Flags");
			new_ei.items.resize(items.size());
			for (auto i = 0; i < items.size(); i++)
			{
				auto& src = items[i];
				auto& dst = new_ei.items[i];
				dst.ei = &new_ei;
				dst.name = src.name;
				dst.name_hash = src.name_hash;
				dst.value = src.value;
			}
			new_ei.library = nullptr;
		}
	}

	void BlueprintPrivate::remove_enum(uint name)
	{
		for (auto it = enums.begin(); it != enums.end(); it++)
		{
			if (it->name_hash == name)
			{
				for (auto it2 = tidb.enums.begin(); it2 != tidb.enums.end(); it2++)
				{
					if (it2->second.name_hash == name)
					{
						auto& ei = it2->second;
						if (ei.library == nullptr) // not from cpp
							tidb.enums.erase(it2); // just remove it
						else
						{
							// filter out the items from bp
							for (auto& i : it->items)
							{
								for (auto it3 = ei.items.begin(); it3 != ei.items.end(); it3++)
								{
									if (it3->name_hash == i.name_hash)
									{
										ei.items.erase(it3);
										break;
									}
								}
							}
						}
						break;
					}
				}

				enums.erase(it);
				return;
			}
		}
	}

	void BlueprintPrivate::alter_enum(uint old_name, const std::string& new_name, const std::vector<BlueprintEnumItem>& new_items)
	{
		for (auto& e : enums)
		{
			if (e.name_hash == old_name)
			{
				auto new_name_hash = sh(new_name.c_str());

				if (auto ei = find_enum(old_name); ei)
				{
					ei->name = new_name;
					ei->name_hash = new_name_hash;
					ei->is_flags = new_name.ends_with("Flags");

					// filter out the items from bp
					for (auto& i : e.items)
					{
						for (auto it = ei->items.begin(); it != ei->items.end(); it++)
						{
							if (it->name_hash == i.name_hash)
							{
								ei->items.erase(it);
								break;
							}
						}
					}
					// extent the cpp enum
					for (auto& i : new_items)
					{
						if (auto ii = (EnumItemInfo*)ei->find_item(i.name_hash); ii)
							assert(0); // cpp enum already has this item
						else
							ei->items.push_back({ ei, i.name, i.name_hash, i.value });
					}
				}

				e.name = new_name;
				e.name_hash = new_name_hash;
				e.items = new_items;
			}
		}
	}

	void BlueprintPrivate::add_struct(const std::string& name, const std::vector<BlueprintStructVariable>& variables)
	{
		for (auto& s : structs)
		{
			if (s.name == name)
			{
				printf("blueprint add_struct: %s struct already existed\n", name.c_str());
				return;
			}
		}

		auto name_hash = sh(name.c_str());

		if (find_udt(name_hash, tidb))
		{
			printf("blueprint add_struct: %s struct already existed\n", name.c_str());
			return;
		}

		auto& s = structs.emplace_back();
		s.name = name;
		s.name_hash = sh(name.c_str());
		s.variables = variables;

		auto& new_ui = tidb.udts[name_hash];
		new_ui.db = &tidb;
		new_ui.name = name;
		new_ui.name_hash = name_hash;
		new_ui.size = 0;
		new_ui.pod = true;
		new_ui.variables.resize(variables.size());
		for (auto i = 0; i < variables.size(); i++)
		{
			auto& src = variables[i];
			auto& dst = new_ui.variables[i];
			dst.ui = &new_ui;
			dst.name = src.name;
			dst.name_hash = src.name_hash;
			dst.type = src.type;
			dst.offset = new_ui.size;
			new_ui.size += dst.type->size;
			dst.default_value = src.default_value;
			new_ui.variables_map[dst.name_hash] = i;
			if (!dst.type->pod)
				new_ui.pod = false;
		}
		new_ui.library = nullptr;
	}

	void BlueprintPrivate::remove_struct(uint name)
	{
		for (auto it = structs.begin(); it != structs.end(); it++)
		{
			if (it->name_hash == name)
			{
				for (auto it2 = tidb.udts.begin(); it2 != tidb.udts.end(); it2++)
				{
					if (it2->second.name_hash == name)
					{
						auto& ui = it2->second;
						if (ui.library == nullptr) // not from cpp
							tidb.udts.erase(it2); // just remove it
						break;
					}
				}

				structs.erase(it);
				return;
			}
		}
	}

	void BlueprintPrivate::alter_struct(uint old_name, const std::string& new_name, const std::vector<BlueprintStructVariable>& new_variables)
	{
		for (auto& s : structs)
		{
			if (s.name_hash == old_name)
			{
				auto new_name_hash = sh(new_name.c_str());

				if (auto ui = find_udt(old_name); ui)
				{
					if (ui->library == nullptr)  // not from cpp
					{
						ui->name = new_name;
						ui->name_hash = new_name_hash;

						ui->variables.clear();
						ui->variables_map.clear();
						ui->size = 0;
						ui->pod = true;
						ui->variables.resize(new_variables.size());
						for (auto i = 0; i < new_variables.size(); i++)
						{
							auto& src = new_variables[i];
							auto& dst = ui->variables[i];
							dst.ui = ui;
							dst.name = src.name;
							dst.name_hash = src.name_hash;
							dst.type = src.type;
							dst.offset = ui->size;
							ui->size += dst.type->size;
							dst.default_value = src.default_value;
							ui->variables_map[dst.name_hash] = i;
							if (!dst.type->pod)
								ui->pod = false;
						}
					}
				}

				s.name = new_name;
				s.name_hash = new_name_hash;
				s.variables = new_variables;
			}
		}
	}

	void* BlueprintPrivate::add_variable(BlueprintGroupPtr group, const std::string& name, TypeInfo* type)
	{
		assert(!group || group->blueprint == this);
		auto& vars = group ? group->variables : variables;

		for (auto& v : vars)
		{
			if (v.name == name)
			{
				printf("blueprint add_variable: variable already exists\n");
				return v.data;
			}
		}

		auto& v = vars.emplace_back();
		v.name = name;
		v.name_hash = sh(name.c_str());
		v.type = type;
		v.data = v.type->create();

		auto frame = frames;
		if (group)
		{
			group->variable_changed_frame = frame;
			group->structure_changed_frame = frame;
		}
		else
		{
			variable_changed_frame = frame;
			for (auto& g : groups)
				g->structure_changed_frame = frame;
		}
		dirty_frame = frame;

		return v.data;
	}

	void BlueprintPrivate::remove_variable(BlueprintGroupPtr group, uint name)
	{
		assert(!group || group->blueprint == this);
		auto& vars = group ? group->variables : variables;

		for (auto it = vars.begin(); it != vars.end(); ++it)
		{
			if (it->name_hash == name)
			{
				it->type->destroy(it->data);
				vars.erase(it);
				break;
			}
		}

		std::vector<BlueprintNodePtr> to_remove_nodes;
		auto process_group = [&](BlueprintGroupPtr group) {
			for (auto& n : group->nodes)
			{
				if (blueprint_is_variable_node(n->name_hash))
				{
					if (*(uint*)n->inputs[0]->data == name && *(uint*)n->inputs[1]->data == 0)
						to_remove_nodes.push_back(n.get());
				}
			}
		};
		if (group)
			process_group(group);
		else
		{
			for (auto& g : groups)
				process_group(g.get());
		}
		for (auto n : to_remove_nodes)
			remove_node(n, false);

		auto frame = frames;
		if (group)
		{
			group->variable_changed_frame = frame;
			group->structure_changed_frame = frame;
		}
		else
		{
			variable_changed_frame = frame;
			for (auto& g : groups)
				g->structure_changed_frame = frame;
		}
		dirty_frame = frame;
	}

	void BlueprintPrivate::alter_variable(BlueprintGroupPtr group, uint old_name, const std::string& new_name, TypeInfo* new_type)
	{
		assert(!group || group->blueprint == this);
		auto& vars = group ? group->variables : variables;

		for (auto it = vars.begin(); it != vars.end(); ++it)
		{
			if (it->name_hash == old_name)
			{
				auto changed = false;
				if (!new_name.empty())
				{
					if (it->name != new_name)
					{
						it->name = new_name;
						it->name_hash = sh(new_name.c_str());
						changed = true;
					}
				}
				if (new_type)
				{
					if (it->type != new_type)
					{
						it->type->destroy(it->data);
						it->type = new_type;
						it->data = new_type->create();
						changed = true;
					}
				}
				if (changed)
					change_references(group, old_name, 0, 0, it->name_hash, 0, 0);

				auto frame = frames;
				if (group)
				{
					group->variable_changed_frame = frame;
					group->structure_changed_frame = frame;
				}
				else
				{
					variable_changed_frame = frame;
					for (auto& g : groups)
						g->structure_changed_frame = frame;
				}
				dirty_frame = frame;
				return;
			}
		}

		printf("blueprint alter_variable: variable not found\n");
	}

	BlueprintSlotPtr BlueprintPrivate::create_slot(BlueprintNodePtr n, const BlueprintSlotDesc& desc, int pos)
	{
		auto s = new BlueprintSlotPrivate;
		s->node = n;
		s->object_id = next_object_id++;
		s->name = desc.name;
		s->name_hash = desc.name_hash;
		s->flags = desc.flags;
		s->allowed_types = desc.allowed_types;
		s->type = desc.allowed_types.front();
		s->data = s->type->create();
		if (s->flags & BlueprintSlotFlagInput)
		{
			if (!desc.default_value.empty())
				s->type->unserialize(desc.default_value, s->data);
			if (pos == -1)
				n->inputs.emplace_back(s);
			else
				n->inputs.emplace(n->inputs.begin() + pos, s);
		}
		else
		{

			if (pos == -1)
				n->outputs.emplace_back(s);
			else
				n->outputs.emplace(n->outputs.begin() + pos, s);
		}
		return s;
	}

	BlueprintNodePtr BlueprintPrivate::add_node(BlueprintGroupPtr group, BlueprintNodePtr parent, const std::string& name, BlueprintNodeFlags flags, const std::string& display_name,
		const std::vector<BlueprintSlotDesc>& inputs, const std::vector<BlueprintSlotDesc>& outputs,
		BlueprintNodeFunction function, BlueprintNodeLoopFunction loop_function,
		BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeChangeStructureCallback change_structure_callback, BlueprintNodePreviewProvider preview_provider, 
		bool is_block, BlueprintNodeBeginBlockFunction begin_block_function, BlueprintNodeEndBlockFunction end_block_function)
	{
		assert(group && group->blueprint == this);
		if (parent)
			assert(group == parent->group);
		else if (!group->nodes.empty())
			parent = group->nodes.front().get();

		auto ret = new BlueprintNodePrivate;
		ret->group = group;
		ret->object_id = next_object_id++;
		ret->name = name;
		ret->name_hash = sh(name.c_str());
		ret->flags = flags;
		ret->display_name = display_name;
		for (auto& src_i : inputs)
		{
			create_slot(ret, {
				.name = src_i.name,
				.name_hash = src_i.name_hash,
				.flags = src_i.flags | BlueprintSlotFlagInput,
				.allowed_types = src_i.allowed_types,
				.default_value = src_i.default_value
			});
		}
		for (auto& src_o : outputs)
		{
			create_slot(ret, {
				.name = src_o.name,
				.name_hash = src_o.name_hash,
				.flags = src_o.flags | BlueprintSlotFlagOutput,
				.allowed_types = src_o.allowed_types
			});
		}
		ret->constructor = constructor;
		ret->destructor = destructor;
		ret->function = function;
		ret->loop_function = loop_function;
		ret->change_structure_callback = change_structure_callback;
		ret->preview_provider = preview_provider;
		ret->is_block = is_block;
		ret->begin_block_function = begin_block_function;
		ret->end_block_function = end_block_function;
		ret->parent = parent;
		if (parent)
		{
			parent->children.push_back(ret);
			ret->depth = parent->depth + 1;
		}
		group->nodes.emplace_back(ret);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	BlueprintNodePtr BlueprintPrivate::add_node(BlueprintGroupPtr g, BlueprintNodePtr parent, uint name_hash)
	{
		for (auto& library : loaded_libraries)
		{
			for (auto& t : library->node_templates)
			{
				if (t.name_hash == name_hash)
				{
					auto n = add_node(g, parent, t.name, t.flags, t.display_name, t.inputs, t.outputs,
						t.function, t.loop_function, t.constructor, t.destructor, t.change_structure_callback, t.preview_provider,
						t.is_block, t.begin_block_function, t.end_block_function);
					return n;
				}
			}
		}
		return nullptr;
	}

	BlueprintNodePtr BlueprintPrivate::add_block(BlueprintGroupPtr group, BlueprintNodePtr parent)
	{
		return add_node(group, parent, "Block", BlueprintNodeFlagNone, "", {
				{
					.name = "Execute",
					.name_hash = "Execute"_h,
					.allowed_types = { TypeInfo::get<BlueprintSignal>() }
				}
			}, {}, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, true);
	}

	BlueprintNodePtr BlueprintPrivate::add_variable_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint variable_name, uint type, uint location_name, uint property_name)
	{
		assert(group && group->blueprint == this);
		if (parent)
			assert(group == parent->group);
		else
			parent = group->nodes.front().get();

		if (!blueprint_is_variable_node(type))
		{
			printf("blueprint add_variable_node: type %u is not a variable node\n", type);
			return nullptr;
		}

		BlueprintVariable variable;
		std::string location_str;
		const VariableInfo* property = nullptr;
		auto found = false;
		if (location_name == 0 || location_name == name_hash)
		{
			location_name = 0;
			for (auto& v : variables)
			{
				if (v.name_hash == variable_name)
				{
					variable = v;
					found = true;
					break;
				}
			}
			if (!found)
			{
				for (auto& v : group->variables)
				{
					if (v.name_hash == variable_name)
					{
						variable = v;
						found = true;
						break;
					}
				}
			}
			if (!found)
			{
				printf("blueprint add_variable_node: cannot find variable %u\n", variable_name);
				return nullptr;
			}
		}
		else
		{
			if (auto bp = Blueprint::get(location_name); bp)
			{
				for (auto& v : bp->variables)
				{
					if (v.name_hash == variable_name)
					{
						location_str = bp->name + '.';
						variable = v;
						found = true;
						break;
					}
				}

				if (!found)
				{
					printf("blueprint add_variable_node: cannot find variable %u in blueprint '%s'\n", variable_name, bp->name.c_str());
					return nullptr;
				}
			}
			else
			{
				if (auto ei = find_enum(location_name); ei)
				{
					auto ii = ei->find_item(variable_name);
					if (!ii)
					{
						printf("blueprint add_variable_node: cannot find item %u in enum '%s'\n", variable_name, ei->name.c_str());
						return nullptr;
					}

					location_str = ei->name + '.';
					variable.name = ii->name;
					variable.name_hash = ii->name_hash;
					variable.type = TypeInfo::get<int>();
				}
				else
				{
					printf("blueprint add_variable_node: cannot find location %u\n", location_name);
					return nullptr;
				}
			}
		}

		if (property_name)
		{
			if (variable.type->tag != TagU && variable.type->tag != TagVU)
			{
				printf("blueprint add_variable_node: getting/setting attributes only works on udts\n");
				return nullptr;
			}

			auto ui = variable.type->retrive_ui();
			property = ui->find_variable(property_name);
			if (!property)
			{
				printf("blueprint add_variable_node: cannot find attribute(%u) in udt %s\n", property_name, ui->name.c_str());
				return nullptr;
			}
		}

		BlueprintNodePtr ret = nullptr;
		auto setup_variable_inputs = [&](BlueprintNodePtr ret) {
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Name";
				i->name_hash = "Name"_h;
				i->flags = BlueprintSlotFlagInput | BlueprintSlotFlagHideInUI;
				i->allowed_types.push_back(TypeInfo::get<uint>());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				*(uint*)i->data = variable.name_hash;
				ret->inputs.emplace_back(i);
			}
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Location";
				i->name_hash = "Location"_h;
				i->flags = BlueprintSlotFlagInput | BlueprintSlotFlagHideInUI;
				i->allowed_types.push_back(TypeInfo::get<uint>());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				*(uint*)i->data = location_name;
				ret->inputs.emplace_back(i);
			}
		};
		switch (type)
		{
		case "Variable"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Variable";
			ret->name_hash = "Variable"_h;
			ret->display_name = location_str + variable.name;
			setup_variable_inputs(ret);
			{
				auto o = new BlueprintSlotPrivate;
				o->node = ret;
				o->object_id = next_object_id++;
				o->name = "V";
				o->name_hash = "V"_h;
				o->flags = BlueprintSlotFlagOutput;
				o->allowed_types.push_back(variable.type);
				o->type = variable.type;
				ret->outputs.emplace_back(o);
			}
			break;
		case "Set Variable"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Set Variable";
			ret->name_hash = "Set Variable"_h;
			ret->display_name = "Set " + location_str + variable.name;
			setup_variable_inputs(ret);
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "V";
				i->name_hash = "V"_h;
				i->flags = BlueprintSlotFlagInput;
				i->allowed_types.push_back(variable.type);
				i->type = variable.type;
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto type = inputs[0].type;
				auto data = inputs[0].data;
				if (data)
					type->copy(inputs[0].data, inputs[1].data);
			};
			break;
		case "Add Assign"_h:
			if (variable.type->tag == TagD)
			{
				auto ti = (TypeInfo_Data*)variable.type;
				if ((ti->data_type == DataInt || ti->data_type == DataFloat) && ti->vec_size == 1)
				{
					ret = new BlueprintNodePrivate;
					ret->group = group;
					ret->object_id = next_object_id++;
					ret->name = "Add Assign";
					ret->name_hash = "Add Assign"_h;
					ret->display_name = location_str + variable.name + " +=";
					setup_variable_inputs(ret);
					{
						auto i = new BlueprintSlotPrivate;
						i->node = ret;
						i->object_id = next_object_id++;
						i->name = "V";
						i->name_hash = "V"_h;
						i->flags = BlueprintSlotFlagInput;
						i->allowed_types.push_back(variable.type);
						i->type = variable.type;
						i->data = i->type->create();
						ret->inputs.emplace_back(i);
					}
					ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
						auto type = inputs[0].type;
						auto data = inputs[0].data;
						if (data)
						{
							if (type == TypeInfo::get<float>())
								*(float*)data += *(float*)inputs[1].data;
							else if (type == TypeInfo::get<int>())
								*(int*)data += *(int*)inputs[1].data;
							else if (type == TypeInfo::get<uint>())
								*(uint*)data += *(uint*)inputs[1].data;
						}
					};
				}
				else
					printf("blueprint add_variable_node: Addition Assign only works on int/uint/float\n");
			}
			else
				printf("blueprint add_variable_node: Addition Assign only works on int/uint/float\n");
			break;
		case "Subtract Assign"_h:
			if (variable.type->tag == TagD)
			{
				auto ti = (TypeInfo_Data*)variable.type;
				if ((ti->data_type == DataInt || ti->data_type == DataFloat) && ti->vec_size == 1)
				{
					ret = new BlueprintNodePrivate;
					ret->group = group;
					ret->object_id = next_object_id++;
					ret->name = "Subtract Assign";
					ret->name_hash = "Subtract Assign"_h;
					ret->display_name = location_str + variable.name + " -=";
					setup_variable_inputs(ret);
					{
						auto i = new BlueprintSlotPrivate;
						i->node = ret;
						i->object_id = next_object_id++;
						i->name = "V";
						i->name_hash = "V"_h;
						i->flags = BlueprintSlotFlagInput;
						i->allowed_types.push_back(variable.type);
						i->type = variable.type;
						i->data = i->type->create();
						ret->inputs.emplace_back(i);
					}
					ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
						auto type = inputs[0].type;
						auto data = inputs[0].data;
						if (data)
						{
							if (type == TypeInfo::get<float>())
								*(float*)data -= *(float*)inputs[1].data;
							else if (type == TypeInfo::get<int>())
								*(int*)data -= *(int*)inputs[1].data;
							else if (type == TypeInfo::get<uint>())
								*(uint*)data -= *(uint*)inputs[1].data;
						}
					};
				}
				else
					printf("blueprint add_variable_node: Addition Assign only works on int/uint/float\n");
			}
			else
				printf("blueprint add_variable_node: Addition Assign only works on int/uint/float\n");
			break;
		case "Multiply Assign"_h:
			if (variable.type->tag == TagD)
			{
				auto ti = (TypeInfo_Data*)variable.type;
				if ((ti->data_type == DataInt || ti->data_type == DataFloat) && ti->vec_size == 1)
				{
					ret = new BlueprintNodePrivate;
					ret->group = group;
					ret->object_id = next_object_id++;
					ret->name = "Multiply Assign";
					ret->name_hash = "Multiply Assign"_h;
					ret->display_name = location_str + variable.name + " *=";
					setup_variable_inputs(ret);
					{
						auto i = new BlueprintSlotPrivate;
						i->node = ret;
						i->object_id = next_object_id++;
						i->name = "V";
						i->name_hash = "V"_h;
						i->flags = BlueprintSlotFlagInput;
						i->allowed_types.push_back(variable.type);
						i->type = variable.type;
						i->data = i->type->create();
						ret->inputs.emplace_back(i);
					}
					ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
						auto type = inputs[0].type;
						auto data = inputs[0].data;
						if (data)
						{
							if (type == TypeInfo::get<float>())
								*(float*)data *= *(float*)inputs[1].data;
							else if (type == TypeInfo::get<int>())
								*(int*)data *= *(int*)inputs[1].data;
							else if (type == TypeInfo::get<uint>())
								*(uint*)data *= *(uint*)inputs[1].data;
						}
					};
				}
				else
					printf("blueprint add_variable_node: Addition Assign only works on int/uint/float\n");
			}
			else
				printf("blueprint add_variable_node: Addition Assign only works on int/uint/float\n");
			break;
		case "Divide Assign"_h:
			if (variable.type->tag == TagD)
			{
				auto ti = (TypeInfo_Data*)variable.type;
				if ((ti->data_type == DataInt || ti->data_type == DataFloat) && ti->vec_size == 1)
				{
					ret = new BlueprintNodePrivate;
					ret->group = group;
					ret->object_id = next_object_id++;
					ret->name = "Divide Assign";
					ret->name_hash = "Divide Assign"_h;
					ret->display_name = location_str + variable.name + " /=";
					setup_variable_inputs(ret);
					{
						auto i = new BlueprintSlotPrivate;
						i->node = ret;
						i->object_id = next_object_id++;
						i->name = "V";
						i->name_hash = "V"_h;
						i->flags = BlueprintSlotFlagInput;
						i->allowed_types.push_back(variable.type);
						i->type = variable.type;
						i->data = i->type->create();
						ret->inputs.emplace_back(i);
					}
					ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
						auto type = inputs[0].type;
						auto data = inputs[0].data;
						if (data)
						{
							if (type == TypeInfo::get<float>())
								*(float*)data /= *(float*)inputs[1].data;
							else if (type == TypeInfo::get<int>())
								*(int*)data /= *(int*)inputs[1].data;
							else if (type == TypeInfo::get<uint>())
								*(uint*)data /= *(uint*)inputs[1].data;
						}
					};
				}
				else
					printf("blueprint add_variable_node: Addition Assign only works on int/uint/float\n");
			}
			else
				printf("blueprint add_variable_node: Addition Assign only works on int/uint/float\n");
			break;
		case "Or Assign"_h:
			if (variable.type->tag == TagD)
			{
				auto ti = (TypeInfo_Data*)variable.type;
				if (ti->data_type == DataInt && ti->vec_size == 1)
				{
					ret = new BlueprintNodePrivate;
					ret->group = group;
					ret->object_id = next_object_id++;
					ret->name = "Or Assign";
					ret->name_hash = "Or Assign"_h;
					ret->display_name = location_str + variable.name + " |=";
					setup_variable_inputs(ret);
					{
						auto i = new BlueprintSlotPrivate;
						i->node = ret;
						i->object_id = next_object_id++;
						i->name = "V";
						i->name_hash = "V"_h;
						i->flags = BlueprintSlotFlagInput;
						i->allowed_types.push_back(variable.type);
						i->type = variable.type;
						i->data = i->type->create();
						ret->inputs.emplace_back(i);
					}
					ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
						auto type = inputs[0].type;
						auto data = inputs[0].data;
						if (data)
						{
							if (type == TypeInfo::get<int>())
								*(int*)data |= *(int*)inputs[1].data;
							else if (type == TypeInfo::get<uint>())
								*(uint*)data |= *(uint*)inputs[1].data;
						}
					};
				}
				else
					printf("blueprint add_variable_node: Addition Assign only works on int/uint\n");
			}
			else
				printf("blueprint add_variable_node: Addition Assign only works on int/uint\n");
			break;
		case "And Assign"_h:
			if (variable.type->tag == TagD)
			{
				auto ti = (TypeInfo_Data*)variable.type;
				if (ti->data_type == DataInt && ti->vec_size == 1)
				{
					ret = new BlueprintNodePrivate;
					ret->group = group;
					ret->object_id = next_object_id++;
					ret->name = "And Assign";
					ret->name_hash = "And Assign"_h;
					ret->display_name = location_str + variable.name + " &=";
					setup_variable_inputs(ret);
					{
						auto i = new BlueprintSlotPrivate;
						i->node = ret;
						i->object_id = next_object_id++;
						i->name = "V";
						i->name_hash = "V"_h;
						i->flags = BlueprintSlotFlagInput;
						i->allowed_types.push_back(variable.type);
						i->type = variable.type;
						i->data = i->type->create();
						ret->inputs.emplace_back(i);
					}
					ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
						auto type = inputs[0].type;
						auto data = inputs[0].data;
						if (data)
						{
							if (type == TypeInfo::get<int>())
								*(int*)data &= *(int*)inputs[1].data;
							else if (type == TypeInfo::get<uint>())
								*(uint*)data &= *(uint*)inputs[1].data;
						}
					};
				}
				else
					printf("blueprint add_variable_node: Addition Assign only works on int/uint\n");
			}
			else
				printf("blueprint add_variable_node: Addition Assign only works on int/uint\n");
			break;
		case "Get Property"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Get Property";
			ret->name_hash = "Get Property"_h;
			ret->display_name = "Get " + location_str + variable.name + '.' + property->name;
			setup_variable_inputs(ret);
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Property";
				i->name_hash = "Property"_h;
				i->flags = BlueprintSlotFlagInput | BlueprintSlotFlagHideInUI;
				i->allowed_types.push_back(TypeInfo::get<uint>());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				*(uint*)i->data = property_name;
				ret->inputs.emplace_back(i);
			}
			{
				auto o = new BlueprintSlotPrivate;
				o->node = ret;
				o->object_id = next_object_id++;
				o->name = "V";
				o->name_hash = "V"_h;
				o->flags = BlueprintSlotFlagOutput;
				o->allowed_types.push_back(property->type);
				o->type = property->type;
				ret->outputs.emplace_back(o);
			}
			break;
		case "Get Properties"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Get Property";
			ret->name_hash = "Get Property"_h;
			ret->display_name = "Get " + location_str + variable.name + ".*";
			setup_variable_inputs(ret);
			assert(property_name == 0);
			{
				auto o = new BlueprintSlotPrivate;
				o->node = ret;
				o->object_id = next_object_id++;
				o->name = "V";
				o->name_hash = "V"_h;
				o->flags = BlueprintSlotFlagOutput;
				o->allowed_types.push_back(property->type);
				o->type = property->type;
				ret->outputs.emplace_back(o);
			}
			if (auto ui = variable.type->retrive_ui(); ui)
			{
				for (auto& vi : ui->variables)
				{
					auto o = new BlueprintSlotPrivate;
					o->node = ret;
					o->object_id = next_object_id++;
					o->name = vi.name;
					o->name_hash = vi.name_hash;
					o->flags = BlueprintSlotFlagOutput;
					o->allowed_types.push_back(vi.type);
					o->type = vi.type;
					ret->outputs.emplace_back(o);
				}
			}
			else
				assert(0);
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto data = inputs[0].data;
				auto type = inputs[0].type;
				auto ui = type->retrive_ui();
				for (auto i = 0; i < outputs_count; i++)
					outputs[i].type->copy(outputs[i].data, (char*)data + ui->variables[i].offset);
			};
			break;
		case "Set Property"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Set Property";
			ret->name_hash = "Set Property"_h;
			ret->display_name = "Set " + location_str + variable.name + '.' + property->name;
			setup_variable_inputs(ret);
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Property";
				i->name_hash = "Property"_h;
				i->flags = BlueprintSlotFlagInput | BlueprintSlotFlagHideInUI;
				i->allowed_types.push_back(TypeInfo::get<uint>());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				*(uint*)i->data = property_name;
				ret->inputs.emplace_back(i);
			}
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "V";
				i->name_hash = "V"_h;
				i->flags = BlueprintSlotFlagInput;
				i->allowed_types.push_back(variable.type);
				i->type = variable.type;
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto type = inputs[0].type;
				auto data = inputs[0].data;
				if (data)
					type->copy(inputs[0].data, inputs[1].data);
			};
			break;
		case "Array Size"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Array Size";
			ret->name_hash = "Array Size"_h;
			ret->display_name = location_str + variable.name + ": Size";
			setup_variable_inputs(ret);
			{
				auto o = new BlueprintSlotPrivate;
				o->node = ret;
				o->object_id = next_object_id++;
				o->name = "V";
				o->name_hash = "V"_h;
				o->flags = BlueprintSlotFlagOutput;
				o->allowed_types.push_back(TypeInfo::get<uint>());
				o->type = o->allowed_types.front();
				ret->outputs.emplace_back(o);

			}
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto item_type = inputs[0].type->get_wrapped();
				auto parray = (std::vector<char>*)inputs[0].data;
				if (parray)
					*(uint*)outputs[0].data = parray->size() / item_type->size;
			};
			break;
		case "Array Clear"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Array Clear";
			ret->name_hash = "Array Clear"_h;
			ret->display_name = location_str + variable.name + ": Clear";
			setup_variable_inputs(ret);
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto parray = inputs[0].data;
				auto item_type = inputs[0].type->get_wrapped();
				resize_vector(parray, item_type, 0);
			};
			break;
		case "Array Get Item"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Array Get Item";
			ret->name_hash = "Array Get Item"_h;
			ret->display_name = "Get " + location_str + variable.name + "[]";
			setup_variable_inputs(ret);
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Index";
				i->name_hash = "Index"_h;
				i->flags = BlueprintSlotFlagInput;
				i->allowed_types.push_back(TypeInfo::get<uint>());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			{
				auto o = new BlueprintSlotPrivate;
				o->node = ret;
				o->object_id = next_object_id++;
				o->name = "V";
				o->name_hash = "V"_h;
				o->flags = BlueprintSlotFlagOutput;
				o->allowed_types.push_back(variable.type->get_wrapped());
				o->type = o->allowed_types.front();
				o->data = o->type->create();
				ret->outputs.emplace_back(o);

			}
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto parray = inputs[0].data;
				auto index = *(uint*)inputs[1].data;
				auto pitem = outputs[0].data;
				auto item_type = outputs[0].type;
				if (parray && pitem && item_type)
				{
					auto& array = *(std::vector<char>*)parray;
					auto array_size = array.size() / item_type->size;
					if (index < array_size)
						item_type->copy(pitem, array.data() + index * item_type->size);
				}
			};
			break;
		case "Array Set Item"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Array Set Item";
			ret->name_hash = "Array Set Item"_h;
			ret->display_name = "Set " + location_str + variable.name + "[]";
			setup_variable_inputs(ret);
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Index";
				i->name_hash = "Index"_h;
				i->flags = BlueprintSlotFlagInput;
				i->allowed_types.push_back(TypeInfo::get<uint>());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "V";
				i->name_hash = "V"_h;
				i->flags = BlueprintSlotFlagInput;
				i->allowed_types.push_back(variable.type->get_wrapped());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto parray = inputs[0].data;
				auto index = *(uint*)inputs[1].data;
				auto pitem = inputs[2].data;
				auto item_type = inputs[2].type;
				if (parray && pitem && item_type)
				{
					auto& array = *(std::vector<char>*)parray;
					auto array_size = array.size() / item_type->size;
					if (index < array_size)
						item_type->copy(array.data() + index * item_type->size, pitem);
				}
			};
			break;
		case "Array Get Item Property"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Array Get Item Property";
			ret->name_hash = "Array Get Item Property"_h;
			ret->display_name = "Get " + location_str + variable.name + "[]." + property->name;
			setup_variable_inputs(ret);
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Property";
				i->name_hash = "Property"_h;
				i->flags = BlueprintSlotFlagInput | BlueprintSlotFlagHideInUI;
				i->allowed_types.push_back(TypeInfo::get<uint>());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				*(uint*)i->data = property_name;
				ret->inputs.emplace_back(i);
			}
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Index";
				i->name_hash = "Index"_h;
				i->flags = BlueprintSlotFlagInput;
				i->allowed_types.push_back(TypeInfo::get<uint>());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			{
				auto o = new BlueprintSlotPrivate;
				o->node = ret;
				o->object_id = next_object_id++;
				o->name = "V";
				o->name_hash = "V"_h;
				o->flags = BlueprintSlotFlagOutput;
				o->allowed_types.push_back(property->type);
				o->type = property->type;
				ret->outputs.emplace_back(o);
			}
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto parray = inputs[0].data;
				auto item_type = inputs[0].type->get_wrapped();
				auto offset = *(uint*)inputs[1].data;
				auto index = *(uint*)inputs[2].data;
				auto pitem = outputs[0].data;
				auto attr_type = outputs[0].type;
				if (parray && pitem && item_type)
				{
					auto& array = *(std::vector<char>*)parray;
					auto array_size = array.size() / item_type->size;
					if (index < array_size)
						attr_type->copy(pitem, array.data() + index * item_type->size + offset);
				}
			};
			break;
		case "Array Get Item Properties"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Array Get Item Properties";
			ret->name_hash = "Array Get Item Properties"_h;
			ret->display_name = "Get " + location_str + variable.name + "[].*";
			setup_variable_inputs(ret);
			assert(property_name == 0);
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Index";
				i->name_hash = "Index"_h;
				i->flags = BlueprintSlotFlagInput;
				i->allowed_types.push_back(TypeInfo::get<uint>());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			if (auto ui = variable.type->retrive_ui(); ui)
			{
				for (auto& vi : ui->variables)
				{
					auto o = new BlueprintSlotPrivate;
					o->node = ret;
					o->object_id = next_object_id++;
					o->name = vi.name;
					o->name_hash = vi.name_hash;
					o->flags = BlueprintSlotFlagOutput;
					o->allowed_types.push_back(vi.type);
					o->type = vi.type;
					ret->outputs.emplace_back(o);
				}
			}
			else
				assert(0);
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				//auto parray = inputs[0].data;
				//auto array_type = inputs[0].type;
				//auto& array = *(std::vector<char>*)parray;
				//auto item_type = array_type->get_wrapped();
				//auto item_ui = item_type->retrive_ui();
				//auto index = *(uint*)inputs[1].data;
				//auto array_size = array.size() / item_type->size;
				//if (index < array_size)
				//{
				//	auto pitem = array.data() + index * item_type->size;
				//	for (auto i = 0; i < outputs_count; i++)
				//		outputs[i].type->copy(outputs[i].data, pitem + item_ui->variables[i].offset);
				//}
				for (auto i = 0; i < outputs_count; i++)
					memset(outputs[i].data, 0, outputs[i].type->size);
			};
			break;
		case "Array Set Item Property"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Array Set Item Property";
			ret->name_hash = "Array Set Item Property"_h;
			ret->display_name = "Set " + location_str + variable.name + "[]." + property->name;
			setup_variable_inputs(ret);
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Property";
				i->name_hash = "Property"_h;
				i->flags = BlueprintSlotFlagInput | BlueprintSlotFlagHideInUI;
				i->allowed_types.push_back(TypeInfo::get<uint>());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				*(uint*)i->data = property_name;
				ret->inputs.emplace_back(i);
			}
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Index";
				i->name_hash = "Index"_h;
				i->flags = BlueprintSlotFlagInput;
				i->allowed_types.push_back(TypeInfo::get<uint>());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "V";
				i->name_hash = "V"_h;
				i->flags = BlueprintSlotFlagInput;
				i->allowed_types.push_back(property->type);
				i->type = property->type;
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto parray = inputs[0].data;
				auto item_type = inputs[0].type->get_wrapped();
				auto offset = *(uint*)inputs[1].data;
				auto index = *(uint*)inputs[2].data;
				auto pitem = inputs[2].data;
				auto attr_type = inputs[2].type;
				if (parray && pitem && item_type)
				{
					auto& array = *(std::vector<char>*)parray;
					auto array_size = array.size() / item_type->size;
					if (index < array_size)
						attr_type->copy(array.data() + index * item_type->size + offset, pitem);
				}
				};
			break;
		case "Array Add Item"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Array Add Item";
			ret->name_hash = "Array Add Item"_h;
			ret->display_name = location_str + variable.name + ": Add Item";
			setup_variable_inputs(ret);
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "V";
				i->name_hash = "V"_h;
				i->flags = BlueprintSlotFlagInput;
				i->allowed_types.push_back(variable.type->get_wrapped());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto parray = inputs[0].data;
				auto pitem = inputs[1].data;
				auto item_type = inputs[1].type;
				if (parray && pitem && item_type)
				{
					auto& array = *(std::vector<char>*)parray;
					auto array_size = array.size() / item_type->size;
					resize_vector(parray, item_type, array_size + 1);
					item_type->copy(array.data() + array_size * item_type->size, pitem);
				}
			};
			break;
		case "Array Emplace Item"_h:
			ret = new BlueprintNodePrivate;
			ret->group = group;
			ret->object_id = next_object_id++;
			ret->name = "Array Emplace Item";
			ret->name_hash = "Array Emplace Item"_h;
			ret->display_name = location_str + variable.name + ": Emplace Item";
			setup_variable_inputs(ret);
			assert(property_name == 0);
			if (auto ui = variable.type->retrive_ui(); ui)
			{
				for (auto& vi : ui->variables)
				{
					auto i = new BlueprintSlotPrivate;
					i->node = ret;
					i->object_id = next_object_id++;
					i->name = vi.name;
					i->name_hash = vi.name_hash;
					i->flags = BlueprintSlotFlagInput;
					i->allowed_types.push_back(vi.type);
					i->type = vi.type;
					i->data = i->type->create();
					ret->inputs.emplace_back(i);
				}
			}
			else
				assert(0);
			ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
				auto parray = inputs[0].data;
				auto array_type = inputs[0].type;
				auto& array = *(std::vector<char>*)parray;
				auto item_type = array_type->get_wrapped();
				auto item_ui = item_type->retrive_ui();
				auto array_size = array.size() / item_type->size;
				if (parray && array_type)
					resize_vector(parray, item_type, array_size + 1);
				auto pitem = array.data() + array_size * item_type->size;
				for (auto i = 1; i < inputs_count; i++)
					inputs[i].type->copy(pitem + item_ui->variables[i - 1].offset, inputs[i].data);
			};
			break;
		default:
			assert(0);
		}

		if (ret)
		{
			ret->parent = parent;
			if (parent)
			{
				parent->children.push_back(ret);
				ret->depth = parent->depth + 1;
			}
			group->nodes.emplace_back(ret);

			auto frame = frames;
			group->structure_changed_frame = frame;
			dirty_frame = frame;
		}

		return ret;
	}

	BlueprintNodePtr BlueprintPrivate::add_call_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint group_name, uint location_name)
	{
		assert(group && group->blueprint == this);
		if (parent)
			assert(group == parent->group);
		else
			parent = group->nodes.front().get();

		std::string location;
		BlueprintGroupPtr call_group = nullptr;
		if (location_name == 0 || location_name == name_hash)
		{
			location_name = 0;
			for (auto& g : groups)
			{
				if (g->name_hash == group_name)
				{
					call_group = g.get();
					break;
				}
			}

			if (!call_group)
			{
				printf("blueprint add_call_node: cannot find group %u\n", group_name);
				return nullptr;
			}
			if (call_group == group)
			{
				printf("blueprint add_call_node: cannot call itself %u\n", group_name);
				return nullptr;
			}
		}
		else
		{
			auto bp = Blueprint::get(location_name);
			if (bp)
			{
				for (auto& g : bp->groups)
				{
					if (g->name_hash == group_name)
					{
						location = bp->name + '.';
						call_group = g.get();
						break;
					}
				}

				if (!call_group)
				{
					printf("blueprint add_call_node: cannot find group %u in blueprint '%s'\n", group_name, bp->name.c_str());
					return nullptr;
				}
			}
		}

		auto ret = new BlueprintNodePrivate;
		ret->group = group;
		ret->object_id = next_object_id++;
		ret->name = "Call";
		ret->name_hash = "Call"_h;
		ret->display_name = "Call " + location + call_group->name;
		{
			auto i = new BlueprintSlotPrivate;
			i->node = ret;
			i->object_id = next_object_id++;
			i->name = "Name";
			i->name_hash = "Name"_h;
			i->flags = BlueprintSlotFlagInput | BlueprintSlotFlagHideInUI;
			i->allowed_types.push_back(TypeInfo::get<uint>());
			i->type = i->allowed_types.front();
			i->data = i->type->create();
			*(uint*)i->data = call_group->name_hash;
			ret->inputs.emplace_back(i);
		}
		{
			auto i = new BlueprintSlotPrivate;
			i->node = ret;
			i->object_id = next_object_id++;
			i->name = "Location";
			i->name_hash = "Location"_h;
			i->flags = BlueprintSlotFlagInput | BlueprintSlotFlagHideInUI;
			i->allowed_types.push_back(TypeInfo::get<uint>());
			i->type = i->allowed_types.front();
			i->data = i->type->create();
			*(uint*)i->data = location_name;
			i->default_value = "0";
			ret->inputs.emplace_back(i);
		}
		for (auto& src_i : call_group->inputs)
		{
			auto i = new BlueprintSlotPrivate;
			i->node = ret;
			i->object_id = next_object_id++;
			i->name = src_i.name;
			i->name_hash = src_i.name_hash;
			i->flags = BlueprintSlotFlagInput;
			i->allowed_types.push_back(src_i.type);
			i->type = src_i.type;
			i->data = i->type->create();
			ret->inputs.emplace_back(i);
		}
		for (auto& src_o : call_group->outputs)
		{
			auto o = new BlueprintSlotPrivate;
			o->node = ret;
			o->object_id = next_object_id++;
			o->name = src_o.name;
			o->name_hash = src_o.name_hash;
			o->flags = BlueprintSlotFlagOutput;
			o->allowed_types.push_back(src_o.type);
			o->type = o->allowed_types.front();
			o->data = o->type->create();
			ret->outputs.emplace_back(o);
		}
		ret->function = [](uint inputs_count, BlueprintAttribute* inputs, uint outputs_count, BlueprintAttribute* outputs) {
			auto& pau = *(PointerAndUint*)inputs[0].data;
			if (pau.p && pau.u)
			{
				if (auto ins = (BlueprintInstancePtr)pau.p; ins)
				{
					if (auto g = ins->find_group(pau.u); g)
					{
						std::vector<voidptr> input_args;
						std::vector<voidptr> output_args;
						if (g->input_node)
						{
							auto n = g->input_node->outputs.size();
							for (auto i = 0; i < n; i++)
								input_args.push_back(inputs[i + 1].data);
						}
						if (g->output_node)
						{
							auto n = g->output_node->inputs.size();
							for (auto i = 0; i < n; i++)
								output_args.push_back(outputs[i].data);
						}
						if (auto tg = ins->find_group(pau.u); tg)
							ins->call(tg, input_args.data(), output_args.data());
					}
				}
			}
		};

		ret->parent = parent;
		if (parent)
		{
			parent->children.push_back(ret);
			ret->depth = parent->depth + 1;
		}
		group->nodes.emplace_back(ret);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	void BlueprintPrivate::remove_node(BlueprintNodePtr node, bool recursively)
	{
		auto group = node->group;
		auto parent = node->parent;
		assert(group->blueprint == this);

		if (node->is_block)
		{
			if (recursively)
			{
				std::vector<BlueprintNodePtr> to_remove_nodes;
				for (auto c : node->children)
					to_remove_nodes.push_back(c);
				for (auto n : to_remove_nodes)
					remove_node(n, true);
			}
			else
			{
				for (auto c : node->children)
				{
					c->parent = parent;
					update_depth(c);
					parent->children.push_back(c);
				}
			}
		}

		if (parent)
		{
			for (auto it = parent->children.begin(); it != parent->children.end(); it++)
			{
				if (*it == node)
				{
					parent->children.erase(it);
					break;
				}
			}
		}
		remove_node_links(node);

		if (auto debugger = BlueprintDebugger::current(); debugger)
			debugger->remove_break_node(node);
		for (auto it = group->nodes.begin(); it != group->nodes.end(); it++)
		{
			if (it->get() == node)
			{
				group->nodes.erase(it);
				break;
			}
		}

		clear_invalid_links(group);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	void BlueprintPrivate::set_nodes_parent(const std::vector<BlueprintNodePtr> _nodes, BlueprintNodePtr new_parent)
	{
		if (_nodes.empty())
			return;

		auto group = _nodes.front()->group;
		assert(group && group->blueprint == this && group == new_parent->group);
		for (auto& n : _nodes)
			assert(group == n->group);

		std::vector<BlueprintNodePtr> nodes;
		for (auto _n : _nodes)
			blueprint_form_top_list(nodes, _n);
		for (auto n : nodes)
		{
			if (n->contains(new_parent))
				return;
		}

		for (auto n : nodes)
		{
			auto old_parent = n->parent;
			for (auto it = old_parent->children.begin(); it != old_parent->children.end(); it++)
			{
				if (*it == n)
				{
					old_parent->children.erase(it);
					break;
				}
			}

			n->parent = new_parent;
			update_depth(n);
			new_parent->children.push_back(n);
		}

		clear_invalid_links(group);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	static void update_node_output_types(BlueprintNodePtr n)
	{
		std::deque<BlueprintNodePtr> input_changed_nodes;
		input_changed_nodes.push_back(n);
		while (!input_changed_nodes.empty())
		{
			auto n = input_changed_nodes.front();
			input_changed_nodes.pop_front();

			if (n->change_structure_callback)
			{
				BlueprintNodeStructureChangeInfo info;
				info.reason = BlueprintNodeInputTypesChanged;
				info.input_types.resize(n->inputs.size());
				info.output_types.resize(n->outputs.size());
				for (auto i = 0; i < info.input_types.size(); i++)
					info.input_types[i] = n->inputs[i]->type;
				for (auto i = 0; i < info.output_types.size(); i++)
					info.output_types[i] = n->outputs[i]->type;

				auto ok = n->change_structure_callback(info);
				assert(ok);

				for (auto i = 0; i < info.output_types.size(); i++)
				{
					auto slot = n->outputs[i].get();
					if (slot->type != info.output_types[i])
					{
						change_slot_type(slot, blueprint_allow_type(slot->allowed_types, info.output_types[i]) ? info.output_types[i] : nullptr);

						for (auto& l : n->group->links)
						{
							if (l->from_slot == slot)
							{
								change_slot_type(l->from_slot, blueprint_allow_type(l->from_slot->allowed_types, slot->type) ? slot->type : nullptr);

								auto node = l->to_slot->node;
								if (std::find(input_changed_nodes.begin(), input_changed_nodes.end(), node) == input_changed_nodes.end())
									input_changed_nodes.push_back(node);
							}
						}
					}
				}
			}
		}
	}
	bool BlueprintPrivate::change_node_structure(BlueprintNodePtr node, const std::string& new_template_string, const std::vector<TypeInfo*>& new_input_types)
	{
		auto group = node->group;
		assert(group->blueprint == this);

		if (!new_template_string.empty())
		{
			if (!node->change_structure_callback)
			{
				printf("blueprint change_node_structure: node must have 'change_structure_callback'\n");
				return false;
			}

			BlueprintNodeStructureChangeInfo info;
			info.reason = BlueprintNodeTemplateChanged;
			info.template_string = new_template_string;
			info.new_function = (BlueprintNodeFunction)INVALID_POINTER;
			info.new_loop_function = (BlueprintNodeLoopFunction)INVALID_POINTER;
			info.new_begin_block_function = (BlueprintNodeBeginBlockFunction)INVALID_POINTER;
			info.new_end_block_function = (BlueprintNodeEndBlockFunction)INVALID_POINTER;
			if (!node->change_structure_callback(info))
			{
				printf("blueprint change_node_structure: change_structure_callback failed\n");
				return false;
			}

			for (auto& d : info.new_inputs)
			{
				if (!d.name_hash)
					d.name_hash = sh(d.name.c_str());
				d.flags = d.flags | BlueprintSlotFlagInput;
			}
			for (auto& d : info.new_outputs)
			{
				if (!d.name_hash)
					d.name_hash = sh(d.name.c_str());
				d.flags = d.flags | BlueprintSlotFlagOutput;
			}

			for (auto it = node->inputs.begin(); it != node->inputs.end(); )
			{
				auto input = it->get();
				auto found = false;
				for (auto& i : info.new_inputs)
				{
					if (i.name_hash == input->name_hash)
					{
						if ((i.allowed_types.empty() && input->type == nullptr) ||
							i.allowed_types.front() == input->type)
						{
							input->allowed_types = i.allowed_types;
							i.flags = BlueprintSlotFlagNone;
							found = true;
							break;
						}
					}
				}

				if (!found)
				{
					if (!input->linked_slots.empty())
					{
						auto link = group->find_link(input->linked_slots.front(), input);
						if (link)
							remove_link(link);
					}
					it = node->inputs.erase(it);
				}
				else
					it++;
			}

			for (auto i = 0; i < info.new_inputs.size(); i++)
			{
				auto& d = info.new_inputs[i];
				if (d.flags)
				{
					auto pos = -1;
					if (i > 0)
					{
						auto prev = info.new_inputs[i - 1].name_hash;
						for (auto j = 0; j < node->inputs.size(); j++)
						{
							if (node->inputs[j]->name_hash == prev)
							{
								pos = j + 1;
								break;
							}
						}
					}
					d.flags = d.flags | BlueprintSlotFlagInput;
					create_slot(node, d, pos);
				}
			}

			for (auto it = node->outputs.begin(); it != node->outputs.end(); )
			{
				auto output = it->get();
				auto found = false;
				for (auto& o : info.new_outputs)
				{
					if (o.name_hash == output->name_hash)
					{
						if ((o.allowed_types.empty() && output->type == nullptr) ||
							o.allowed_types.front() == output->type)
						{
							output->allowed_types = o.allowed_types;
							o.flags = BlueprintSlotFlagNone;
							found = true;
							break;
						}
					}
				}

				if (!found)
				{
					for (auto slot : output->linked_slots)
					{
						auto link = group->find_link(output, slot);
						if (link)
							remove_link(link);
					}
					it = node->outputs.erase(it);
				}
				else
					it++;
			}

			for (auto i = 0; i < info.new_outputs.size(); i++)
			{
				auto& d = info.new_outputs[i];
				if (d.flags)
				{
					auto pos = -1;
					if (i > 0)
					{
						auto prev = info.new_outputs[i - 1].name_hash;
						for (auto j = 0; j < node->outputs.size(); j++)
						{
							if (node->outputs[j]->name_hash == prev)
							{
								pos = j + 1;
								break;
							}
						}
					}
					d.flags = d.flags | BlueprintSlotFlagOutput;
					create_slot(node, d, pos);
				}
			}

			if (info.new_function != INVALID_POINTER)
				node->function = info.new_function;
			if (info.new_loop_function != INVALID_POINTER)
				node->loop_function = info.new_loop_function;
			if (info.new_begin_block_function != INVALID_POINTER)
				node->begin_block_function = info.new_begin_block_function;
			if (info.new_end_block_function != INVALID_POINTER)
				node->end_block_function = info.new_end_block_function;

			node->template_string = info.template_string;
		}

		if (!new_input_types.empty())
		{
			if (new_input_types.size() != node->inputs.size())
			{
				printf("blueprint change_node_structure: new input types must as much as node's inputs\n");
				return false;
			}

			auto ok = true;
			for (auto i = 0; i < node->inputs.size(); i++)
			{
				auto input = node->inputs[i].get();
				if (!blueprint_allow_type(input->allowed_types, new_input_types[i]))
				{
					ok = false;
					break;
				}
			}

			if (!ok)
			{
				printf("blueprint change_node_structure: new input types must be allowed\n");
				return false;
			}

			for (auto i = 0; i < node->inputs.size(); i++)
				change_slot_type(node->inputs[i].get(), new_input_types[i]);
			update_node_output_types(node);
		}

		clear_invalid_links(group);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return true;
	}

	static uint get_variable_node_desc(BlueprintNodePtr nn, uint* out_name, uint* out_location, uint* out_property, uint node_type = 0)
	{
		auto n = (BlueprintNode*)nn;
		if (n)
			node_type = n->name_hash;
		switch (node_type)
		{
		case "Variable"_h:
		case "Set Variable"_h:
		case "Add Assign"_h:
		case "Subtract Assign"_h:
		case "Multiple Assign"_h:
		case "Divide Assign"_h:
		case "Or Assign"_h:
		case "And Assign"_h:
			if (n)
			{
				if (out_name)
					*out_name = *(uint*)n->inputs[0]->data;
				if (out_location)
					*out_location = *(uint*)n->inputs[1]->data;
				if (out_property)
					*out_property = 0;
			}
			return 2;
		case "Get Property"_h:
		case "Set Property"_h:
			if (n)
			{
				if (out_name)
					*out_name = *(uint*)n->inputs[0]->data;
				if (out_location)
					*out_location = *(uint*)n->inputs[1]->data;
				if (out_property)
					*out_property = *(uint*)n->inputs[2]->data;
			}
			return 3;
		case "Array Size"_h:
		case "Array Clear"_h:
			if (n)
			{
				if (out_name)
					*out_name = *(uint*)n->inputs[0]->data;
				if (out_location)
					*out_location = *(uint*)n->inputs[1]->data;
				if (out_property)
					*out_property = 0;
			}
			return 2;
		case "Array Get Item"_h:
		case "Array Set Item"_h:
			if (n)
			{
				if (out_name)
					*out_name = *(uint*)n->inputs[0]->data;
				if (out_location)
					*out_location = *(uint*)n->inputs[1]->data;
				if (out_property)
					*out_property = 0;
			}
			return 2;
		case "Array Get Item Property"_h:
		case "Array Set Item Property"_h:
			if (n)
			{
				if (out_name)
					*out_name = *(uint*)n->inputs[0]->data;
				if (out_location)
					*out_location = *(uint*)n->inputs[1]->data;
				if (out_property)
					*out_property = *(uint*)n->inputs[2]->data;
			}
			return 3;
		case "Array Add Item"_h:
		case "Array Emplace Item"_h:
			if (n)
			{
				if (out_name)
					*out_name = *(uint*)n->inputs[0]->data;
				if (out_location)
					*out_location = *(uint*)n->inputs[1]->data;
				if (out_property)
					*out_property = 0;
			}
			return 2;
		}
		return 0;
	}

	static TypeInfo* var_type_from_variable_node(BlueprintNodePtr n)
	{
		switch (n->name_hash)
		{
		case "Variable"_h:
			return n->outputs[0]->type;
		case "Set Variable"_h:
			return n->inputs[2]->type;
		case "Get Property"_h:
			return n->outputs[0]->type;
		case "Set Property"_h:
			return n->inputs[3]->type;
		case "Array Get Item"_h:
			return n->outputs[0]->type;
		case "Array Set Item"_h:
			return n->inputs[3]->type;
		case "Array Get Item Property"_h:
			return n->outputs[0]->type;
		case "Array Set Item Property"_h:
			return n->inputs[4]->type;
		case "Array Add Item"_h:
			return n->inputs[2]->type;
		}
		return nullptr;
	}

	bool BlueprintPrivate::change_references(BlueprintGroupPtr group, uint old_name, uint old_location, uint old_property, uint new_name, uint new_location, uint new_property)
	{
		auto changed = false;
		auto process_group = [&](BlueprintGroupPtr g) {
			std::vector<BlueprintNodePtr> nodes;
			for (auto& n : g->nodes)
				nodes.push_back(n.get());
			for (auto n : nodes)
			{
				if (blueprint_is_variable_node(n->name_hash))
				{
					if ((old_name  == 0 || *(uint*)n->inputs[0]->data == old_name) &&
						*(uint*)n->inputs[1]->data == old_location)
					{
						uint var_name, var_location, property_name;
						auto desc_n = get_variable_node_desc(n, &var_name, &var_location, &property_name);

						struct StagingValue
						{
							uint name;
							TypeInfo* old_type;
							std::string value;
						};

						std::vector<StagingValue> staging_values;
						for (auto i = desc_n; i < n->inputs.size(); i++)
						{
							auto& input = n->inputs[i];
							if (input->linked_slots.empty())
							{
								if (auto value_str = input->type->serialize(input->data); value_str != input->default_value)
								{
									auto& v = staging_values.emplace_back();
									v.name = input->name_hash;
									v.old_type = input->type;
									v.value = value_str;
								}
							}
						}

						std::vector<BlueprintLinkPtr> relevant_links;
						for (auto& src_l : g->links)
						{
							if (n == src_l->from_slot->node || n == src_l->to_slot->node)
								relevant_links.push_back(src_l.get());
						}
						std::sort(relevant_links.begin(), relevant_links.end(), [](const auto a, const auto b) {
							return a->from_slot->node->degree < b->from_slot->node->degree;
						});

						struct StagingLink
						{
							uint from_node;
							uint from_slot;
							uint to_node;
							uint to_slot;
						};
						std::vector<StagingLink> staging_links;
						for (auto src_l : relevant_links)
						{
							auto& l = staging_links.emplace_back();
							l.from_node = src_l->from_slot->node->object_id;
							l.from_slot = src_l->from_slot->name_hash;
							l.to_node = src_l->to_slot->node->object_id;
							l.to_slot = src_l->to_slot->name_hash;
						}

						auto old_id = n->object_id;
						auto node_type = n->name_hash;
						auto old_var_type = var_type_from_variable_node(n);
						var_name = new_name ? new_name : var_name;
						var_location = new_location ? new_location : var_location;
						property_name = new_property ? new_property : property_name;
						auto parent = n->parent;
						auto position = n->position;
						remove_node(n, false);
						if (auto new_n = add_variable_node(g, parent, var_name, node_type, var_location, property_name); new_n)
						{
							auto new_var_type = var_type_from_variable_node(new_n);
							if (old_var_type && new_var_type)
							{
								switch (node_type)
								{
									case "Variable"_h:
									case "Set Variable"_h:
										break;
									case "Array Size"_h:
										break;
									case "Array Get Item"_h:
										if (new_var_type->tag == TagU && (old_var_type->tag == TagD || is_pointer(old_var_type->tag)))
										{
											auto ui = new_var_type->retrive_ui();
											for (auto& vi : ui->variables)
											{
												if (vi.type == old_var_type)
												{
													remove_node(new_n, false);
													new_n = add_variable_node(g, parent, var_name, "Array Get Item Property"_h, new_location, vi.name_hash);
													break;
												}
											}
										}
										break;
									case "Array Set Item"_h:
										if (new_var_type->tag == TagU && (old_var_type->tag == TagD || is_pointer(old_var_type->tag)))
										{
											auto ui = new_var_type->retrive_ui();
											for (auto& vi : ui->variables)
											{
												if (vi.type == old_var_type)
												{
													remove_node(new_n, false);
													new_n = add_variable_node(g, parent, var_name, "Array Set Item Property"_h, new_location, vi.name_hash);
												}
											}
										}
										break;
									case "Array Add Item"_h:
										if (new_var_type->tag == TagU && (old_var_type->tag == TagD || is_pointer(old_var_type->tag)))
										{
											auto ui = new_var_type->retrive_ui();
											for (auto& vi : ui->variables)
											{
												if (vi.type == old_var_type)
												{
													remove_node(new_n, false);
													new_n = add_variable_node(g, parent, var_name, "Array Emplace Item"_h, new_location, 0);
													for (auto& src_v : staging_values)
													{
														if (src_v.name == "V"_h)
														{
															src_v.name = vi.name_hash;
															break;
														}
													}
													for (auto& src_l : staging_links)
													{
														if (src_l.to_node == old_id && src_l.to_slot == "V"_h)
														{
															src_l.to_slot = vi.name_hash;
															break;
														}
													}
												}
											}
										}
										break;
								}
							}

							new_n->position = position;

							for (auto& src_v : staging_values)
							{
								if (auto input = new_n->find_input(src_v.name); input && input->type == src_v.old_type)
									input->type->unserialize(src_v.value, input->data);
							}

							for (auto& src_l : staging_links)
							{
								BlueprintNodePtr from_node = nullptr;
								BlueprintNodePtr to_node = nullptr;
								if (src_l.from_node == old_id)
								{
									from_node = new_n;
									if (src_l.from_slot == old_name)
										src_l.from_slot = new_name;
								}
								else
									from_node = g->find_node_by_id(src_l.from_node);
								assert(from_node);
								if (src_l.to_node == old_id)
								{
									to_node = new_n;
									if (src_l.to_slot == old_name)
										src_l.to_slot = new_name;
								}
								else
									to_node = g->find_node_by_id(src_l.to_node);
								assert(to_node);

								auto from_slot = from_node->find_output(src_l.from_slot);
								auto to_slot = to_node->find_input(src_l.to_slot);
								if (from_slot && to_slot)
									add_link(from_slot, to_slot);
							}
						}
						else
							printf("blueprint change_references: cannot add new node\n");

						changed = true;
					}
				}
			}
		};
		if (group)
		{
			assert(group->blueprint == this);
			process_group(group);
		}
		else
		{
			for (auto& g : groups)
				process_group(g.get());
		}
		return changed;
	}

	BlueprintLinkPtr BlueprintPrivate::add_link(BlueprintSlotPtr from_slot, BlueprintSlotPtr to_slot)
	{
		auto group = from_slot->node->group;
		assert(group && group->blueprint == this && group == to_slot->node->group);
		assert(from_slot->flags & BlueprintSlotFlagOutput);
		assert(to_slot->flags & BlueprintSlotFlagInput);

		if (from_slot->node == to_slot->node)
		{
			printf("blueprint add_link: cannot link because from_slot(%s) and to_slot(%s) are from the same node\n", from_slot->name.c_str(), to_slot->name.c_str());
			return nullptr;
		}
		if (!from_slot->node->parent->contains(to_slot->node))
		{
			printf("blueprint add_link: cannot link because to_slot(%s)'s node should comes from from_slot(%s)'s node's parent\n", to_slot->name.c_str(), from_slot->name.c_str());
			return nullptr;
		}

		if (!blueprint_allow_type(to_slot->allowed_types, from_slot->type))
		{
			printf("blueprint add_link: cannot link because from_slot(%s)'s type is not allowed to to_slot(%s)\n", from_slot->name.c_str(), to_slot->name.c_str());
			return nullptr;
		}

		for (auto& l : group->links)
		{
			if (l->to_slot == to_slot)
			{
				remove_link(l.get());
				break;
			}
		}

		if (from_slot->type == TypeInfo::get<BlueprintSignal>() &&
			to_slot->node->name_hash == "Block"_h)
		{
			to_slot->node->display_name = from_slot->name;
			;
		}

		auto ret = new BlueprintLinkPrivate;
		ret->object_id = next_object_id++;
		ret->from_slot = from_slot;
		ret->to_slot = to_slot;
		group->links.emplace_back(ret);

		from_slot->linked_slots.push_back(to_slot);
		to_slot->linked_slots.push_back(from_slot);
		change_slot_type(to_slot, from_slot->type);
		update_node_output_types(to_slot->node);
		clear_invalid_links(group);
		update_degree(to_slot->node);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	void BlueprintPrivate::remove_link(BlueprintLinkPtr link)
	{
		auto group = link->from_slot->node->group;
		assert(group && group->blueprint == this);

		for (auto it = group->links.begin(); it != group->links.end(); it++)
		{
			if (it->get() == link)
			{
				auto from_slot = link->from_slot;
				auto to_node = link->to_slot->node;
				auto to_slot = link->to_slot;
				std::erase_if(from_slot->linked_slots, [&](const auto& slot) {
					return slot == to_slot;
				});
				std::erase_if(to_slot->linked_slots, [&](const auto& slot) {
					return slot == from_slot;
				});
				group->links.erase(it);
				change_slot_type(to_slot, !to_slot->allowed_types.empty() ? to_slot->allowed_types.front() : nullptr);
				update_node_output_types(to_node);
				clear_invalid_links(group);
				update_degree(to_node);
				break;
			}
		}

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	BlueprintGroupPtr BlueprintPrivate::add_group(const std::string& name)
	{
		auto g = new BlueprintGroupPrivate;
		g->blueprint = this;
		g->object_id = next_object_id++;
		g->name = name;
		g->name_hash = sh(name.c_str());
		groups.emplace_back(g);

		add_block(g, nullptr)->flags = BlueprintNodeFlagBreakTarget;

		auto frame = frames;
		g->structure_changed_frame = frame;
		dirty_frame = frame;

		return g;
	}

	void BlueprintPrivate::remove_group(BlueprintGroupPtr group) 
	{
		assert(group && group->blueprint == this);

		if (groups.size() == 1)
		{
			printf("blueprint remove_group: cannot remove the last group\n");
			return;
		}

		std::vector<BlueprintNodePtr> to_remove_nodes;
		for (auto& g : group->blueprint->groups)
		{
			if (g.get() != group)
			{
				for (auto& n : g->nodes)
				{
					if (n->name_hash == "Call"_h)
					{
						if (*(uint*)n->inputs[0]->data == group->name_hash && *(uint*)n->inputs[1]->data == 0)
							to_remove_nodes.push_back(n.get());
					}
				}
			}
		}
		for (auto n : to_remove_nodes)
			remove_node(n, false);

		for (auto it = groups.begin(); it != groups.end(); it++)
		{
			if (it->get() == group)
			{
				groups.erase(it);
				break;
			}
		}

		auto frame = frames;
		dirty_frame = frame;
	}

	void BlueprintPrivate::alter_group(uint old_name, const std::string& new_name)
	{
		for (auto it = groups.begin(); it != groups.end(); it++)
		{
			if ((*it)->name_hash == old_name)
			{
				(*it)->name = new_name;
				(*it)->name_hash = sh(new_name.c_str());

				auto frame = frames;
				dirty_frame = frame;
				return;
			}
		}
	}

	static void update_group_input_node(BlueprintGroupPtr g)
	{
		auto n = g->find_node("Input"_h);
		vec2 old_position = vec2(0.f);
		std::vector<std::pair<uint, std::vector<BlueprintSlotPtr>>> old_links;
		if (n)
		{
			old_position = n->position;
			for (auto& o : n->outputs)
			{
				if (!o->linked_slots.empty())
					old_links.emplace_back(o->name_hash, o->linked_slots);
			}
			g->blueprint->remove_node(n, false);
		}
		if (g->inputs.empty())
			return;

		std::vector<BlueprintSlotDesc> outputs;
		for (auto& i : g->inputs)
		{
			auto& o = outputs.emplace_back();
			o.name = i.name;
			o.name_hash = i.name_hash;
			o.allowed_types = { i.type };
		}
		n = g->blueprint->add_node(g, nullptr, "Input", BlueprintNodeFlagNone, "", {}, outputs, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
		n->position = old_position;
		for (auto& l : old_links)
		{
			if (auto from_slot = n->find_output(l.first); from_slot)
			{
				for (auto& ll : l.second)
					g->blueprint->add_link(from_slot, ll);
			}
		}
	}

	void BlueprintPrivate::add_group_input(BlueprintGroupPtr group, const std::string& name, TypeInfo* type)
	{
		assert(group && group->blueprint == this);

		for (auto& i : group->inputs)
		{
			if (i.name == name)
			{
				printf("blueprint add_group_input: input already exists\n");
				return;
			}
		}

		auto& input = group->inputs.emplace_back();
		input.name = name;
		input.name_hash = sh(name.c_str());
		input.type = type;
		update_group_input_node(group);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	void BlueprintPrivate::remove_group_input(BlueprintGroupPtr group, uint name)
	{
		assert(group && group->blueprint == this);

		for (auto it = group->inputs.begin(); it != group->inputs.end(); it++)
		{
			if (it->name_hash == name)
			{
				group->inputs.erase(it);
				break;
			}
		}
		update_group_input_node(group);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	void BlueprintPrivate::alter_group_input(BlueprintGroupPtr group, uint old_name, const std::string& new_name, TypeInfo* new_type)
	{
		assert(group && group->blueprint == this);

		for (auto it = group->inputs.begin(); it != group->inputs.end(); ++it)
		{
			if (it->name_hash == old_name)
			{
				auto input_node = group->find_node("Input"_h);
				auto slot = input_node->find_output(it->name_hash);
				if (!new_name.empty())
				{
					if (it->name != new_name)
					{
						it->name = new_name;
						it->name_hash = sh(new_name.c_str());
						slot->name = it->name;
						slot->name_hash = it->name_hash;
					}
				}
				if (new_type)
				{
					if (it->type != new_type)
					{
						it->type = new_type;
						for (auto& l : group->links)
						{
							if (l->from_slot == slot)
							{
								if (blueprint_allow_type(l->to_slot->allowed_types, new_type))
									change_slot_type(l->to_slot, new_type);
								else
									change_slot_type(l->to_slot, nullptr);
							}
						}
					}
					clear_invalid_links(group);
				}
				update_group_input_node(group);
			}
		}
	}

	static void update_group_output_node(BlueprintGroupPtr g)
	{
		auto n = g->find_node("Output"_h);
		vec2 old_position = vec2(500.f, 0.f);
		std::vector<std::pair<BlueprintSlotPtr, uint>> old_links;
		if (n)
		{
			old_position = n->position;
			for (auto& i : n->inputs)
			{
				if (!i->linked_slots.empty())
					old_links.emplace_back(i->linked_slots[0], i->name_hash);
			}
			g->blueprint->remove_node(n, false);
		}
		if (g->outputs.empty())
			return;

		std::vector<BlueprintSlotDesc> inputs;
		for (auto& o : g->outputs)
		{
			auto& i = inputs.emplace_back();
			i.name = o.name;
			i.name_hash = o.name_hash;
			i.allowed_types = { o.type };
		}
		n = g->blueprint->add_node(g, nullptr, "Output", BlueprintNodeFlagNone, "", inputs, {}, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
		n->position = old_position;
		for (auto& l : old_links)
		{
			auto to_slot = n->find_input(l.second);
			if (to_slot)
				g->blueprint->add_link(l.first, to_slot);
		}
	}

	void BlueprintPrivate::add_group_output(BlueprintGroupPtr group, const std::string& name, TypeInfo* type)
	{
		assert(group && group->blueprint == this);

		for (auto& o : group->outputs)
		{
			if (o.name == name)
			{
				printf("blueprint add_group_output: output already exists\n");
				return;
			}
		}

		auto& output = group->outputs.emplace_back();
		output.name = name;
		output.name_hash = sh(name.c_str());
		output.type = type;
		update_group_output_node(group);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	void BlueprintPrivate::remove_group_output(BlueprintGroupPtr group, uint name)
	{
		assert(group && group->blueprint == this);

		for (auto it = group->outputs.begin(); it != group->outputs.end(); it++)
		{
			if (it->name_hash == name)
			{
				group->outputs.erase(it);
				break;
			}
		}
		update_group_output_node(group);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	void BlueprintPrivate::alter_group_output(BlueprintGroupPtr group, uint old_name, const std::string& new_name, TypeInfo* new_type)
	{
		assert(group && group->blueprint == this);

		for (auto it = group->outputs.begin(); it != group->outputs.end(); ++it)
		{
			if (it->name_hash == old_name)
			{
				auto output_node = group->find_node("Output"_h);
				auto slot = output_node->find_input(it->name_hash);
				if (!new_name.empty())
				{
					if (it->name != new_name)
					{
						it->name = new_name;
						it->name_hash = sh(new_name.c_str());
						slot->name = it->name;
						slot->name_hash = it->name_hash;
					}
				}
				if (new_type)
				{
					if (it->type != new_type)
					{
						it->type = new_type;
						for (auto& l : group->links)
						{
							if (l->to_slot == slot)
							{
								if (blueprint_allow_type(l->from_slot->allowed_types, new_type))
									change_slot_type(l->from_slot, new_type);
								else
									change_slot_type(l->from_slot, nullptr);
							}
						}
						clear_invalid_links(group);
					}
				}
				update_group_output_node(group);
			}
		}
	}

	void BlueprintPrivate::load(const std::filesystem::path& path, bool load_typeinfos)
	{
		filename = Path::get(path);
		if (!std::filesystem::exists(filename))
			return;

		pugi::xml_document doc;
		pugi::xml_node doc_root;

		if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("blueprint"))
		{
			wprintf(L"blueprint does not exist or wrong format: %s\n", path.c_str());
			return;
		}

		for (auto n_dependency : doc_root.child("dependencies"))
		{
			std::filesystem::path path(n_dependency.attribute("v").value());
			if (std::filesystem::exists(path))
				Blueprint::get(path, true);
		}

		auto read_ti = [&](pugi::xml_attribute a) {
			auto sp = SUS::to_string_vector(SUS::split(a.value(), '@'));
			TypeTag tag;
			TypeInfo::unserialize_t(sp[0], tag);
			return TypeInfo::get(tag, sp[1]);
		};

		for (auto n_enum : doc_root.child("enums"))
		{
			std::string enum_name = n_enum.attribute("name").value();
			std::vector<BlueprintEnumItem> items;
			for (auto n_item : n_enum.child("items"))
			{
				auto& i = items.emplace_back();
				i.name = n_item.attribute("name").value();
				i.name_hash = sh(i.name.c_str());
				i.value = n_item.attribute("value").as_int();
			}
			if (load_typeinfos)
				add_enum(enum_name, items);
			else
			{
				auto& e = enums.emplace_back();
				e.name = enum_name;
				e.name_hash = sh(name.c_str());
				e.items = items;
			}
		}
		for (auto n_struct : doc_root.child("structs"))
		{
			std::string struct_name = n_struct.attribute("name").value();
			std::vector<BlueprintStructVariable> variables;
			for (auto n_variable : n_struct.child("variables"))
			{
				auto& v = variables.emplace_back();
				v.name = n_variable.attribute("name").value();
				v.name_hash = sh(v.name.c_str());
				v.type = read_ti(n_variable.attribute("type"));
				v.default_value = n_variable.attribute("default_value").value();
			}
			if (load_typeinfos)
				add_struct(struct_name, variables);
			else
			{
				auto& s = structs.emplace_back();
				s.name = struct_name;
				s.name_hash = sh(name.c_str());
				s.variables = variables;
			}
		}
		for (auto n_variable : doc_root.child("variables"))
		{
			auto type = read_ti(n_variable.attribute("type"));
			auto data = add_variable(nullptr, n_variable.attribute("name").value(), type);
			type->unserialize(n_variable.attribute("value").value(), data);
		}
		for (auto n_group : doc_root.child("groups"))
		{
			auto g = add_group(n_group.attribute("name").value());

			if (auto a = n_group.attribute("trigger_message"); a)
				g->trigger_message = a.value();

			for (auto n_variable : n_group.child("variables"))
			{
				auto type = read_ti(n_variable.attribute("type"));
				auto data = add_variable(g, n_variable.attribute("name").value(), type);
				type->unserialize(n_variable.attribute("value").value(), data);
			}
			for (auto n_input : n_group.child("inputs"))
				add_group_input(g, n_input.attribute("name").value(), read_ti(n_input.attribute("type")));
			for (auto n_output : n_group.child("outputs"))
				add_group_output(g, n_output.attribute("name").value(), read_ti(n_output.attribute("type")));

			for (auto n_split : n_group.child("splits"))
				g->splits.push_back(n_split.attribute("v").as_float());

			std::map<uint, BlueprintNodePtr> node_map;

			for (auto n_node : n_group.child("nodes"))
			{
				std::string node_name = n_node.attribute("name").value();
				auto sp = SUS::to_string_vector(SUS::split(node_name, '#'));
				node_name = sp[0];
				auto node_template = sp.size() > 1 ? sp[1] : "";
				auto node_name_hash = sh(node_name.c_str());
				auto parent_id = n_node.attribute("parent_id").as_uint();
				auto object_id = n_node.attribute("object_id").as_uint();
				auto position = s2t<2, float>(n_node.attribute("position").value());
				BlueprintNodePtr parent = nullptr;
				if (parent_id != 0)
				{
					if (auto it = node_map.find(parent_id); it != node_map.end())
						parent = it->second;
					else
					{
						printf("add node: cannot find parent with id %d\n", parent_id);
						printf("in bp: %s, group: %s\n", filename.string().c_str(), g->name.c_str());
						continue;
					}
				}
				auto read_input = [&](BlueprintNodePtr n, pugi::xml_node n_input) {
					auto name = n_input.attribute("name").value();
					auto i = n->find_input(name);
					if (i)
					{
						if (auto a_type = n_input.attribute("type"); a_type)
						{
							change_slot_type(i, read_ti(a_type));
							update_node_output_types(n);
							clear_invalid_links(g);
						}
						if (i->type->tag != TagU)
							i->type->unserialize(n_input.attribute("value").value(), i->data);
					}
					else
					{
						printf("add node: read input: cannot find input: %s in node %d\n", name, n->object_id);
						printf(" in bp: %s, group: %s\n", filename.string().c_str(), g->name.c_str());

						BlueprintInvalidInput ii;
						ii.reason = BlueprintInvalidName;
						ii.node = n->object_id;
						ii.name = name;
						ii.value = n_input.attribute("value").value();
						g->invalid_inputs.push_back(ii);
					}
				};
				if (node_name == "Block")
				{
					auto n = add_block(g, parent);
					node_map[object_id] = n;
					n->position = position;
				}
				else if (node_name == "Input")
				{
					if (auto n = g->find_node("Input"_h); n)
					{
						node_map[object_id] = n;
						n->position = position;
					}
				}
				else if (node_name == "Output")
				{
					if (auto n = g->find_node("Output"_h); n)
					{
						node_map[object_id] = n;
						n->position = position;
					}
				}
				else if (blueprint_is_variable_node(node_name_hash))
				{
					std::vector<pugi::xml_node> other_inputs;
					auto desc_n = get_variable_node_desc(nullptr, nullptr, nullptr, nullptr, node_name_hash);
					uint var_name = 0, var_location = 0, property_name = 0;
					auto idx = 0;
					for (auto n_input : n_node.child("inputs"))
					{
						if (idx < desc_n)
						{
							std::string n_input_name = n_input.attribute("name").value();
							if (n_input_name == "Name")
								var_name = n_input.attribute("value").as_uint();
							else if (n_input_name == "Location")
								var_location = n_input.attribute("value").as_uint();
							else if (n_input_name == "Property")
								property_name = n_input.attribute("value").as_uint();
							else
								other_inputs.push_back(n_input);
						}
						else
							other_inputs.push_back(n_input);
						idx++;
					}
					if (auto n = add_variable_node(g, parent, var_name, node_name_hash, var_location, property_name); n)
					{
						for (auto n_input : other_inputs)
							read_input(n, n_input);
						node_map[object_id] = n;
						n->position = position;
					}
					else
					{
						printf(" node with id %u cannot not be added\n", object_id);
						printf(" in bp: %s, group: %s\n", filename.string().c_str(), g->name.c_str());
					}
				}
				else if (node_name == "Call")
				{
					std::vector<pugi::xml_node> other_inputs;
					uint name = 0;
					uint location_name = 0;
					for (auto n_input : n_node.child("inputs"))
					{
						std::string n_input_name = n_input.attribute("name").value();
						if (n_input_name == "Name")
							name = n_input.attribute("value").as_uint();
						else if (n_input_name == "Location")
							location_name = n_input.attribute("value").as_uint();
						else
							other_inputs.push_back(n_input);
					}
					auto n = add_call_node(g, parent, name, location_name);
					if (n)
					{
						for (auto n_input : other_inputs)
							read_input(n, n_input);
						node_map[object_id] = n;
						n->position = position;
					}
					else
					{
						printf(" node with id %u cannot not be added\n", object_id);
						printf(" in bp: %s, group: %s\n", filename.string().c_str(), g->name.c_str());
					}
				}
				else
				{
					if (auto n = add_node(g, parent, sh(node_name.c_str())); n)
					{
						if (n->flags & BlueprintNodeFlagEnableTemplate && !node_template.empty())
						{
							if (!change_node_structure(n, node_template, {}))
							{
								printf("cannot apply template(%s) to node: %s, id: %u\n", node_template.c_str(), node_name.c_str(), object_id);
								printf(" in bp: %s, group: %s\n", filename.string().c_str(), g->name.c_str());
							}
						}
						for (auto n_input : n_node.child("inputs"))
							read_input(n, n_input);
						node_map[object_id] = n;
						n->position = position;
					}
					else
					{
						printf("cannot find node template: %s, id: %u\n", node_name.c_str(), object_id);
						printf(" in bp: %s, group: %s\n", filename.string().c_str(), g->name.c_str());
					}
				}
			}
			for (auto n_link : n_group.child("links"))
			{
				uint from_node_id = 0, to_node_id = 0;
				uint from_slot_hash = 0, to_slot_hash = 0;
				std::string from_slot_name, to_slot_name;
				BlueprintNodePtr from_node = nullptr, to_node = nullptr;
				BlueprintSlotPtr from_slot = nullptr, to_slot = nullptr;

				from_node_id = n_link.attribute("from_node").as_uint();
				to_node_id = n_link.attribute("to_node").as_uint();
				from_slot_name = n_link.attribute("from_slot").value();
				to_slot_name = n_link.attribute("to_slot").value();

				if (std::isdigit(from_slot_name[0]))
				{
					from_slot_hash = s2t<uint>(from_slot_name);
					from_slot_name = "";
				}
				else
					from_slot_hash = sh(from_slot_name.c_str());
				if (std::isdigit(to_slot_name[0]))
				{
					to_slot_hash = s2t<uint>(to_slot_name);
					to_slot_name = "";
				}
				else
					to_slot_hash = sh(to_slot_name.c_str());

				if (auto it = node_map.find(from_node_id); it != node_map.end())
				{
					from_node = it->second;
					from_node_id = from_node->object_id;
				}
				else
					printf("link: cannot find node: %u\n", from_node_id);

				if (auto it = node_map.find(to_node_id); it != node_map.end())
				{
					to_node = it->second;
					to_node_id = to_node->object_id;
				}
				else
					printf("link: cannot find node: %u\n", to_node_id);

				auto report_invalid_link = [&](BlueprintInvalidReason reason) {
					BlueprintInvalidLink il;
					il.reason = reason;
					il.from_node = from_node_id;
					il.from_slot = from_slot_hash;
					il.from_slot_name = from_slot_name;
					il.to_node = to_node_id;
					il.to_slot = to_slot_hash;
					il.to_slot_name = to_slot_name;
					g->invalid_links.push_back(il);
				};

				if (!from_node || !to_node)
				{
					printf(" in bp: %s, group: %s\n", filename.string().c_str(), g->name.c_str());

					report_invalid_link(!from_node ? BlueprintInvalidFromNode : BlueprintInvalidToNode);
					continue;
				}

				auto get_node_name = [](BlueprintNodePtr node) {
					auto name = node->display_name.empty() ? node->name : node->display_name;
					if (!node->template_string.empty())
						name += '#' + node->template_string;
					return name;
				};

				if (from_slot_name.empty())
				{
					from_slot = from_node->find_output(from_slot_hash);
					if (!from_slot)
						printf("link: cannot find output: %u in node: %s\n", from_slot_hash, get_node_name(from_node).c_str());
				}
				else
				{
					from_slot = from_node->find_output(from_slot_name);
					if (!from_slot)
						printf("link: cannot find output: %s in node: %s\n", from_slot_name.c_str(), get_node_name(from_node).c_str());
				}

				if (to_slot_name.empty())
				{
					to_slot = to_node->find_input(to_slot_hash);
					if (!to_slot)
						printf("link: cannot find input: %u in node: %s\n", to_slot_hash, get_node_name(to_node).c_str());
				}
				else
				{
					to_slot = to_node->find_input(to_slot_name);
					if (!to_slot)
						printf("link: cannot find input: %s in node: %s\n", to_slot_name.c_str(), get_node_name(to_node).c_str());
				}

				if (!from_slot || !to_slot)
				{
					printf(" in bp: %s, group: %s\n", filename.string().c_str(), g->name.c_str());

					report_invalid_link(!from_slot ? BlueprintInvalidFromSlot : BlueprintInvalidToSlot);
					continue;
				}

				if (!add_link(from_slot, to_slot))
					printf(" in bp: %s, group: %s\n", filename.string().c_str(), g->name.c_str());
			}
		}

		name = filename.filename().stem().string();
		name_hash = sh(name.c_str());
	}

	void BlueprintPrivate::save(const std::filesystem::path& path)
	{
		pugi::xml_document doc;

		auto write_ti = [&](TypeInfo* ti, pugi::xml_attribute a) {
			a.set_value((TypeInfo::serialize_t(ti->tag) + '@' + ti->name).c_str());
		};

		auto doc_root = doc.append_child("blueprint");
		std::vector<BlueprintPtr> dependencies;
		for (auto& g : groups)
		{
			for (auto& n : g->nodes)
			{
				if (blueprint_is_variable_node(n->name_hash))
				{
					auto name = *(uint*)n->inputs[0]->data;
					auto location = *(uint*)n->inputs[1]->data;
					if (location != 0)
					{
						auto bp = Blueprint::get(location);
						if (bp)
						{
							if (std::find(dependencies.begin(), dependencies.end(), bp) == dependencies.end())
								dependencies.push_back(bp);
						}
					}
				}
			}
		}
		if (!dependencies.empty())
		{
			auto n_dependencies = doc_root.append_child("dependencies");
			for (auto bp : dependencies)
			{
				auto n_dependency = n_dependencies.append_child("dependency");
				n_dependency.append_attribute("v").set_value(bp->filename.string().c_str());
			}
		}
		if (!enums.empty())
		{
			auto n_enums = doc_root.append_child("enums");
			for (auto& e : enums)
			{
				auto n_enum = n_enums.append_child("enum");
				n_enum.append_attribute("name").set_value(e.name.c_str());
				auto n_items = n_enum.append_child("items");
				for (auto& i : e.items)
				{
					auto n_item = n_items.append_child("item");
					n_item.append_attribute("name").set_value(i.name.c_str());
					n_item.append_attribute("value").set_value(i.value);
				}
			}
		}
		if (!structs.empty())
		{
			auto n_structs = doc_root.append_child("structs");
			for (auto& s : structs)
			{
				auto n_struct = n_structs.append_child("struct");
				n_struct.append_attribute("name").set_value(s.name.c_str());
				auto n_variables = n_struct.append_child("variables");
				for (auto& v : s.variables)
				{
					auto n_variable = n_variables.append_child("variable");
					n_variable.append_attribute("name").set_value(v.name.c_str());
					write_ti(v.type, n_variable.append_attribute("type"));
					n_variable.append_attribute("default_value").set_value(v.default_value.c_str());
				}
			}
		}
		if (!variables.empty())
		{
			auto n_variables = doc_root.append_child("variables");
			for (auto& v : variables)
			{
				auto n_variable = n_variables.append_child("variable");
				n_variable.append_attribute("name").set_value(v.name.c_str());
				write_ti(v.type, n_variable.append_attribute("type"));
				if (!is_pointer(v.type->tag) && !is_vector(v.type->tag))
					n_variable.append_attribute("value").set_value(v.type->serialize(v.data).c_str());
			}
		}
		auto n_groups = doc_root.append_child("groups");
		std::vector<std::pair<BlueprintGroupPtr, int>> sorted_groups(groups.size());
		for (auto i = 0; i < groups.size(); i++)
			sorted_groups[i] = std::make_pair(groups[i].get(), -1);
		std::function<void(uint)> get_group_rank;
		get_group_rank = [&](uint idx) {
			if (sorted_groups[idx].second == -1)
			{
				int rank = 0;
				for (auto& n : sorted_groups[idx].first->nodes)
				{
					if (n->name_hash == "Call"_h)
					{
						auto name = *(uint*)n->inputs[0]->data;
						auto location = *(uint*)n->inputs[1]->data;
						if (location == 0)
						{
							for (auto i = 0; i < sorted_groups.size(); i++)
							{
								if (sorted_groups[i].first->name_hash == name)
								{
									get_group_rank(i);
									rank = max(rank, sorted_groups[i].second + 1);
									break;
								}
							}
						}
					}
				}
				sorted_groups[idx].second = rank;
			}
		};
		for (auto i = 0; i < sorted_groups.size(); i++)
			get_group_rank(i);
		std::sort(sorted_groups.begin(), sorted_groups.end(), [](const auto& a, const auto& b) {
			return a.second < b.second;
		});
		for (auto& g : sorted_groups)
		{
			auto n_group = n_groups.append_child("group");
			n_group.append_attribute("object_id").set_value(g.first->object_id);
			n_group.append_attribute("name").set_value(g.first->name.c_str());
			if (!g.first->trigger_message.empty())
				n_group.append_attribute("trigger_message").set_value(g.first->trigger_message.c_str());
			if (!g.first->variables.empty())
			{
				auto n_variables = n_group.append_child("variables");
				for (auto& v : g.first->variables)
				{
					if (v.name.starts_with("loop_index"))
						continue;
					auto n_variable = n_variables.append_child("variable");
					n_variable.append_attribute("name").set_value(v.name.c_str());
					write_ti(v.type, n_variable.append_attribute("type"));
					n_variable.append_attribute("value").set_value(v.type->serialize(v.data).c_str());
				}
			}
			if (!g.first->inputs.empty())
			{
				auto n_inputs = n_group.append_child("inputs");
				for (auto& v : g.first->inputs)
				{
					auto n_input = n_inputs.append_child("input");
					n_input.append_attribute("name").set_value(v.name.c_str());
					write_ti(v.type, n_input.append_attribute("type"));
				}
			}
			if (!g.first->outputs.empty())
			{
				auto n_outputs = n_group.append_child("outputs");
				for (auto& v : g.first->outputs)
				{
					auto n_output = n_outputs.append_child("output");
					n_output.append_attribute("name").set_value(v.name.c_str());
					write_ti(v.type, n_output.append_attribute("type"));
				}
			}

			if (!g.first->splits.empty())
			{
				auto n_splits = n_group.append_child("splits");
				for (auto s : g.first->splits)
				{
					auto n_split = n_splits.append_child("split");
					n_split.append_attribute("v").set_value(s);
				}
			}

			std::vector<BlueprintNodePtr> sorted_nodes;
			std::function<void(BlueprintNodePtr)> tranverse_node;
			tranverse_node = [&](BlueprintNodePtr n) {
				sorted_nodes.push_back(n);
				for (auto c : n->children)
					tranverse_node(c);
			};
			tranverse_node(g.first->nodes.front().get());
			auto n_nodes = n_group.append_child("nodes");
			for (auto n : sorted_nodes)
			{
				if (n == g.first->nodes.front().get())
					continue;
				if (n->name == "Block")
				{
					auto n_node = n_nodes.append_child("node");
					n_node.append_attribute("object_id").set_value(n->object_id);
					if (n->parent->parent)
						n_node.append_attribute("parent_id").set_value(n->parent->object_id);
					n_node.append_attribute("name").set_value(n->name.c_str());
					n_node.append_attribute("position").set_value(str(n->position).c_str());
					continue;
				}
				if (n->name == "Input")
				{
					auto n_node = n_nodes.append_child("node");
					n_node.append_attribute("object_id").set_value(n->object_id);
					n_node.append_attribute("name").set_value(n->name.c_str());
					n_node.append_attribute("position").set_value(str(n->position).c_str());
					continue; // only record position of the Input node
				}
				if (n->name == "Output")
				{
					auto n_node = n_nodes.append_child("node");
					n_node.append_attribute("object_id").set_value(n->object_id);
					n_node.append_attribute("name").set_value(n->name.c_str());
					n_node.append_attribute("position").set_value(str(n->position).c_str());
					continue; // only record position of the Output node
				}

				auto n_node = n_nodes.append_child("node");
				n_node.append_attribute("object_id").set_value(n->object_id);
				if (n->parent->parent)
					n_node.append_attribute("parent_id").set_value(n->parent->object_id);
				auto node_name = n->name;
				if (!n->template_string.empty())
					node_name += '#' + n->template_string;
				n_node.append_attribute("name").set_value(node_name.c_str());
				n_node.append_attribute("position").set_value(str(n->position).c_str());

				pugi::xml_node n_inputs;
				for (auto& i : n->inputs)
				{
					if (i->linked_slots.empty())
					{
						pugi::xml_node n_input;
						if (!i->allowed_types.empty() && i->type && i->type != i->allowed_types.front())
						{
							if (!n_input)
							{
								if (!n_inputs)
									n_inputs = n_node.append_child("inputs");
								n_input = n_inputs.append_child("input");
							}
							write_ti(i->type, n_input.append_attribute("type"));
						}
						if (i->type->tag != TagU && i->data)
						{
							if (auto value_str = i->type->serialize(i->data); value_str != i->default_value)
							{
								if (!n_input)
								{
									if (!n_inputs)
										n_inputs = n_node.append_child("inputs");
									n_input = n_inputs.append_child("input");
								}
								n_input.append_attribute("name").set_value(i->name.c_str());
								n_input.append_attribute("value").set_value(value_str.c_str());
							}
						}
					}
				}
			}

			auto n_links = n_group.append_child("links");
			std::vector<BlueprintLinkPtr> sorted_links(g.first->links.size());
			for (auto i = 0; i < sorted_links.size(); i++)
				sorted_links[i] = g.first->links[i].get();
			std::sort(sorted_links.begin(), sorted_links.end(), [](const auto a, const auto b) {
				return a->from_slot->node->degree < b->from_slot->node->degree;
			});
			for (auto l : sorted_links)
			{
				auto n_link = n_links.append_child("link");
				n_link.append_attribute("object_id").set_value(l->object_id);
				n_link.append_attribute("from_node").set_value(l->from_slot->node->object_id);
				n_link.append_attribute("from_slot").set_value(l->from_slot->name.c_str());
				n_link.append_attribute("to_node").set_value(l->to_slot->node->object_id);
				n_link.append_attribute("to_slot").set_value(l->to_slot->name.c_str());
			}
		}

		if (!path.empty())
			filename = path;
		doc.save_file(filename.c_str());
	}

	struct BlueprintCreate : Blueprint::Create
	{
		BlueprintPtr operator()(bool empty) override
		{
			auto ret = new BlueprintPrivate;
			if (!empty)
				ret->add_group("main");
			return ret;
		}
	}Blueprint_create;
	Blueprint::Create& Blueprint::create = Blueprint_create;

	struct BlueprintDestroy : Blueprint::Destroy
	{
		void operator()(BlueprintPtr bp) override
		{
			delete bp;
		}
	}Blueprint_destroy;
	Blueprint::Destroy& Blueprint::destroy = Blueprint_destroy;

	struct BlueprintGet : Blueprint::Get
	{
		BlueprintPtr operator()(const std::filesystem::path& _filename, bool is_static) override
		{
			auto filename = Path::get(_filename);

			for (auto& bp : loaded_blueprints)
			{
				if (bp->filename == filename)
				{
					bp->ref++;
					return bp.get();
				}
			}

			if (!std::filesystem::exists(filename))
			{
				wprintf(L"cannot found blueprint: %s\n", _filename.c_str());
				return nullptr;
			}

			auto ret = new BlueprintPrivate;
			ret->load(filename, true);

			if (is_static)
			{
				ret->is_static = true;
				assert(named_blueprints.find(ret->name_hash) == named_blueprints.end());
				auto ins = BlueprintInstance::create(ret);
				ins->is_static = true;
				named_blueprints[ret->name_hash] = std::make_pair(ret, ins);
			}
			ret->ref = 1;
			loaded_blueprints.emplace_back(ret);
			return ret;
		}

		BlueprintPtr operator()(uint name) override
		{
			auto it = named_blueprints.find(name);
			if (it == named_blueprints.end())
				return nullptr;
			return it->second.first;
		}
	}Blueprint_get;
	Blueprint::Get& Blueprint::get = Blueprint_get;

	struct BlueprintRelease : Blueprint::Release
	{
		void operator()(BlueprintPtr blueprint) override
		{
			if (blueprint->ref == 1)
			{
				std::erase_if(loaded_blueprints, [&](const auto& bp) {
					return bp.get() == blueprint;
				});
			}
			else
				blueprint->ref--;
		}
	}Blueprint_release;
	Blueprint::Release& Blueprint::release = Blueprint_release;

	void BlueprintNodeLibraryPrivate::add_template(const std::string& name, const std::string& display_name, BlueprintNodeFlags flags,
		const std::vector<BlueprintSlotDesc>& inputs, const std::vector<BlueprintSlotDesc>& outputs,
		BlueprintNodeFunction function, BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeChangeStructureCallback change_structure_callback, BlueprintNodePreviewProvider preview_provider)
	{
		auto& t = node_templates.emplace_back();
		t.library = this;
		t.name = name;
		t.name_hash = sh(name.c_str());
		t.flags = flags;
		t.display_name = display_name;
		t.inputs = inputs;
		for (auto& i : t.inputs)
			i.name_hash = sh(i.name.c_str());
		t.outputs = outputs;
		for (auto& o : t.outputs)
			o.name_hash = sh(o.name.c_str());
		t.function = function;
		t.loop_function = nullptr;
		t.constructor = constructor;
		t.destructor = destructor;
		t.change_structure_callback = change_structure_callback;
		t.preview_provider = preview_provider;
		t.is_block = false;
		t.begin_block_function = nullptr;
		t.end_block_function = nullptr;
	}

	void BlueprintNodeLibraryPrivate::add_template(const std::string& name, const std::string& display_name, BlueprintNodeFlags flags,
		const std::vector<BlueprintSlotDesc>& inputs , const std::vector<BlueprintSlotDesc>& outputs,
		BlueprintNodeLoopFunction loop_function, BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeChangeStructureCallback change_structure_callback, BlueprintNodePreviewProvider preview_provider)
	{
		auto& t = node_templates.emplace_back();
		t.library = this;
		t.name = name;
		t.name_hash = sh(name.c_str());
		t.flags = flags;
		t.display_name = display_name;
		t.inputs = inputs;
		for (auto& i : t.inputs)
			i.name_hash = sh(i.name.c_str());
		t.outputs = outputs;
		for (auto& o : t.outputs)
			o.name_hash = sh(o.name.c_str());
		t.function = nullptr;
		t.loop_function = loop_function;
		t.constructor = constructor;
		t.destructor = destructor;
		t.change_structure_callback = change_structure_callback;
		t.preview_provider = preview_provider;
		t.is_block = false;
		t.begin_block_function = nullptr;
		t.end_block_function = nullptr;
	}

	void BlueprintNodeLibraryPrivate::add_template(const std::string& name, const std::string& display_name, BlueprintNodeFlags flags,
		const std::vector<BlueprintSlotDesc>& inputs, const std::vector<BlueprintSlotDesc>& outputs,
		bool is_block, BlueprintNodeBeginBlockFunction begin_block_function, BlueprintNodeEndBlockFunction end_block_function, 
		BlueprintNodeLoopFunction loop_function, BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeChangeStructureCallback change_structure_callback, BlueprintNodePreviewProvider preview_provider)
	{
		auto& t = node_templates.emplace_back();
		t.library = this;
		t.name = name;
		t.name_hash = sh(name.c_str());
		t.flags = flags;
		t.display_name = display_name;
		t.inputs = inputs;
		for (auto& i : t.inputs)
			i.name_hash = sh(i.name.c_str());
		t.outputs = outputs;
		for (auto& o : t.outputs)
			o.name_hash = sh(o.name.c_str());
		t.function = nullptr;
		t.loop_function = nullptr;
		t.constructor = constructor;
		t.destructor = destructor;
		t.change_structure_callback = change_structure_callback;
		t.preview_provider = preview_provider;
		t.is_block = is_block;
		t.begin_block_function = begin_block_function;
		t.end_block_function = end_block_function;
		t.loop_function = loop_function;
	}

	struct BlueprintNodeLibraryGet : BlueprintNodeLibrary::Get
	{
		BlueprintNodeLibraryPtr operator()(const std::filesystem::path& _filename) override
		{
			auto filename = Path::get(_filename);
			if (filename.empty())
				filename = _filename;

			for (auto& lib : loaded_libraries)
			{
				if (lib->filename == filename)
				{
					lib->ref++;
					return lib.get();
				}
			}

			auto ret = new BlueprintNodeLibraryPrivate;
			ret->filename = filename;
			ret->ref = 1;
			loaded_libraries.emplace_back(ret);
			return ret;
		}
	}BlueprintNodeLibrary_get;
	BlueprintNodeLibrary::Get& BlueprintNodeLibrary::get = BlueprintNodeLibrary_get;

	BlueprintInstancePrivate::BlueprintInstancePrivate(BlueprintPtr _blueprint)
	{
		blueprint = _blueprint;
		if (blueprint)
		{
			blueprint->ref++;
			build();
		}
	}

	static void destroy_instance_group(BlueprintInstanceGroup& g)
	{
		if (g.trigger_message)
		{
			if (auto it = message_receivers.find(g.trigger_message); it != message_receivers.end())
			{
				std::erase_if(it->second, [&](const auto& i) {
					return i == &g;
				});
			}
		}

		for (auto& v : g.variables)
			v.second.type->destroy(v.second.data);

		std::function<void(BlueprintInstanceNode&)> destroy_node;
		destroy_node = [&](BlueprintInstanceNode& n) {
			if (n.destructor)
				n.destructor(n.inputs.size(), n.inputs.data(), n.outputs.size(), n.outputs.data());
			for (auto& c : n.children)
				destroy_node(c);
		};
		destroy_node(g.root_node);
		for (auto& pair : g.slot_datas)
		{
			if (pair.second.own_data)
			{
				auto& attr = pair.second.attribute;
				if (attr.data)
					attr.type->destroy(attr.data);
			}
		}
	}

	BlueprintInstancePrivate::~BlueprintInstancePrivate()
	{
		if (auto debugger = BlueprintDebugger::current(); debugger && debugger->debugging && debugger->debugging->instance == this)
			debugger->debugging = nullptr;
		for (auto& v : variables)
			v.second.type->destroy(v.second.data);
		for (auto& g : groups)
			destroy_instance_group(g.second);
		if (blueprint)
			Blueprint::release(blueprint);
	}

	void BlueprintInstancePrivate::build()
	{
		auto frame = frames;
		std::map<uint, std::list<BlueprintExecutingBlock>>	old_ececuting_stacks;
		std::map<uint, uint>								old_ececuting_node_id;
		for (auto& g : groups)
		{
			old_ececuting_stacks[g.first] = g.second.executing_stack;
			g.second.executing_stack.clear();
			auto executing_node = g.second.executing_node();
			old_ececuting_node_id[g.first] = executing_node ? executing_node->object_id : 0;
		}

		// create data for variables
		if (blueprint->variable_changed_frame > variable_updated_frame)
		{
			std::unordered_map<uint, BlueprintAttribute> new_variables;
			for (auto& v : blueprint->variables)
			{
				BlueprintAttribute attr;
				attr.type = v.type;
				attr.data = v.type->create();
				if (is_pointer(attr.type->tag))
					memset(attr.data, 0, sizeof(voidptr));
				else
					attr.type->copy(attr.data, v.data);
				new_variables.emplace(v.name_hash, attr);
			}
			for (auto& pair : variables)
				pair.second.type->destroy(pair.second.data);
			variables.clear();
			variables = std::move(new_variables);

			variable_updated_frame = frame;
		}

		auto create_group_structure = [&](BlueprintGroupPtr src_g, BlueprintInstanceGroup& g, std::map<uint, BlueprintInstanceGroup::Data>& slots_data) {
			g.execution_type = BlueprintExecutionFunction;
			g.trigger_message = src_g->trigger_message.empty() ? 0 : sh(src_g->trigger_message.c_str());
			// we dont register the group here, maybe some 'static' blueprints need this functionality in later development
			//if (g.trigger_message)
			//{
			//	if (auto it = message_receivers.find(g.trigger_message); it != message_receivers.end())
			//		it->second.push_back(&g);
			//	else
			//		message_receivers[g.trigger_message] = { &g };
			//}

			// create data for group variables
			if (src_g->variable_changed_frame > g.variable_updated_frame)
			{
				std::unordered_map<uint, BlueprintAttribute> new_variables;
				for (auto& v : src_g->variables)
				{
					BlueprintAttribute attr;
					attr.type = v.type;
					attr.data = v.type->create();
					if (is_pointer(attr.type->tag))
						memset(attr.data, 0, sizeof(voidptr));
					else
						attr.type->copy(attr.data, v.data);
					new_variables.emplace(v.name_hash, attr);
				}
				for (auto& pair : g.variables)
					pair.second.type->destroy(pair.second.data);
				g.variables.clear();
				g.variables = std::move(new_variables);

				g.variable_updated_frame = frame;
			}

			std::function<void(BlueprintNodePtr, BlueprintInstanceNode&)> create_node;
			create_node = [&](BlueprintNodePtr block, BlueprintInstanceNode& o) {
				std::vector<BlueprintInstanceNode> rest_nodes;
				o.original = block;
				for (auto n : block->children)
				{
					auto& c = rest_nodes.emplace_back();
					c.original = n;
					c.destructor = n->destructor;
					c.object_id = n->object_id;
					if (n->name.starts_with("Co "))
						g.execution_type = BlueprintExecutionCoroutine;
					create_node(n, c);
				}
				std::function<void(BlueprintInstanceNode&)> process_node;
				process_node = [&](BlueprintInstanceNode& n) {
					BlueprintInstanceNode* unsatisfied_upstream = nullptr;
					for (auto& l : src_g->links)
					{
						auto from_node = l->from_slot->node;
						auto to_node = l->to_slot->node;
						if (from_node->parent == block)
						{
							// if the link's to_node is the node or to_node is inside the node, then the link counts
							if (to_node == n.original || n.original->contains(to_node->parent))
							{
								// if the link's from node still not add, then not ok
								if (auto it = std::find_if(rest_nodes.begin(), rest_nodes.end(), [&](const auto& i) {
									return i.object_id == from_node->object_id;
									}); it != rest_nodes.end())
								{
									unsatisfied_upstream = &(*it);
									break; // one link is not satisfied, break
								}
							}
						}
					}
					if (!unsatisfied_upstream)
					{
						o.children.push_back(n);
						std::erase_if(rest_nodes, [n](const auto& i) {
							return i.original == n.original;
						});
					}
					else
						process_node(*unsatisfied_upstream);
				};
				while (!rest_nodes.empty())
					process_node(rest_nodes.front());
			};
			create_node(src_g->nodes.front().get(), g.root_node);

			g.node_map.clear();
			uint order = 0;
			std::function<void(BlueprintInstanceNode&)> create_map;
			create_map = [&](BlueprintInstanceNode& n) {
				if (n.object_id)
					g.node_map[n.object_id] = &n;
				n.order = order++;
				for (auto& c : n.children)
					create_map(c);
			};
			create_map(g.root_node);

			if (auto n = src_g->find_node("Input"_h); n)
				g.input_node = g.node_map[n->object_id];
			if (auto n = src_g->find_node("Output"_h); n)
				g.output_node = g.node_map[n->object_id];

			// create slot data
			std::function<void(BlueprintInstanceNode&)> create_slots_data;
			create_slots_data = [&](BlueprintInstanceNode& node) {
				auto process_linked_input_slot = [&](BlueprintSlotPtr input) {
					auto linked = false;
					for (auto& l : src_g->links)
					{
						if (l->to_slot == input)
						{
							if (auto it = slots_data.find(l->from_slot->object_id); it != slots_data.end())
							{
								if ((it->second.attribute.type->tag == TagE || it->second.attribute.type->tag == TagD || it->second.attribute.type->tag == TagU)
									&& (input->type == TypeInfo::get<voidptr>() || input->type->tag == TagPU))
								{
									BlueprintInstanceGroup::Data data;
									data.changed_frame = it->second.changed_frame;
									data.attribute.type = input->type;
									data.attribute.data = input->type->create();
									auto ptr = it->second.attribute.data;
									memcpy((voidptr*)data.attribute.data, &ptr, sizeof(voidptr));
									slots_data.emplace(input->object_id, data);

									node.inputs.push_back(data.attribute);
								}
								else
								{
									BlueprintAttribute attr;
									attr.type = it->second.attribute.type;
									attr.data = it->second.attribute.data;
									node.inputs.push_back(attr);
								}
							}
							else
								assert(0);
							linked = true;
						}
					}

					return linked;
				};
				auto create_input_slot_data = [&](BlueprintSlotPtr input) {
					BlueprintInstanceGroup::Data data;
					data.changed_frame = input->data_changed_frame;
					auto is_string_hash = input->type == TypeInfo::get<std::string>() && input->name.ends_with("_hash");
					if (is_string_hash)
						data.attribute.type = TypeInfo::get<uint>();
					else
						data.attribute.type = input->type;
					if (data.attribute.type)
					{
						data.attribute.data = data.attribute.type->create();
						if (is_pointer(data.attribute.type->tag))
							memset(data.attribute.data, 0, sizeof(voidptr));
						else if (input->data)
						{
							if (is_string_hash)
								*(uint*)data.attribute.data = sh((*(std::string*)input->data).c_str());
							else
								data.attribute.type->copy(data.attribute.data, input->data);
						}
					}
					else
						data.attribute.data = nullptr;
					slots_data.emplace(input->object_id, data);

					node.inputs.push_back(data.attribute);
				};
				auto create_output_slot_data = [&](BlueprintSlotPtr output) {
					BlueprintInstanceGroup::Data data;
					data.changed_frame = output->data_changed_frame;
					data.attribute.type = output->type;
					if (data.attribute.type)
					{
						data.attribute.data = data.attribute.type->create();
						if (is_pointer(data.attribute.type->tag))
							memset(data.attribute.data, 0, sizeof(voidptr));
					}
					else
						data.attribute.data = nullptr;
					slots_data.emplace(output->object_id, data);

					node.outputs.push_back(data.attribute);
				};

				auto find_var = [&](uint name, uint location_name = 0)->std::pair<TypeInfo*, void*> {
					if (location_name == 0)
					{
						if (auto it = variables.find(name); it != variables.end())
							return { it->second.type, it->second.data };
						if (auto it = g.variables.find(name); it != g.variables.end())
							return { it->second.type, it->second.data };
						assert(0);
					}
					else
					{
						auto bp_ins = BlueprintInstance::get(location_name);
						if (bp_ins)
						{
							if (bp_ins->built_frame < bp_ins->blueprint->dirty_frame)
								bp_ins->build();

							auto it = bp_ins->variables.find(name);
							assert(it != bp_ins->variables.end());
							return { it->second.type, it->second.data };
						}
						else if (auto ei = find_enum(location_name); ei)
						{
							auto ii = ei->find_item(name);
							assert(ii);

							return { TypeInfo::get<int>(), (void*)&ii->value };
						}
						else
							assert(0);
					}
				};
				auto find_group = [&](uint name, uint location_name = 0)->BlueprintInstanceGroup* {
					if (location_name == 0)
					{
						auto it = groups.find(name);
						assert(it != groups.end());
						return &it->second;
					}
					else
					{
						auto bp_ins = BlueprintInstance::get(location_name);
						assert(bp_ins);

						auto it = bp_ins->groups.find(name);
						assert(it != bp_ins->groups.end());
						return &it->second;
					}
					return nullptr;
				};
				
				if (auto n = node.original; n)
				{
					uint var_name, var_location, property_name;
					get_variable_node_desc(n, &var_name, &var_location, &property_name);

					if (n->name_hash == "Variable"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;
								data.own_data = false;
								slots_data.emplace(n->outputs[0]->object_id, data);

								node.outputs.push_back(data.attribute);
							}
						}
						return;
					}
					if (n->name_hash == "Set Variable"_h ||
						n->name_hash == "Add Assign"_h || 
						n->name_hash == "Subtract Assign"_h || 
						n->name_hash == "Multiple Assign"_h || 
						n->name_hash == "Divide Assign"_h || 
						n->name_hash == "Or Assign"_h || 
						n->name_hash == "And Assign"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[2].get()))
								create_input_slot_data(n->inputs[2].get());
						}
						return;
					}
					if (n->name_hash == "Get Property"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								auto ui = vtype->retrive_ui();
								auto vi = ui->find_variable(property_name);

								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = (char*)vdata + vi->offset;
								data.own_data = false;
								slots_data.emplace(n->outputs[0]->object_id, data);

								node.outputs.push_back(data.attribute);
							}
						}
						return;
					}
					if (n->name_hash == "Get Properties"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								auto ui = vtype->retrive_ui();
								for (auto i = 0; i < n->outputs.size(); i++)
								{
									BlueprintInstanceGroup::Data data;
									data.changed_frame = frame;
									data.attribute.type = ui->variables[i].type;
									data.attribute.data = data.attribute.type->create();
									slots_data.emplace(n->outputs[i]->object_id, data);

									node.outputs.push_back(data.attribute);
								}
							}
						}
						return;
					}
					if (n->name_hash == "Set Property"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								auto ui = vtype->retrive_ui();
								auto vi = ui->find_variable(property_name);

								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = (char*)vdata + vi->offset;

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[3].get()))
								create_input_slot_data(n->inputs[3].get());
						}
					}
					if (n->name_hash == "Array Size"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<uint>();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->outputs[0]->object_id, data);

								node.outputs.push_back(data.attribute);
							}
						}
						return;
					}
					if (n->name_hash == "Array Clear"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
						}
						return;
					}
					if (n->name_hash == "Array Get Item"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[2].get()))
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<uint>();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[2]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype->get_wrapped();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->outputs[0]->object_id, data);

								node.outputs.push_back(data.attribute);
							}
						}
						return;
					}
					if (n->name_hash == "Array Set Item"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[2].get()))
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<uint>();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[2]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[3].get()))
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype->get_wrapped();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[3]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
						}
					}
					if (n->name_hash == "Array Get Item Property"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							auto ui = vtype->retrive_ui();
							auto vi = ui->find_variable(property_name);

							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<uint>();
								data.attribute.data = data.attribute.type->create();
								*(uint*)data.attribute.data = vi->offset;
								slots_data.emplace(n->inputs[2]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[3].get()))
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<uint>();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[3]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vi->type;
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->outputs[0]->object_id, data);

								node.outputs.push_back(data.attribute);
							}
						}
						return;
					}
					if (n->name_hash == "Array Get Item Properties"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[2].get()))
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<uint>();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[2]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
							{
								auto ui = vtype->retrive_ui();
								for (auto i = 0; i < n->outputs.size(); i++)
								{
									BlueprintInstanceGroup::Data data;
									data.changed_frame = frame;
									data.attribute.type = ui->variables[i].type;
									data.attribute.data = data.attribute.type->create();
									slots_data.emplace(n->outputs[i]->object_id, data);

									node.outputs.push_back(data.attribute);
								}
							}
						}
						return;
					}
					if (n->name_hash == "Array Set Item Property"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							auto ui = vtype->retrive_ui();
							auto vi = ui->find_variable(property_name);

							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<uint>();
								data.attribute.data = data.attribute.type->create();
								*(uint*)data.attribute.data = vi->offset;
								slots_data.emplace(n->inputs[2]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[3].get()))
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<uint>();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[3]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[4].get()))
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vi->type;
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[4]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
						}
					}
					if (n->name_hash == "Array Add Item"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[2].get()))
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype->get_wrapped();
								data.attribute.data = data.attribute.type->create();
								if (is_pointer(data.attribute.type->tag))
									memset(data.attribute.data, 0, sizeof(voidptr));
								slots_data.emplace(n->inputs[2]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
						}
						return;
					}
					if (n->name_hash == "Array Emplace Item"_h)
					{
						if (auto [vtype, vdata] = find_var(var_name, var_location); vtype && vdata)
						{
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							for (auto i = 2; i < n->inputs.size(); i++)
							{
								if (!process_linked_input_slot(n->inputs[i].get()))
								{
									BlueprintInstanceGroup::Data data;
									data.changed_frame = frame;
									data.attribute.type = n->inputs[i]->type;
									data.attribute.data = data.attribute.type->create();
									if (is_pointer(data.attribute.type->tag))
										memset(data.attribute.data, 0, sizeof(voidptr));
									slots_data.emplace(n->inputs[i]->object_id, data);

									node.inputs.push_back(data.attribute);
								}
							}
						}
						return;
					}
					if (n->name_hash == "Call"_h)
					{
						auto name = *(uint*)n->inputs[0]->data;
						auto location_name = *(uint*)n->inputs[1]->data;
						if (auto call_group = find_group(name, location_name); call_group)
						{
							{
								BlueprintInstanceGroup::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<PointerAndUint>();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[0]->object_id, data);

								node.inputs.push_back(data.attribute);

								auto& pau = *(PointerAndUint*)data.attribute.data;
								pau.p = call_group->instance;
								pau.u = name;
							}

							for (auto i = 2; i < n->inputs.size(); i++)
							{
								auto input = n->inputs[i].get();
								if (!process_linked_input_slot(input))
									create_input_slot_data(input);
							}
							for (auto& o : n->outputs)
								create_output_slot_data(o.get());
						}
						return;
					}

					for (auto& i : n->inputs)
					{
						if (!process_linked_input_slot(i.get()))
							create_input_slot_data(i.get());
					}
					for (auto& o : n->outputs)
						create_output_slot_data(o.get());

					if (n->constructor)
						n->constructor(node.inputs.size(), node.inputs.data(), node.outputs.size(), node.outputs.data());
				}

				for (auto& c : node.children)
					create_slots_data(c);
			};
			create_slots_data(g.root_node);
		};

		// remove groups that are not in the blueprint anymore
		for (auto it = groups.begin(); it != groups.end();)
		{
			if (!blueprint->find_group(it->first))
			{
				destroy_instance_group(it->second);
				it = groups.erase(it);
			}
			else
				it++;
		}

		// update existing groups
		for (auto& g : groups)
		{
			auto src_g = blueprint->find_group(g.first);

			if (src_g->structure_changed_frame > g.second.structure_updated_frame)
			{
				std::map<uint, BlueprintInstanceGroup::Data> new_slot_datas;
				g.second.root_node.children.clear();
				create_group_structure(src_g, g.second, new_slot_datas);
				for (auto& d : new_slot_datas)
				{
					if (auto it = g.second.slot_datas.find(d.first); it != g.second.slot_datas.end())
					{
						if (it->second.attribute.type == d.second.attribute.type && it->second.changed_frame > d.second.changed_frame)
						{
							if (d.second.attribute.type && d.second.attribute.type != TypeInfo::get<voidptr>() && !is_pointer(d.second.attribute.type->tag))
								d.second.attribute.type->copy(d.second.attribute.data, it->second.attribute.data);
							d.second.changed_frame = it->second.changed_frame;
						}
					}
				}
				for (auto& pair : g.second.slot_datas)
				{
					if (pair.second.own_data)
					{
						if (pair.second.attribute.data)
							pair.second.attribute.type->destroy(pair.second.attribute.data);
					}
				}
				g.second.slot_datas = std::move(new_slot_datas);
				g.second.structure_updated_frame = frame;
				g.second.data_updated_frame = frame;
			}
			else if (src_g->data_changed_frame > g.second.data_updated_frame)
			{
				for (auto& n : src_g->nodes)
				{
					for (auto& i : n->inputs)
					{
						if (auto it = g.second.slot_datas.find(i->object_id); it != g.second.slot_datas.end())
						{
							if (i->data_changed_frame > it->second.changed_frame)
							{
								auto& arg = it->second.attribute;
								if (i->type == arg.type)
								{
									if (!is_pointer(arg.type->tag))
										i->type->copy(arg.data, i->data);
								}
								else if (i->type == TypeInfo::get<std::string>() && arg.type == TypeInfo::get<uint>())
									*(uint*)arg.data = sh((*(std::string*)i->data).c_str());
								else
									assert(0);
								it->second.changed_frame = i->data_changed_frame;
							}
						}
					}
				}
				g.second.data_updated_frame = frame;
			}
		}

		// create new groups
		for (auto& src_g : blueprint->groups)
		{
			auto found = false;
			for (auto& g : groups)
			{
				if (g.first == src_g->name_hash)
				{
					found = true;
					break;
				}
			}
			if (found)
				continue;

			auto& g = groups.emplace(src_g->name_hash, BlueprintInstanceGroup()).first->second;
			g.instance = this;
			g.original = src_g.get();
			g.name = src_g->name_hash;
			create_group_structure(src_g.get(), g, g.slot_datas);
			g.structure_updated_frame = frame;
			g.data_updated_frame = frame;
		}

		for (auto& g : groups)
		{
			if (auto it = old_ececuting_stacks.find(g.first); it != old_ececuting_stacks.end())
			{
				if (!it->second.empty())
				{
					auto id = old_ececuting_node_id[g.first];
					std::function<void(std::vector<BlueprintInstanceNode*>)> find_node;
					find_node = [&](std::vector<BlueprintInstanceNode*> stack) {
						for (auto& c : stack.back()->children)
						{
							if (c.object_id == id)
							{
								for (auto b : stack)
								{
									BlueprintExecutingBlock new_executing_block;
									new_executing_block.node = b;

									for (auto& b2 : it->second)
									{
										if (b2.node->object_id == b->object_id)
										{
											new_executing_block.child_index = b2.child_index;
											new_executing_block.executed_times = b2.executed_times;
											new_executing_block.max_execute_times = b2.max_execute_times;
											break;
										}
									}
									if (!g.second.executing_stack.empty())
										new_executing_block.parent = &g.second.executing_stack.back();
									g.second.executing_stack.push_back(new_executing_block);
								}
								return;
							}
							if (!c.children.empty())
							{
								auto s = stack;
								s.push_back(&c);
								find_node(s);
							}
						}
					};
					find_node({ &g.second.root_node });
				}
			}
		}

		built_frame = frames;
	}

	void BlueprintInstancePrivate::prepare_executing(BlueprintInstanceGroup* group)
	{
		assert(group->instance == this);
		if (built_frame < blueprint->dirty_frame)
			build();

		group->wait_time = 0.f;

		if (!group->executing_stack.empty())
		{
			if (group->executing_stack.front().node->object_id == group->root_node.object_id)
				return;
			group->executing_stack.clear();
		}
		BlueprintExecutingBlock root_block;
		root_block.node = &group->root_node;
		root_block.child_index = 0;
		root_block.executed_times = 0;
		root_block.max_execute_times = 1;
		group->executing_stack.push_back(root_block);

		if (group->executing_stack.front().node->children.empty())
			group->executing_stack.clear();
	}

	void BlueprintInstancePrivate::run(BlueprintInstanceGroup* group)
	{
		while (!group->executing_stack.empty())
		{
			if (auto debugger = BlueprintDebugger::current(); debugger && debugger->debugging)
				return;
			step(group);
		}
	}

	BlueprintInstanceNode* BlueprintInstancePrivate::step(BlueprintInstanceGroup* group)
	{
		if (built_frame < blueprint->dirty_frame)
			build();

		auto debugger = BlueprintDebugger::current();
		if (debugger && debugger->debugging)
			return nullptr;
		if (group->executing_stack.empty())
			return nullptr;

		auto frame = frames;
		{
			auto& current_block = group->executing_stack.back();
			auto& current_node = current_block.node->children[current_block.child_index];

			if (auto node = current_node.original; node)
			{
				BlueprintBreakpointOption breakpoint_option;
				if (debugger && debugger->has_break_node(node, &breakpoint_option))
				{
					if (breakpoint_option == BlueprintBreakpointBreakInCode)
					{
						debugger->remove_break_node(node);
						debug_break();
					}
					else
					{
						debugger->debugging = group;
						if (breakpoint_option == BlueprintBreakpointTriggerOnce)
							debugger->remove_break_node(node);
						debugger->callbacks.call("breakpoint_triggered"_h, node, nullptr);
						printf("Blueprint breakpoint triggered: %s\n", node->name.c_str());
						return nullptr;
					}
				}
				if (!node->is_block)
				{
					if (node->function)
						node->function(current_node.inputs.size(), current_node.inputs.data(), current_node.outputs.size(), current_node.outputs.data());
					if (node->loop_function) // node's loop funtion is used as the odinary function but with execution data
					{
						BlueprintExecutionData execution_data;
						execution_data.group = group;
						execution_data.block = &current_block;
						node->loop_function(current_node.inputs.size(), current_node.inputs.data(), current_node.outputs.size(), current_node.outputs.data(), execution_data);
					}
				}
				else
				{
					if (node->name_hash != "Block"_h || *(uint*)current_node.inputs[0].data)
					{
						BlueprintExecutingBlock new_block;
						new_block.max_execute_times = 1;
						if (node->begin_block_function)
						{
							BlueprintExecutionData execution_data;
							execution_data.group = group;
							execution_data.block = &new_block;
							node->begin_block_function(current_node.inputs.size(), current_node.inputs.data(), current_node.outputs.size(), current_node.outputs.data(), execution_data);
						}
						if (new_block.max_execute_times > 0)
						{
							new_block.parent = &current_block;
							new_block.node = &current_node;
							group->executing_stack.push_back(new_block);
						}
					}
				}
			}

			current_node.updated_frame = frame;
		}

		while (true)
		{
			if (group->executing_stack.empty())
				break;
			auto& current_block = group->executing_stack.back();
			current_block.child_index++;
			if (current_block.child_index < current_block.node->children.size())
				break;
			auto node = current_block.node->original;
			if (node && node->loop_function) // block's loop funtion is used for executing after every loop
			{
				BlueprintExecutionData execution_data;
				execution_data.group = group;
				execution_data.block = &current_block;
				node->loop_function(current_block.node->inputs.size(), current_block.node->inputs.data(), current_block.node->outputs.size(), current_block.node->outputs.data(), execution_data);
			}
			current_block.executed_times++;
			if (group->executing_stack.size() > 1 && // not the root block
				current_block.executed_times < current_block.max_execute_times &&
				!current_block.node->children.empty()) // loop
			{
				current_block.child_index = 0;
				break;
			}
			if (node && node->end_block_function)
				node->end_block_function(current_block.node->inputs.size(), current_block.node->inputs.data(), current_block.node->outputs.size(), current_block.node->outputs.data());
			group->executing_stack.pop_back();
		}

		return group->executing_node();
	}

	void BlueprintInstancePrivate::stop(BlueprintInstanceGroup* group)
	{
		assert(group->instance == this);

		auto debugger = BlueprintDebugger::current();
		if (debugger && debugger->debugging)
			return;
		
		group->executing_stack.clear();
	}

	void BlueprintInstancePrivate::call(BlueprintInstanceGroup* group, void** inputs, void** outputs)
	{
		assert(group->instance == this);

		if (auto obj = group->input_node; obj)
		{
			for (auto i = 0; i < obj->outputs.size(); i++)
			{
				auto& slot = obj->outputs[i];
				slot.type->copy(slot.data, inputs[i]);
			}
		}

		prepare_executing(group);
		run(group);

		if (auto obj = group->output_node; obj)
		{
			for (auto i = 0; i < obj->inputs.size(); i++)
			{
				auto& slot = obj->inputs[i];
				slot.type->copy(outputs[i], slot.data);
			}
		}
	}

	void BlueprintInstancePrivate::register_group(BlueprintInstanceGroup* group)
	{
		assert(group->instance == this);

		if (group->trigger_message)
		{
			if (auto it = message_receivers.find(group->trigger_message); it != message_receivers.end())
				it->second.push_back(group);
			else
				message_receivers[group->trigger_message] = { group };
		}
	}

	void BlueprintInstancePrivate::unregister_group(BlueprintInstanceGroup* group)
	{
		assert(group->instance == this);

		if (group->trigger_message)
		{
			if (auto it = message_receivers.find(group->trigger_message); it != message_receivers.end())
			{
				std::erase_if(it->second, [&](const auto& i) {
					return i == group;
				});
			}
		}
	}

	void BlueprintInstancePrivate::broadcast(uint message)
	{
		if (auto it = message_receivers.find(message); it != message_receivers.end())
		{
			for (auto g : it->second)
			{
				g->instance->prepare_executing(g);
				g->instance->run(g);
			}
		}
	}

	struct BlueprintInstanceCreate : BlueprintInstance::Create
	{
		BlueprintInstancePtr operator()(BlueprintPtr blueprint) override
		{
			return new BlueprintInstancePrivate(blueprint);
		}
	}BlueprintInstance_create;
	BlueprintInstance::Create& BlueprintInstance::create = BlueprintInstance_create;

	struct BlueprintInstanceDestroy : BlueprintInstance::Destroy
	{
		void operator()(BlueprintInstancePtr instance) override
		{
			delete instance;
		}
	}BlueprintInstance_destroy;
	BlueprintInstance::Destroy& BlueprintInstance::destroy = BlueprintInstance_destroy;

	struct BlueprintInstanceGet : BlueprintInstance::Get
	{
		BlueprintInstancePtr operator()(uint name) override
		{
			auto it = named_blueprints.find(name);
			if (it == named_blueprints.end())
				return nullptr;
			return it->second.second;
		}
	}BlueprintInstance_get;
	BlueprintInstance::Get& BlueprintInstance::get = BlueprintInstance_get;

	void BlueprintDebuggerPrivate::add_break_node(BlueprintNodePtr node, BlueprintBreakpointOption option)
	{
		if (!has_break_node(node))
			break_nodes.emplace_back(node, option);
	}

	void BlueprintDebuggerPrivate::remove_break_node(BlueprintNodePtr node)
	{
		std::erase_if(break_nodes, [node](const auto& i) { 
			return i.first == node; 
		});
	}

	static BlueprintDebuggerPtr current_debugger = nullptr;

	struct BlueprintDebuggerCreate : BlueprintDebugger::Create
	{
		BlueprintDebuggerPtr operator()() override
		{
			return new BlueprintDebuggerPrivate();
		}
	}BlueprintDebugger_create;
	BlueprintDebugger::Create& BlueprintDebugger::create = BlueprintDebugger_create;

	struct BlueprintDebuggerCurrent : BlueprintDebugger::Current
	{
		BlueprintDebuggerPtr operator()() override
		{
			return current_debugger;
		}
	}BlueprintDebugger_current;
	BlueprintDebugger::Current& BlueprintDebugger::current = BlueprintDebugger_current;

	struct BlueprintDebuggerSetCurrent : BlueprintDebugger::SetCurrent
	{
		BlueprintDebuggerPtr operator()(BlueprintDebuggerPtr debugger) override
		{
			auto last_one = current_debugger;
			current_debugger = debugger;
			return last_one;
		}
	}BlueprintDebugger_set_current;
	BlueprintDebugger::SetCurrent& BlueprintDebugger::set_current = BlueprintDebugger_set_current;
}
