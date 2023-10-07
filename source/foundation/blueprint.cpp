#include "../xml.h"
#include "typeinfo_serialize.h"
#include "sheet_private.h"
#include "blueprint_private.h"

namespace flame
{
	std::vector<std::unique_ptr<BlueprintT>>						loaded_blueprints;
	std::map<uint, std::pair<BlueprintPtr, BlueprintInstancePtr>>	named_blueprints;
	std::vector<std::unique_ptr<BlueprintNodeLibraryT>>				loaded_libraries;

	static const char* loop_index_names[] = {
		"loop_index_i",
		"loop_index_j",
		"loop_index_k",
	};

	static uint loop_index_hashes[] = {
		"loop_index_i"_h,
		"loop_index_j"_h,
		"loop_index_k"_h,
	};

	struct PointerAndUint
	{
		void* p;
		uint u;
	};

	BlueprintSlotPrivate::~BlueprintSlotPrivate()
	{
		if (data)
			type->destroy(data);
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
		for (auto& v : variables)
			v.type->destroy(v.data);
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

		dirty_frame = frames;

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
				return;
			}
		}

		std::vector<BlueprintNodePtr> to_remove_nodes;
		for (auto& n : group->nodes)
		{
			if (n->name_hash == "Variable"_h ||
				n->name_hash == "Set Variable"_h ||
				n->name_hash == "Array Size"_h ||
				n->name_hash == "Array Get Item"_h ||
				n->name_hash == "Array Set Item"_h ||
				n->name_hash == "Array Add Item"_h)
			{
				auto target_name = *(uint*)n->inputs[0]->data;
				if (target_name == name)
					to_remove_nodes.push_back(n.get());
			}
		}
		for (auto n : to_remove_nodes)
			remove_node(n, false);

		dirty_frame = frames;
	}

	void BlueprintPrivate::alter_variable(BlueprintGroupPtr group, uint old_name, const std::string& new_name, TypeInfo* type)
	{
		assert(!group || group->blueprint == this);
		auto& vars = group ? group->variables : variables;

		for (auto it = vars.begin(); it != vars.end(); ++it)
		{
			if (it->name_hash == old_name)
			{
				if (!new_name.empty())
				{
					if (it->name != new_name)
					{
						it->name = new_name;
						it->name_hash = sh(new_name.c_str());
						auto process_group = [&](BlueprintGroupPtr group) {
							for (auto& n : group->nodes)
							{
								if (n->name_hash == "Variable"_h ||
									n->name_hash == "Set Variable"_h ||
									n->name_hash == "Array Size"_h ||
									n->name_hash == "Array Get Item"_h ||
									n->name_hash == "Array Set Item"_h ||
									n->name_hash == "Array Add Item"_h)
								{
									auto target_name = *(uint*)n->inputs[0]->data;
									if (target_name == old_name)
									{
										switch (n->name_hash)
										{
										case "Variable"_h: n->display_name = new_name; break;
										case "Set Variable"_h: n->display_name = "Set " + new_name; break;
										case "Array Size"_h: n->display_name = new_name + ": Size"; break;
										case "Array Get Item"_h: n->display_name = new_name + ": Get Item"; break;
										case "Array Set Item"_h: n->display_name = new_name + ": Set Item"; break;
										case "Array Add Item"_h: n->display_name = new_name + ": Add Item"; break;
										}
										*(uint*)n->inputs[0]->data = it->name_hash;
									}
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
					}
				}
				if (type)
				{
					if (it->type != type)
					{
						it->type = type;
						// TOOD: break links that nolonger match the type
					}
				}
				return;
			}
		}

		printf("blueprint alter_variable: variable not found\n");
	}

	BlueprintNodePtr BlueprintPrivate::add_node(BlueprintGroupPtr group, BlueprintNodePtr parent, const std::string& name, const std::string& display_name,
		const std::vector<BlueprintSlotDesc>& inputs, const std::vector<BlueprintSlotDesc>& outputs,
		BlueprintNodeFunction function, BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeInputSlotChangedCallback input_slot_changed_callback, BlueprintNodePreviewProvider preview_provider, 
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
		ret->display_name = display_name;
		if (is_block)
		{
			auto i = new BlueprintSlotPrivate;
			i->node = ret;
			i->object_id = next_object_id++;
			i->name = "Execute";
			i->name_hash = "Execute"_h;
			i->flags = BlueprintSlotFlagInput;
			i->allowed_types.push_back(TypeInfo::get<BlueprintSignal>());
			i->type = i->allowed_types.front();
			i->data = i->type->create();
			(*(BlueprintSignal*)i->data).v = 1;
			ret->inputs.emplace_back(i);

			auto o = new BlueprintSlotPrivate;
			o->node = ret;
			o->object_id = next_object_id++;
			o->name = "Execute";
			o->name_hash = "Execute"_h;
			o->flags = BlueprintSlotFlagOutput;
			o->allowed_types.push_back(TypeInfo::get<BlueprintSignal>());
			o->type = o->allowed_types.front();
			ret->outputs.emplace_back(o);
		}
		for (auto& src_i : inputs)
		{
			auto i = new BlueprintSlotPrivate;
			i->node = ret;
			i->object_id = next_object_id++;
			i->name = src_i.name;
			i->name_hash = src_i.name_hash;
			i->flags = src_i.flags | BlueprintSlotFlagInput;
			i->allowed_types = src_i.allowed_types;
			i->default_value = src_i.default_value;
			if (!i->allowed_types.empty())
			{
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				if (!i->default_value.empty())
					i->type->unserialize(i->default_value, i->data);
			}
			ret->inputs.emplace_back(i);
		}
		for (auto& src_o : outputs)
		{
			auto o = new BlueprintSlotPrivate;
			o->node = ret;
			o->object_id = next_object_id++;
			o->name = src_o.name;
			o->name_hash = src_o.name_hash;
			o->flags = src_o.flags | BlueprintSlotFlagOutput;
			o->allowed_types = src_o.allowed_types;
			if (!o->allowed_types.empty())
				o->type = o->allowed_types.front();
			ret->outputs.emplace_back(o);
		}
		ret->constructor = constructor;
		ret->destructor = destructor;
		ret->function = function;
		ret->input_slot_changed_callback = input_slot_changed_callback;
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
		if (is_block)
		{
			ret->rect.a = vec2(0.f);
			ret->rect.b = vec2(200.f);
		}
		group->nodes.emplace_back(ret);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	BlueprintNodePtr BlueprintPrivate::add_block(BlueprintGroupPtr group, BlueprintNodePtr parent)
	{
		return add_node(group, parent, "Block", "", {}, {}, nullptr, nullptr, nullptr, nullptr, nullptr, true);
	}

	BlueprintNodePtr BlueprintPrivate::add_variable_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint variable_name, uint type, uint location_name)
	{
		assert(group && group->blueprint == this);
		if (parent)
			assert(group == parent->group);
		else
			parent = group->nodes.front().get();

		BlueprintVariable variable;
		auto found = false;
		if (location_name == 0)
		{
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
				printf("blueprint add_variable_node: cannot find variable %d\n", variable_name);
				return nullptr;
			}
		}
		else
		{
			auto sht = Sheet::get(location_name);
			if (sht)
			{
				auto idx = sht->find_column(variable_name);
				if (idx == -1)
				{
					printf("blueprint add_variable_node: cannot find variable %d in sheet %s\n", variable_name, sht->name.c_str());
					return nullptr;
				}
				if (sht->rows.empty())
				{
					printf("blueprint add_variable_node: there is no rows in sheet: %s\n", sht->name.c_str());
					return nullptr;
				}

				auto& col = sht->columns[idx];
				auto& row = sht->rows[0];
				variable.name = col.name;
				variable.name_hash = col.name_hash;
				variable.type = col.type;
			}
			else
			{
				auto bp = Blueprint::get(location_name);
				if (bp)
				{
					for (auto& v : bp->variables)
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
						printf("blueprint add_variable_node: cannot find variable %d in blueprint: %s\n", variable_name, bp->name.c_str());
						return nullptr;
					}
				}
				else
				{
					printf("blueprint add_variable_node: cannot find sheet or blueprint %d\n", location_name);
					return nullptr;
				}
			}
		}

		auto ret = new BlueprintNodePrivate;
		ret->group = group;
		ret->object_id = next_object_id++;
		switch (type)
		{
		case "get"_h:
			ret->name = "Variable";
			ret->name_hash = "Variable"_h;
			ret->display_name = variable.name;
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
				i->default_value = "0";
				ret->inputs.emplace_back(i);
			}
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
		case "set"_h:
			ret->name = "Set Variable";
			ret->name_hash = "Set Variable"_h;
			ret->display_name = "Set " + variable.name;
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
				i->default_value = "0";
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
			ret->function = [](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto type = inputs[0].type;
				auto data = inputs[0].data;
				if (data)
					type->copy(inputs[0].data, inputs[1].data);
			};
			break;
		case "array_size"_h:
			ret->name = "Array Size";
			ret->name_hash = "Array Size"_h;
			ret->display_name = variable.name + ": Size";
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
				i->default_value = "0";
				ret->inputs.emplace_back(i);
			}
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
			ret->function = [](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto array_type = inputs[0].type;
				auto parray = (std::vector<char>*)inputs[0].data;
				if (parray && array_type)
					*(uint*)outputs[0].data = parray->size() / array_type->get_wrapped()->size;
			};
			break;
		case "array_get_item"_h:
			ret->name = "Array Get Item";
			ret->name_hash = "Array Get Item"_h;
			ret->display_name = variable.name + ": Get Item";
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
				i->default_value = "0";
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
				o->allowed_types.push_back(variable.type->get_wrapped());
				o->type = o->allowed_types.front();
				o->data = o->type->create();
				ret->outputs.emplace_back(o);

			}
			ret->function = [](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto parray = inputs[0].data;
				auto index = *(uint*)inputs[1].data;
				auto pitem = outputs[0].data;
				auto item_type = outputs[0].type;
				if (parray && pitem && item_type)
				{
					switch (item_type->tag)
					{
					case TagD:
					{
						auto& array = *(std::vector<char>*)parray;
						if (index * item_type->size < array.size())
							memcpy(pitem, array.data() + index * item_type->size, item_type->size);
					}
						break;
					case TagPU:
					{
						auto& array = *(std::vector<voidptr>*)parray;
						if (index < array.size())
							*(voidptr*)pitem = array[index];
					}
						break;
					}
				}
			};
			break;
		case "array_set_item"_h:
			ret->name = "Array Set Item";
			ret->name_hash = "Array Set Item"_h;
			ret->display_name = variable.name + ": Set Item";
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
				i->default_value = "0";
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
				i->allowed_types.push_back(variable.type->get_wrapped());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			ret->function = [](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto parray = inputs[0].data;
				auto index = *(uint*)inputs[1].data;
				auto pitem = inputs[2].data;
				auto item_type = inputs[2].type;
				if (parray && pitem && item_type)
				{
					switch (item_type->tag)
					{
					case TagD:
					{
						auto& array = *(std::vector<char>*)parray;
						if (index * item_type->size < array.size())
							memcpy(array.data() + index * item_type->size, pitem, item_type->size);
					}
						break;
					}
				}
			};
			break;
		case "array_add_item"_h:
			ret->name = "Array Add Item";
			ret->name_hash = "Array Add Item"_h;
			ret->display_name = variable.name + ": Add Item";
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
				i->default_value = "0";
				ret->inputs.emplace_back(i);
			}
			{
				auto i = new BlueprintSlotPrivate;
				i->node = ret;
				i->object_id = next_object_id++;
				i->name = "Item";
				i->name_hash = "Item"_h;
				i->flags = BlueprintSlotFlagInput;
				i->allowed_types.push_back(variable.type->get_wrapped());
				i->type = i->allowed_types.front();
				i->data = i->type->create();
				ret->inputs.emplace_back(i);
			}
			ret->function = [](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
				auto parray = inputs[0].data;
				auto pitem = inputs[1].data;
				auto item_type = inputs[1].type;
				if (parray && pitem && item_type)
				{
					switch (item_type->tag)
					{
					case TagD:
						if (item_type->pod)
						{
							auto& array = *(std::vector<char>*)parray;
							array.resize(array.size() + item_type->size);
							memcpy(array.data() + array.size() - item_type->size, pitem, item_type->size);
						}
						break;
					case TagPU:
					{
						auto& array = *(std::vector<voidptr>*)parray;
						array.push_back(*(voidptr*)pitem);
					}
						break;
					}
				}
			};
			break;
		}
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

	BlueprintNodePtr BlueprintPrivate::add_call_node(BlueprintGroupPtr group, BlueprintNodePtr parent, uint group_name, uint location_name)
	{
		assert(group && group->blueprint == this);
		if (parent)
			assert(group == parent->group);
		else
			parent = group->nodes.front().get();

		BlueprintGroupPtr call_group = nullptr;
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
			printf("blueprint add_call_node: cannot find group %d\n", group_name);
			return nullptr;
		}
		if (call_group == group)
		{
			printf("blueprint add_call_node: cannot call its own group %d\n", group_name);
			return nullptr;
		}

		auto ret = new BlueprintNodePrivate;
		ret->group = group;
		ret->object_id = next_object_id++;
		ret->name = "Call";
		ret->name_hash = "Call"_h;
		ret->display_name = "Call " + call_group->name;
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
		ret->function = [](BlueprintAttribute* inputs, BlueprintAttribute* outputs) {
			auto& pau = *(PointerAndUint*)inputs[0].data;
			if (pau.p && pau.u)
			{
				if (auto ins = (BlueprintInstancePtr)pau.p; ins)
				{
					if (auto g = ins->get_group(pau.u); g)
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
						ins->call(pau.u, input_args.data(), output_args.data());
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

	static void update_depth(BlueprintNodePtr n) 
	{
		for (auto& c : n->children)
		{
			c->depth = n->depth + 1;
			update_depth(c);
		}
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

	static void change_slot_type(BlueprintSlotPtr slot, TypeInfo* new_type)
	{
		if (slot->type == new_type)
			return;
		auto has_data = slot->data != nullptr;
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
				slot->data = new_type->create();
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
		}
		for (auto l : to_remove_links)
			remove_link(l);
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

	void BlueprintPrivate::set_node_parent(BlueprintNodePtr node, BlueprintNodePtr new_parent)
	{
		auto group = node->group;
		assert(group && group->blueprint == this && group == new_parent->group);

		if (node->parent == new_parent)
			return;

		auto old_parent = node->parent;
		for (auto it = old_parent->children.begin(); it != old_parent->children.end(); it++)
		{
			if (*it == node)
			{
				old_parent->children.erase(it);
				break;
			}
		}

		node->parent = new_parent;
		update_depth(node);
		new_parent->children.push_back(node);

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
			std::vector<TypeInfo*> input_types(n->inputs.size());
			std::vector<TypeInfo*> output_types(n->outputs.size());
			for (auto i = 0; i < input_types.size(); i++)
				input_types[i] = n->inputs[i]->type;
			for (auto i = 0; i < output_types.size(); i++)
				output_types[i] = n->outputs[i]->type;

			if (n->input_slot_changed_callback)
			{
				n->input_slot_changed_callback(input_types.data(), output_types.data());

				for (auto i = 0; i < output_types.size(); i++)
				{
					auto slot = n->outputs[i].get();
					if (slot->type != output_types[i])
					{
						change_slot_type(slot, blueprint_allow_type(slot->allowed_types, output_types[i]) ? output_types[i] : nullptr);

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

	void BlueprintPrivate::set_input_type(BlueprintSlotPtr slot, TypeInfo* type)
	{
		auto group = slot->node->group;
		assert(group && group->blueprint == this);
		assert(slot->flags & BlueprintSlotFlagInput);

		if (!blueprint_allow_type(slot->allowed_types, type))
		{
			printf("blueprint set_input_type: type not allowed\n");
			return;
		}
		else
		{
			change_slot_type(slot, type);
			update_node_output_types(slot->node);
			clear_invalid_links(group);

			auto frame = frames;
			group->structure_changed_frame = frame;
			dirty_frame = frame;
		}
	}

	BlueprintLinkPtr BlueprintPrivate::add_link(BlueprintSlotPtr from_slot, BlueprintSlotPtr to_slot)
	{
		auto group = from_slot->node->group;
		assert(group && group->blueprint == this && group == to_slot->node->group);
		assert(from_slot->flags & BlueprintSlotFlagOutput);
		assert(to_slot->flags & BlueprintSlotFlagInput);

		if (from_slot->node == to_slot->node)
		{
			printf("blueprint add_link: cannot link because from_slot and to_slot are from the same node\n");
			return nullptr;
		}
		if (!from_slot->node->parent->contains(to_slot->node))
		{
			printf("blueprint add_link: cannot link because to_slot's node should comes from from_slot's node's parent\n");
			return nullptr;
		}

		if (!blueprint_allow_type(to_slot->allowed_types, from_slot->type))
		{
			printf("blueprint add_link: cannot link because type is not allowed\n");
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

		for (auto n : loop_index_names)
			add_variable(g, n, TypeInfo::get<uint>());
		add_block(g, nullptr);

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

		for (auto it = groups.begin(); it != groups.end(); it++)
		{
			if (it->get() == group)
			{
				groups.erase(it);
				break;
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
		n = g->blueprint->add_node(g, nullptr, "Input", "", {}, outputs, nullptr, nullptr, nullptr, nullptr, nullptr);
		n->position = old_position;
		for (auto& l : old_links)
		{
			auto from_slot = n->find_output(l.first);
			if (from_slot)
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

	static void update_group_output_node(BlueprintGroupPtr g)
	{
		auto n = g->find_node("Output"_h);
		vec2 old_position = vec2(0.f);
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
		n = g->blueprint->add_node(g, nullptr, "Output", "", inputs, {}, nullptr, nullptr, nullptr, nullptr, nullptr);
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

	void BlueprintPrivate::save(const std::filesystem::path& path)
	{
		pugi::xml_document doc;

		auto write_ti = [&](TypeInfo* ti, pugi::xml_attribute a) {
			a.set_value((TypeInfo::serialize_t(ti->tag) + '@' + ti->name).c_str());
		};

		auto doc_root = doc.append_child("blueprint");
		if (!variables.empty())
		{
			auto n_variables = doc_root.append_child("variables");
			for (auto& v : variables)
			{
				auto n_variable = n_variables.append_child("variable");
				n_variable.append_attribute("name").set_value(v.name.c_str());
				write_ti(v.type, n_variable.append_attribute("type"));
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
					n_input.append_attribute("default_value").set_value(v.default_value.c_str());
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
					n_node.append_attribute("rect").set_value(str((vec4)n->rect).c_str());
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
				n_node.append_attribute("name").set_value(n->name.c_str());
				n_node.append_attribute("position").set_value(str(n->position).c_str());
				if (n->is_block)
					n_node.append_attribute("rect").set_value(str((vec4)n->rect).c_str());

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
			auto n_links = n_group.append_child("links");
			std::vector<BlueprintLinkPtr> sorted_links(g.first->links.size());
			for (auto i = 0; i < sorted_links.size(); i++)
				sorted_links[i] = g.first->links[i].get();
			std::sort(sorted_links.begin(), sorted_links.end(), [](const auto a, const auto b) {
				if (a->from_slot->node == b->to_slot->node)
					return false;
				return a->object_id < b->object_id;
			});
			for (auto l : sorted_links)
			{
				auto n_link = n_links.append_child("link");
				n_link.append_attribute("object_id").set_value(l->object_id);
				n_link.append_attribute("from_node").set_value(l->from_slot->node->object_id);
				n_link.append_attribute("from_slot").set_value(l->from_slot->name_hash);
				n_link.append_attribute("to_node").set_value(l->to_slot->node->object_id);
				n_link.append_attribute("to_slot").set_value(l->to_slot->name_hash);
			}
		}

		if (!path.empty())
			filename = path;
		doc.save_file(filename.c_str());
	}

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

			auto ret = new BlueprintPrivate;
			if (std::filesystem::exists(filename))
			{
				pugi::xml_document doc;
				pugi::xml_node doc_root;

				if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("blueprint"))
				{
					wprintf(L"blueprint does not exist or wrong format: %s\n", _filename.c_str());
					return nullptr;
				}

				auto read_ti = [&](pugi::xml_attribute a) {
					auto sp = SUS::to_string_vector(SUS::split(a.value(), '@'));
					TypeTag tag;
					TypeInfo::unserialize_t(sp[0], tag);
					return TypeInfo::get(tag, sp[1]);
				};

				for (auto n_variable : doc_root.child("variables"))
				{
					auto type = read_ti(n_variable.attribute("type"));
					auto data = ret->add_variable(nullptr, n_variable.attribute("name").value(), type);
					type->unserialize(n_variable.attribute("value").value(), data);
				}
				for (auto n_group : doc_root.child("groups"))
				{
					auto g = ret->add_group(n_group.attribute("name").value());

					for (auto n_variable : n_group.child("variables"))
					{
						auto type = read_ti(n_variable.attribute("type"));
						auto data = ret->add_variable(g, n_variable.attribute("name").value(), type);
						type->unserialize(n_variable.attribute("value").value(), data);
					}
					for (auto n_input : n_group.child("inputs"))
						ret->add_group_input(g, n_input.attribute("name").value(), read_ti(n_input.attribute("type")));
					for (auto n_output : n_group.child("outputs"))
						ret->add_group_output(g, n_output.attribute("name").value(), read_ti(n_output.attribute("type")));

					std::map<uint, BlueprintNodePtr> node_map;

					for (auto n_node : n_group.child("nodes"))
					{
						std::string name = n_node.attribute("name").value();
						auto parent_id = n_node.attribute("parent_id").as_uint();
						BlueprintNodePtr parent = nullptr;
						if (parent_id != 0)
						{
							if (auto it = node_map.find(parent_id); it != node_map.end())
								parent = it->second;
							else
							{
								printf("add node: cannot find parent with id %d\n", parent_id);
								continue;
							}
						}
						auto read_input = [&](BlueprintNodePtr n, pugi::xml_node n_input) {
							auto name = n_input.attribute("name").value();
							auto i = n->find_input(sh(name));
							if (i)
							{
								if (auto a_type = n_input.attribute("type"); a_type)
								{
									change_slot_type(i, read_ti(a_type));
									update_node_output_types(n);
									clear_invalid_links(g);
								}
								i->type->unserialize(n_input.attribute("value").value(), i->data);
							}
							else
								printf("add node: cannot find input: %s\n", name);
						};
						if (name == "Block")
						{
							auto n = ret->add_block(g, parent);
							node_map[n_node.attribute("object_id").as_uint()] = n;
							n->position = s2t<2, float>(n_node.attribute("position").value());
							n->rect = s2t<4, float>(n_node.attribute("rect").value());
						}
						else if (name == "Input")
						{
							if (auto n = g->find_node("Input"_h); n)
							{
								node_map[n_node.attribute("object_id").as_uint()] = n;
								n->position = s2t<2, float>(n_node.attribute("position").value());
							}
						}
						else if (name == "Output")
						{
							if (auto n = g->find_node("Output"_h); n)
							{
								node_map[n_node.attribute("object_id").as_uint()] = n;
								n->position = s2t<2, float>(n_node.attribute("position").value());
							}
						}
						else if (name == "Variable")
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
							auto n = ret->add_variable_node(g, parent, name, "get"_h, location_name);
							node_map[n_node.attribute("object_id").as_uint()] = n;
							n->position = s2t<2, float>(n_node.attribute("position").value());
						}
						else if (name == "Set Variable")
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
							auto n = ret->add_variable_node(g, parent, name, "set"_h, location_name);
							for (auto n_input : other_inputs)
								read_input(n, n_input);
							node_map[n_node.attribute("object_id").as_uint()] = n;
							n->position = s2t<2, float>(n_node.attribute("position").value());
						}
						else if (name == "Array Size")
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
							auto n = ret->add_variable_node(g, parent, name, "array_size"_h, location_name);
							for (auto n_input : other_inputs)
								read_input(n, n_input);
							node_map[n_node.attribute("object_id").as_uint()] = n;
							n->position = s2t<2, float>(n_node.attribute("position").value());
						}
						else if (name == "Array Get Item")
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
							auto n = ret->add_variable_node(g, parent, name, "array_get_item"_h, location_name);
							for (auto n_input : other_inputs)
								read_input(n, n_input);
							node_map[n_node.attribute("object_id").as_uint()] = n;
							n->position = s2t<2, float>(n_node.attribute("position").value());
						}
						else if (name == "Array Set Item")
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
							auto n = ret->add_variable_node(g, parent, name, "array_set_item"_h, location_name);
							for (auto n_input : other_inputs)
								read_input(n, n_input);
							node_map[n_node.attribute("object_id").as_uint()] = n;
							n->position = s2t<2, float>(n_node.attribute("position").value());
						}
						else if (name == "Array Add Item")
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
							auto n = ret->add_variable_node(g, parent, name, "array_add_item"_h, location_name);
							for (auto n_input : other_inputs)
								read_input(n, n_input);
							node_map[n_node.attribute("object_id").as_uint()] = n;
							n->position = s2t<2, float>(n_node.attribute("position").value());
						}
						else if (name == "Call")
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
							auto n = ret->add_call_node(g, parent, name, location_name);
							for (auto n_input : other_inputs)
								read_input(n, n_input);
							node_map[n_node.attribute("object_id").as_uint()] = n;
							n->position = s2t<2, float>(n_node.attribute("position").value());
						}
						else
						{
							auto added = false;
							for (auto& library : loaded_libraries)
							{
								for (auto& t : library->node_templates)
								{
									if (t.name == name)
									{
										auto n = ret->add_node(g, parent, t.name, t.display_name, t.inputs, t.outputs,
											t.function, t.constructor, t.destructor, t.input_slot_changed_callback, t.preview_provider, 
											t.is_block, t.begin_block_function, t.end_block_function);
										for (auto n_input : n_node.child("inputs"))
											read_input(n, n_input);
										node_map[n_node.attribute("object_id").as_uint()] = n;
										n->position = s2t<2, float>(n_node.attribute("position").value());
										if (n->is_block)
										{
											n->rect = s2t<4, float>(n_node.attribute("rect").value());
											if (n->rect.invalid())
												n->rect.a = n->rect.b = n->position;
										}

										added = true;
										break;
									}
								}
								if (added)
									break;
							}
							if (!added)
								printf("cannot find node template: %s\n", name.c_str());
						}
					}
					for (auto n_link : n_group.child("links"))
					{
						BlueprintNodePtr from_node, to_node;
						BlueprintSlotPtr from_slot, to_slot;
						if (auto id = n_link.attribute("from_node").as_uint(); id)
						{
							if (auto it = node_map.find(id); it != node_map.end())
								from_node = it->second;
							else
							{
								printf("link: cannot find node: %u\n", id);
								continue;
							}
						}
						if (auto id = n_link.attribute("to_node").as_uint(); id)
						{
							if (auto it = node_map.find(id); it != node_map.end())
								to_node = it->second;
							else
							{
								printf("link: cannot find node: %u\n", id);
								continue;
							}
						}
						if (auto name = n_link.attribute("from_slot").as_uint(); name)
						{
							from_slot = from_node->find_output(name);
							if (!from_slot)
							{
								printf("link: cannot find output: %u in node: %s\n", name, from_node->name.c_str());
								continue;
							}
						}
						if (auto name = n_link.attribute("to_slot").as_uint(); name)
						{
							to_slot = to_node->find_input(name);
							if (!to_slot)
							{
								printf("link: cannot find input: %u in node: %s\n", name, to_node->name.c_str());
								continue;
							}
						}
						ret->add_link(from_slot, to_slot);
					}
				}
			}
			else
			{
				ret->add_group("main");
				ret->save(filename);
			}

			ret->filename = filename;
			if (is_static)
			{
				ret->name = filename.filename().stem().string();
				ret->name_hash = sh(ret->name.c_str());
				assert(named_blueprints.find(ret->name_hash) == named_blueprints.end());
				named_blueprints[ret->name_hash] = std::make_pair(ret, BlueprintInstance::create(ret));
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

	void BlueprintNodeLibraryPrivate::add_template(const std::string& name, const std::string& display_name, 
		const std::vector<BlueprintSlotDesc>& inputs, const std::vector<BlueprintSlotDesc>& outputs,
		BlueprintNodeFunction function, BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeInputSlotChangedCallback input_slot_changed_callback, BlueprintNodePreviewProvider preview_provider, 
		bool is_block, BlueprintNodeBeginBlockFunction begin_block_function, BlueprintNodeEndBlockFunction end_block_function)
	{
		auto& t = node_templates.emplace_back();
		t.name = name;
		t.name_hash = sh(name.c_str());
		t.display_name = display_name;
		t.inputs = inputs;
		for (auto& i : t.inputs)
			i.name_hash = sh(i.name.c_str());
		t.outputs = outputs;
		for (auto& o : t.outputs)
			o.name_hash = sh(o.name.c_str());
		t.function = function;
		t.constructor = constructor;
		t.destructor = destructor;
		t.input_slot_changed_callback = input_slot_changed_callback;
		t.preview_provider = preview_provider;
		t.is_block = is_block;
		t.begin_block_function = begin_block_function;
		t.end_block_function = end_block_function;
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
		blueprint->ref++;
		build();
	}

	static void destroy_instance_group(BlueprintInstance::Group& g)
	{
		std::function<void(BlueprintInstance::Node&)> destroy_node;
		destroy_node = [&](BlueprintInstance::Node& n) {
			if (n.original && n.original->destructor)
				n.original->destructor(n.inputs.data(), n.outputs.data());
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
		for (auto& g : groups)
			destroy_instance_group(g.second);
		Blueprint::release(blueprint);
	}

	void BlueprintInstancePrivate::build()
	{
		auto frame = frames;
		std::map<uint, std::vector<ExecutingBlock>> old_ececuting_stacks;
		std::map<uint, uint>						old_ececuting_node_id;
		for (auto& g : groups)
		{
			old_ececuting_stacks[g.first] = g.second.executing_stack;
			g.second.executing_stack.clear();
			auto executing_node = g.second.executing_node();
			old_ececuting_node_id[g.first] = executing_node ? executing_node->object_id : 0;
		}

		// create data for variables
		std::unordered_map<uint, BlueprintAttribute> new_variables;
		for (auto& v : blueprint->variables)
		{
			BlueprintAttribute attr;
			attr.type = v.type;
			attr.data = v.type->create();
			if (is_pointer(attr.type->tag))
				memset(attr.data, 0, sizeof(voidptr));
			if (auto it = variables.find(v.name_hash); it != variables.end())
				attr.type->copy(attr.data, it->second.data);
			new_variables.emplace(v.name_hash, attr);
		}
		for (auto& pair : variables)
			pair.second.type->destroy(pair.second.data);
		variables.clear();
		variables = std::move(new_variables);

		auto create_group_structure = [&](BlueprintGroupPtr src_g, Group& g, std::map<uint, Group::Data>& slots_data) {
			// create data for variables
			std::unordered_map<uint, BlueprintAttribute> new_variables;
			for (auto& v : src_g->variables)
			{
				BlueprintAttribute attr;
				attr.type = v.type;
				attr.data = v.type->create();
				if (is_pointer(attr.type->tag))
					memset(attr.data, 0, sizeof(voidptr));
				if (auto it = g.variables.find(v.name_hash); it != g.variables.end())
					attr.type->copy(attr.data, it->second.data);
				new_variables.emplace(v.name_hash, attr);
			}
			for (auto& pair : g.variables)
				pair.second.type->destroy(pair.second.data);
			g.variables.clear();
			g.variables = std::move(new_variables);

			std::function<void(BlueprintNodePtr, Node&)> create_node;
			create_node = [&](BlueprintNodePtr block, Node& o) {
				std::vector<Node> rest_nodes;
				for (auto n : block->children)
				{
					auto& c = rest_nodes.emplace_back();
					c.original = n;
					c.object_id = n->object_id;
					create_node(n, c);
				}
				auto is_inside = [](BlueprintNodePtr b1, BlueprintNodePtr b2) {
					while (b2)
					{
						if (b1 == b2)
							return true;
						b2 = b2->parent;
					}
					return false;
				};
				while (!rest_nodes.empty())
				{
					for (auto it = rest_nodes.begin(); it != rest_nodes.end();)
					{
						auto ok = true;
						for (auto& l : src_g->links)
						{
							if (l->from_slot->node->parent == block)
							{
								auto from_node = l->from_slot->node;
								auto to_node = l->to_slot->node;
								// if the link's to node is the node or to node is inside the node, then the link counts
								if (to_node == it->original || it->original->contains(to_node->parent))
								{
									// if the link's from node still not add, then not ok
									if (std::find_if(rest_nodes.begin(), rest_nodes.end(), [&](const auto& i) {
										return i.object_id == from_node->object_id;
									}) != rest_nodes.end())
									{
										ok = false;
										break; // one link is not satisfied, break
									}
								}
							}
						}
						if (ok)
						{
							o.children.push_back(*it);
							it = rest_nodes.erase(it);
						}
						else
							it++;
					}
				}
			};
			create_node(src_g->nodes.front().get(), g.root_node);

			g.node_map.clear();
			std::function<void(Node&)> create_map;
			create_map = [&](Node& n) {
				if (n.object_id)
					g.node_map[n.object_id] = &n;
				for (auto& c : n.children)
					create_map(c);
			};
			create_map(g.root_node);

			if (auto n = src_g->find_node("Input"_h); n)
				g.input_node = g.node_map[n->object_id];
			if (auto n = src_g->find_node("Output"_h); n)
				g.output_node = g.node_map[n->object_id];

			// create slot data
			std::function<void(Node&)> create_slots_data;
			create_slots_data = [&](Node& node) {
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
									Group::Data data;
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
					Group::Data data;
					data.changed_frame = input->data_changed_frame;
					data.attribute.type = input->type;
					if (data.attribute.type)
					{
						data.attribute.data = data.attribute.type->create();
						if (is_pointer(data.attribute.type->tag))
							memset(data.attribute.data, 0, sizeof(voidptr));
						else if (input->data)
							data.attribute.type->copy(data.attribute.data, input->data);
					}
					else
						data.attribute.data = nullptr;
					slots_data.emplace(input->object_id, data);

					node.inputs.push_back(data.attribute);
				};
				auto create_output_slot_data = [&](BlueprintSlotPtr output) {
					Group::Data data;
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
						auto sht = Sheet::get(location_name);
						if (sht)
						{
							auto idx = sht->find_column(name);
							assert(idx != -1);
							assert(!sht->rows.empty());

							auto& col = sht->columns[idx];
							auto& row = sht->rows[0];
							return { col.type, row.datas[idx] };
						}
						else
						{
							auto bp_ins = BlueprintInstance::get(location_name);
							assert(bp_ins);

							auto it = bp_ins->variables.find(name);
							assert(it != bp_ins->variables.end());
							return { it->second.type, it->second.data };
						}
					}
				};
				auto find_group = [&](uint name, uint location_name = 0)->BlueprintInstance::Group* {
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
				auto n = node.original;
				if (n)
				{
					if (n->name_hash == "Variable"_h)
					{
						auto name = *(uint*)n->inputs[0]->data;
						auto location_name = *(uint*)n->inputs[1]->data;
						if (auto [vtype, vdata] = find_var(name, location_name); vtype && vdata)
						{
							{
								Group::Data data;
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
					if (n->name_hash == "Set Variable"_h)
					{
						auto name = *(uint*)n->inputs[0]->data;
						auto location_name = *(uint*)n->inputs[1]->data;
						if (auto [vtype, vdata] = find_var(name, location_name); vtype && vdata)
						{
							{
								Group::Data data;
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
					if (n->name_hash == "Array Size"_h)
					{
						auto name = *(uint*)n->inputs[0]->data;
						auto location_name = *(uint*)n->inputs[1]->data;
						if (auto [vtype, vdata] = find_var(name, location_name); vtype && vdata)
						{
							{
								Group::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							{
								Group::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<uint>();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->outputs[0]->object_id, data);

								node.outputs.push_back(data.attribute);
							}
						}
						return;
					}
					if (n->name_hash == "Array Get Item"_h)
					{
						auto name = *(uint*)n->inputs[0]->data;
						auto location_name = *(uint*)n->inputs[1]->data;
						if (auto [vtype, vdata] = find_var(name, location_name); vtype && vdata)
						{
							{
								Group::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[2].get()))
							{
								Group::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<uint>();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[2]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
							{
								Group::Data data;
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
						auto name = *(uint*)n->inputs[0]->data;
						auto location_name = *(uint*)n->inputs[1]->data;
						if (auto [vtype, vdata] = find_var(name, location_name); vtype && vdata)
						{
							{
								Group::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[2].get()))
							{
								Group::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<uint>();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[2]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[2].get()))
							{
								Group::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype->get_wrapped();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[2]->object_id, data);

								node.inputs.push_back(data.attribute);
							}
						}
					}
					if (n->name_hash == "Array Add Item"_h)
					{
						auto name = *(uint*)n->inputs[0]->data;
						auto location_name = *(uint*)n->inputs[1]->data;
						if (auto [vtype, vdata] = find_var(name, location_name); vtype && vdata)
						{
							{
								Group::Data data;
								data.changed_frame = frame;
								data.attribute.type = vtype;
								data.attribute.data = vdata;

								node.inputs.push_back(data.attribute);
							}
							if (!process_linked_input_slot(n->inputs[2].get()))
							{
								Group::Data data;
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
					if (n->name_hash == "Call"_h)
					{
						auto name = *(uint*)n->inputs[0]->data;
						auto location_name = *(uint*)n->inputs[1]->data;
						if (auto call_group = find_group(name, location_name); call_group)
						{
							{
								Group::Data data;
								data.changed_frame = frame;
								data.attribute.type = TypeInfo::get<PointerAndUint>();
								data.attribute.data = data.attribute.type->create();
								slots_data.emplace(n->inputs[0]->object_id, data);

								node.inputs.push_back(data.attribute);

								auto& pau = *(PointerAndUint*)data.attribute.data;
								pau.p = this;
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
						n->constructor(node.inputs.data(), node.outputs.data());
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
				std::map<uint, Group::Data> new_slot_datas;
				g.second.root_node.children.clear();
				create_group_structure(src_g, g.second, new_slot_datas);
				for (auto& d : new_slot_datas)
				{
					if (auto it = g.second.slot_datas.find(d.first); it != g.second.slot_datas.end())
					{
						if (it->second.attribute.type == d.second.attribute.type && it->second.changed_frame >= d.second.changed_frame)
						{
							if (d.second.attribute.type && d.second.attribute.type != TypeInfo::get<voidptr>() && d.second.attribute.type->tag != TagPU)
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
								assert(i->type == it->second.attribute.type);
								i->type->copy(it->second.attribute.data, i->data);
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

			auto& g = groups.emplace(src_g->name_hash, Group()).first->second;
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
					std::function<void(std::vector<Node*>)> find_node;
					find_node = [&](std::vector<Node*> stack) {
						for (auto& c : stack.back()->children)
						{
							if (c.object_id == id)
							{
								for (auto b : stack)
								{
									auto child_idx = 0;
									auto executed_times = 0;
									auto max_execute_times = 0;

									for (auto& b2 : it->second)
									{
										if (b2.node->object_id == b->object_id)
										{
											child_idx = b2.child_index;
											executed_times = b2.executed_times;
											max_execute_times = b2.max_execute_times;
											break;
										}
									}
									g.second.executing_stack.emplace_back(b, child_idx, executed_times, max_execute_times);
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

	void BlueprintInstancePrivate::prepare_executing(Group* group)
	{
		assert(group->instance == this);
		if (built_frame < blueprint->dirty_frame)
			build();

		if (!group->executing_stack.empty())
		{
			if (group->executing_stack.front().node->object_id == group->root_node.object_id)
				return;
			group->executing_stack.clear();
		}
		group->executing_stack.emplace_back(&group->root_node, 0, 0, 1);

		if (group->executing_stack.front().node->children.empty())
			group->executing_stack.clear();
		else
		{
			for (auto h : loop_index_hashes)
				group->set_variable(h, 0U);
		}
	}

	void BlueprintInstancePrivate::run(Group* group)
	{
		while (!group->executing_stack.empty())
		{
			if (auto debugger = BlueprintDebugger::current(); debugger && debugger->debugging)
				return;
			step(group);
		}
	}

	void BlueprintInstancePrivate::step(Group* group)
	{
		if (built_frame < blueprint->dirty_frame)
			build();

		auto debugger = BlueprintDebugger::current();
		if (debugger && debugger->debugging)
			return;
		if (group->executing_stack.empty())
			return;

		auto set_loop_index = [group]() {
			auto& current_block = group->executing_stack.back();
			auto depth = current_block.node->original->depth;
			if (depth - 1 < countof(loop_index_names))
				group->set_variable(loop_index_hashes[depth - 1], current_block.executed_times);
		};

		auto frame = frames;
		{
			auto& current_block = group->executing_stack.back();
			auto& current_node = current_block.node->children[current_block.child_index];

			auto node = current_node.original;
			if (debugger && debugger->has_break_node(node))
			{
				debugger->debugging = group;
				printf("Blueprint break node triggered: %s\n", node->name.c_str());
				return;
			}
			if (node->function)
				node->function(current_node.inputs.data(), current_node.outputs.data());
			if (node->is_block)
			{
				if (*(uint*)current_node.inputs[0].data)
				{
					uint max_execute_times = *(uint*)current_node.inputs[0].data;
					if (node->begin_block_function)
						node->begin_block_function(current_node.inputs.data(), current_node.outputs.data(), &max_execute_times);
					if (max_execute_times > 0)
					{
						group->executing_stack.emplace_back(&current_node, -1, 0, max_execute_times);
						set_loop_index();
					}
					*(uint*)current_node.outputs[0].data = 1;
				}
				else
					*(uint*)current_node.outputs[0].data = 0;
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
			current_block.executed_times++;
			if (group->executing_stack.size() > 1 && // not the root block
				current_block.executed_times < current_block.max_execute_times)
			{
				current_block.child_index = 0;
				set_loop_index();
				break;
			}
			group->executing_stack.pop_back();
		}
	}

	void BlueprintInstancePrivate::stop(Group* group)
	{
		auto debugger = BlueprintDebugger::current();
		if (debugger && debugger->debugging)
			return;
		
		group->executing_stack.clear();
	}

	void BlueprintInstancePrivate::call(uint group_name, void** inputs, void** outputs)
	{
		auto g = get_group(group_name);
		if (!g)
		{
			printf("blueprint call: cannot find group: %d\n", group_name);
			return;
		}

		if (auto obj = g->input_node; obj)
		{
			for (auto i = 0; i < obj->outputs.size(); i++)
			{
				auto& slot = obj->outputs[i];
				slot.type->copy(slot.data, inputs[i]);
			}
		}

		prepare_executing(g);
		run(g);

		if (auto obj = g->output_node; obj)
		{
			for (auto i = 0; i < obj->inputs.size(); i++)
			{
				auto& slot = obj->inputs[i];
				slot.type->copy(outputs[i], slot.data);
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

	void BlueprintDebuggerPrivate::add_break_node(BlueprintNodePtr node)
	{
		if (!has_break_node(node))
			break_nodes.emplace_back(node, true);
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
