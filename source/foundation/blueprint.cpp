#include "../xml.h"
#include "typeinfo_serialize.h"
#include "blueprint_private.h"

namespace flame
{
	std::vector<std::unique_ptr<BlueprintT>>			loaded_blueprints;
	std::vector<std::unique_ptr<BlueprintNodeLibraryT>> loaded_blueprint_node_libraries;
	static BlueprintNodeLibraryPtr standard_library = nullptr;

	BlueprintPrivate::BlueprintPrivate()
	{
		dirty_frame = frames;
	}

	BlueprintNodePtr BlueprintPrivate::add_node(BlueprintGroupPtr group, BlueprintBlockPtr block, const std::string& name,
		const std::vector<BlueprintSlotDesc>& inputs, const std::vector<BlueprintSlotDesc>& outputs,
		BlueprintNodeFunction function, BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeInputSlotChangedCallback input_slot_changed_callback, BlueprintNodePreviewProvider preview_provider)
	{
		assert(group && group->blueprint == this);
		if (block)
			assert(group == block->group);
		else
			block = group->blocks.front().get();

		auto ret = new BlueprintNodePrivate;
		ret->group = group;
		ret->block = block;
		ret->object_id = next_object_id++;
		ret->name = name;
		ret->name_hash = sh(name.c_str());
		for (auto& src_i : inputs)
		{
			auto i = new BlueprintSlotPrivate;
			i->parent = ret;
			i->object_id = next_object_id++;
			i->name = src_i.name;
			i->name_hash = src_i.name_hash;
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
			o->parent = ret;
			o->object_id = next_object_id++;
			o->name = src_o.name;
			o->name_hash = src_o.name_hash;
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
		ret->block->nodes.push_back(ret);
		group->nodes.emplace_back(ret);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	BlueprintNodePtr BlueprintPrivate::add_input_node(BlueprintGroupPtr group, BlueprintBlockPtr block, uint name)
	{
		assert(group && group->blueprint == this);
		if (block)
			assert(group == block->group);
		else
			block = group->blocks.front().get();

		BlueprintSlotPtr input = nullptr;
		for (auto& i : group->inputs)
		{
			if (i->name_hash == name)
			{
				input = i.get();
				break;
			}
		}

		if (!input)
		{
			printf("blueprint add_input_node: cannot find input\n");
			return nullptr;
		}

		auto ret = new BlueprintNodePrivate;
		ret->group = group;
		ret->block = block;
		ret->object_id = next_object_id++;
		ret->name = input->name;
		ret->name_hash = sh(ret->name.c_str());
		{
			auto o = new BlueprintSlotPrivate;
			o->parent = ret;
			o->object_id = next_object_id++;
			o->name = "Out";
			o->name_hash = "Out"_h;
			o->allowed_types.push_back(input->type);
			o->type = input->type;
			ret->outputs.emplace_back(o);
		}
		ret->block->nodes.push_back(ret);
		group->nodes.emplace_back(ret);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	BlueprintNodePtr BlueprintPrivate::add_variable_node(BlueprintGroupPtr group, BlueprintBlockPtr block, uint variable_group_name)
	{
		assert(group && group->blueprint == this);
		if (block)
			assert(group == block->group);
		else
			block = group->blocks.front().get();

		auto variable_group = find_group(variable_group_name);
		assert(variable_group);
		if (!variable_group)
		{
			printf("blueprint add_variable_node: cannot find variable group\n");
			return nullptr;
		}

		auto ret = new BlueprintNodePrivate;
		ret->group = group;
		ret->block = block;
		ret->object_id = next_object_id++;
		ret->name = variable_group->name;
		ret->name_hash = sh(ret->name.c_str());
		if (!variable_group->outputs.empty())
		{
			auto& src_o = variable_group->outputs.front();
			auto o = new BlueprintSlotPrivate;
			o->parent = ret;
			o->object_id = next_object_id++;
			o->name = "Out";
			o->name_hash = "Out"_h;
			o->allowed_types.push_back(src_o->type);
			o->type = src_o->type;
			ret->outputs.emplace_back(o);

		}
		ret->function = [](BlueprintArgument* inputs, BlueprintArgument* outputs) {

		};
		ret->block->nodes.push_back(ret);
		group->nodes.emplace_back(ret);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	void BlueprintPrivate::remove_node(BlueprintNodePtr node)
	{
		auto group = node->group;
		auto block = node->block;
		assert(group->blueprint == this);
		for (auto it = group->nodes.begin(); it != group->nodes.end(); it++)
		{
			if (it->get() == node)
			{
				group->nodes.erase(it);
				break;
			}
		}
		for (auto it = block->nodes.begin(); it != block->nodes.end(); it++)
		{
			if (*it == node)
			{
				block->nodes.erase(it);
				break;
			}
		}
		for (auto it = group->links.begin(); it != group->links.end();)
		{
			auto link = it->get();
			if (link->from_slot->parent.p.node == node)
			{
				std::erase_if(link->to_slot->linked_slots, [&](const auto& slot) {
					return slot == link->from_slot;
				});
				it = group->links.erase(it);
			}
			else if (link->to_slot->parent.p.node == node)
			{
				std::erase_if(link->from_slot->linked_slots, [&](const auto& slot) {
					return slot == link->to_slot;
				});
				it = group->links.erase(it);
			}
			else
				it++;
		}
		if (auto debugger = BlueprintDebugger::current(); debugger)
			debugger->remove_break_node(node);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	void BlueprintPrivate::change_node_block(BlueprintNodePtr node, BlueprintBlockPtr new_block)
	{
		auto group = node->group;
		assert(group && group->blueprint == this && group == new_block->group);

		if (node->block == new_block)
			return;

		auto old_block = node->block;
		for (auto it = old_block->nodes.begin(); it != old_block->nodes.end(); it++)
		{
			if (*it == node)
			{
				old_block->nodes.erase(it);
				break;
			}
		}

		node->block = new_block;

		new_block->nodes.push_back(node);
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
		if (!(slot->type && slot->type->tag == TagPU && new_type->tag == TagU)) // if a udt type link to its pointer type, dont change the type
		{
			slot->type = new_type;
			if (new_type && has_data) // is input slot and has data
				slot->data = new_type->create();
		}
	}

	static void clear_invalid_links(BlueprintGroupPtr group)
	{
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
					group->blueprint->remove_link(l.get());
					done = false;
					break;
				}
			}
		}
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

								auto node = l->to_slot->parent.p.node;
								if (std::find(input_changed_nodes.begin(), input_changed_nodes.end(), node) == input_changed_nodes.end())
									input_changed_nodes.push_back(node);
							}
						}
					}
				}
			}
		}

		clear_invalid_links(n->group);
	}

	BlueprintLinkPtr BlueprintPrivate::add_link(BlueprintSlotPtr from_slot, BlueprintSlotPtr to_slot)
	{
		auto group = from_slot->parent.get_locate_group();
		assert(group && group->blueprint == this && group == to_slot->parent.get_locate_group());

		if (!from_slot || !to_slot)
			return nullptr;
		if (from_slot == to_slot)
			return nullptr;
		if (from_slot->parent.get_locate_block()->depth > to_slot->parent.get_locate_block()->depth)
			return nullptr;

		if (!blueprint_allow_type(to_slot->allowed_types, from_slot->type))
			return nullptr;

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
		if (to_slot->parent.type == BlueprintObjectNode)
			update_node_output_types(to_slot->parent.p.node);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	void BlueprintPrivate::remove_link(BlueprintLinkPtr link)
	{
		auto group = link->from_slot->parent.get_locate_group();
		assert(group && group->blueprint == this);

		for (auto it = group->links.begin(); it != group->links.end(); it++)
		{
			if (it->get() == link)
			{
				auto from_slot = link->from_slot;
				auto to_object = link->to_slot->parent;
				auto to_slot = link->to_slot;
				std::erase_if(from_slot->linked_slots, [&](const auto& slot) {
					return slot == to_slot;
				});
				std::erase_if(to_slot->linked_slots, [&](const auto& slot) {
					return slot == from_slot;
				});
				group->links.erase(it);
				change_slot_type(to_slot, !to_slot->allowed_types.empty() ? to_slot->allowed_types.front() : nullptr);
				if (to_object.type == BlueprintObjectNode)
					update_node_output_types(to_object.p.node);
				break;
			}
		}

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	BlueprintBlockPtr BlueprintPrivate::add_block(BlueprintGroupPtr group, BlueprintBlockPtr parent)
	{
		assert(group && group->blueprint == this);
		if (parent)
			assert(parent->group == group);
		else if (!group->blocks.empty())
			parent = group->blocks.front().get();

		auto ret = new BlueprintBlockPrivate;
		ret->group = group;
		ret->parent = parent;
		ret->object_id = next_object_id++;
		if (parent)
		{
			parent->children.push_back(ret);
			ret->depth = parent->depth + 1;
		}
		else
			ret->depth = 0;
		ret->input.reset(new BlueprintSlotPrivate);
		ret->input->parent = ret;
		ret->input->object_id = next_object_id++;
		ret->input->name = "Execute";
		ret->input->name_hash = "Execute"_h;
		ret->input->allowed_types.push_back(TypeInfo::get<Signal>());
		ret->input->type = ret->input->allowed_types.front();
		ret->output.reset(new BlueprintSlotPrivate);
		ret->output->parent = ret;
		ret->output->object_id = next_object_id++;
		ret->output->name = "Execute";
		ret->output->name_hash = "Execute"_h;
		ret->output->allowed_types.push_back(TypeInfo::get<Signal>());
		ret->output->type = ret->output->allowed_types.front();

		group->blocks.emplace_back(ret);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	void BlueprintPrivate::remove_block(BlueprintBlockPtr block)
	{
		auto group = block->group;
		assert(group && group->blueprint == this);

		std::function<void(BlueprintBlockPtr)> destroy_block;
		destroy_block = [&](BlueprintBlockPtr b) {
			for (auto& c : b->children)
				destroy_block(c);
			for (auto n : b->nodes)
				remove_node(n);
			for (auto it = group->blocks.begin(); it != group->blocks.end(); it++)
			{
				if (it->get() == b)
				{
					group->blocks.erase(it);
					return;
				}
			}
		};

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	void BlueprintPrivate::change_block_parent(BlueprintBlockPtr block, BlueprintBlockPtr new_parent)
	{
		auto group = block->group;
		assert(group && group->blueprint == this && group == new_parent->group);

		if (block->parent == new_parent)
			return;

		auto old_block = block->parent;
		for (auto it = old_block->children.begin(); it != old_block->children.end(); it++)
		{
			if (*it == block)
			{
				old_block->children.erase(it);
				break;
			}
		}

		block->parent = new_parent;
		block->depth = new_parent->depth + 1;

		new_parent->children.push_back(block);
	}

	BlueprintGroupPtr BlueprintPrivate::add_group(const std::string& name)
	{
		auto g = new BlueprintGroupPrivate;
		g->blueprint = this;
		g->object_id = next_object_id++;
		g->name = name;
		g->name_hash = sh(name.c_str());
		groups.emplace_back(g);

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

	BlueprintSlotPtr BlueprintPrivate::add_group_input(BlueprintGroupPtr group, const std::string& name, TypeInfo* type)
	{
		assert(group && group->blueprint == this);

		for (auto& i : group->inputs)
		{
			if (i->name == name)
			{
				printf("blueprint add_group_input: input already exists\n");
				return nullptr;
			}
		}

		auto ret = new BlueprintSlotPrivate;
		ret->parent = group;
		ret->object_id = next_object_id++;
		ret->name = name;
		ret->name_hash = sh(name.c_str());
		ret->type = type;
		ret->data = type->create();
		group->inputs.emplace_back(ret);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	void BlueprintPrivate::remove_group_input(BlueprintGroupPtr group, BlueprintSlotPtr slot)
	{
		assert(group && group->blueprint == this);

		for (auto it = group->inputs.begin(); it != group->inputs.end(); it++)
		{
			if (it->get() == slot)
			{
				group->inputs.erase(it);
				break;
			}
		}

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	BlueprintSlotPtr BlueprintPrivate::add_group_output(BlueprintGroupPtr group, const std::string& name, TypeInfo* type)
	{
		assert(group && group->blueprint == this);

		for (auto& o : group->outputs)
		{
			if (o->name == name)
			{
				printf("blueprint add_group_output: output already exists\n");
				return nullptr;
			}
		}

		auto ret = new BlueprintSlotPrivate;
		ret->parent = group;
		ret->object_id = next_object_id++;
		ret->name = name;
		ret->name_hash = sh(name.c_str());
		ret->type = type;
		group->outputs.emplace_back(ret);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	void BlueprintPrivate::remove_group_output(BlueprintGroupPtr group, BlueprintSlotPtr slot)
	{
		assert(group && group->blueprint == this);

		for (auto it = group->outputs.begin(); it != group->outputs.end(); it++)
		{
			if (it->get() == slot)
			{
				group->outputs.erase(it);
				break;
			}
		}

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
		auto n_groups = doc_root.append_child("groups");
		for (auto& g : groups)
		{
			auto n_group = n_groups.append_child("group");
			n_group.append_attribute("object_id").set_value(g->object_id);
			n_group.append_attribute("name").set_value(g->name.c_str());
			auto n_nodes = n_group.append_child("nodes");
			for (auto& n : g->nodes)
			{
				auto n_node = n_nodes.append_child("node");
				n_node.append_attribute("object_id").set_value(n->object_id);
				n_node.append_attribute("block_id").set_value(n->block ? n->block->object_id : 0);
				n_node.append_attribute("name").set_value(n->name.c_str());
				n_node.append_attribute("position").set_value(str(n->position).c_str());
			}
			auto n_blocks = n_group.append_child("blocks");
			for (auto& b : g->blocks)
			{
				auto n_block = n_blocks.append_child("block");
				n_block.append_attribute("object_id").set_value(b->object_id);
				n_block.append_attribute("parent_id").set_value(b->parent ? b->parent->object_id : 0);
				n_block.append_attribute("position").set_value(str(b->position).c_str());
			}
			auto n_links = n_group.append_child("links");
			for (auto& l : g->links)
			{
				auto n_link = n_links.append_child("link");
				n_link.append_attribute("object_id").set_value(l->object_id);
				n_link.append_attribute("from_object").set_value(l->from_slot->parent.get_id());
				n_link.append_attribute("from_slot").set_value(l->from_slot->name_hash);
				n_link.append_attribute("to_object").set_value(l->to_slot->parent.get_id());
				n_link.append_attribute("to_slot").set_value(l->to_slot->name_hash);
			}
			auto n_inputs = n_group.append_child("inputs");
			for (auto& i : g->inputs)
			{
				auto n_input = n_inputs.append_child("input");
				n_input.append_attribute("object_id").set_value(i->object_id);
				n_input.append_attribute("name").set_value(i->name.c_str());
				write_ti(i->type, n_input.append_attribute("type"));
			}
			auto n_outputs = n_group.append_child("outputs");
			for (auto& o : g->outputs)
			{
				auto n_output = n_outputs.append_child("output");
				n_output.append_attribute("object_id").set_value(o->object_id);
				n_output.append_attribute("name").set_value(o->name.c_str());
				write_ti(o->type, n_output.append_attribute("type"));
			}
		}

		if (!path.empty())
			filename = path;
		doc.save_file(filename.c_str());
	}

	struct BlueprintGet : Blueprint::Get
	{
		BlueprintPtr operator()(const std::filesystem::path& _filename) override
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

				for (auto n_group : doc_root.child("groups"))
				{
					auto g = ret->add_group(n_group.attribute("name").value());

					std::map<uint, BlueprintObject> object_map;

					for (auto n_block : n_group.child("blocks"))
					{
						auto parent_id = n_block.attribute("parent_id").as_uint();
						auto b = ret->add_block(g, parent_id ? object_map[parent_id].p.block : nullptr);
						object_map[n_block.attribute("object_id").as_uint()] = b;
						b->position = s2t<2, float>(n_block.attribute("position").value());
					}
					for (auto n_node : n_group.child("nodes"))
					{
						std::string name = n_node.attribute("name").value();
						auto added = false;
						for (auto& library : loaded_blueprint_node_libraries)
						{
							for (auto& t : library->node_templates)
							{
								if (t.name == name)
								{
									auto block_id = n_node.attribute("block_id").as_uint();
									auto block = block_id ? object_map[block_id].p.block : nullptr;
									auto n = ret->add_node(g, block, t.name, t.inputs, t.outputs,
										t.function, t.constructor, t.destructor, t.input_slot_changed_callback, t.preview_provider);
									object_map[n_node.attribute("object_id").as_uint()] = n;
									n->position = s2t<2, float>(n_node.attribute("position").value());
									added = true;
									break;
								}
							}
							if (added)
								break;
						}
					}
					for (auto n_link : n_group.child("links"))
					{
						auto from_object = object_map[n_link.attribute("from_object").as_uint()];
						auto from_slot = n_link.attribute("from_slot").as_uint();
						auto to_object = object_map[n_link.attribute("to_object").as_uint()];
						auto to_slot = n_link.attribute("to_slot").as_uint();
						ret->add_link(from_object.find_output(from_slot), to_object.find_input(to_slot));
					}
					for (auto n_input : n_group.child("inputs"))
					{

					}
					for (auto n_output : n_group.child("outputs"))
					{

					}
				}
			}
			else
			{
				ret->add_group("main");
				ret->save(filename);
			}

			ret->filename = filename;
			ret->ref = 1;
			loaded_blueprints.emplace_back(ret);
			return ret;
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

	void BlueprintNodeLibraryPrivate::add_template(const std::string& name, const std::vector<BlueprintSlotDesc>& inputs, const std::vector<BlueprintSlotDesc>& outputs,
		BlueprintNodeFunction function, BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeInputSlotChangedCallback input_slot_changed_callback, BlueprintNodePreviewProvider preview_provider)
	{
		auto& t = node_templates.emplace_back();
		t.name = name;
		t.name_hash = sh(name.c_str());
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
	}

	struct BlueprintNodeLibraryGet : BlueprintNodeLibrary::Get
	{
		BlueprintNodeLibraryPtr operator()(const std::filesystem::path& _filename) override
		{
			auto filename = Path::get(_filename);
			if (filename.empty())
				filename = _filename;

			for (auto& lib : loaded_blueprint_node_libraries)
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
			loaded_blueprint_node_libraries.emplace_back(ret);
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

	static void destroy_group(BlueprintInstance::Group& g)
	{
		std::function<void(BlueprintInstance::Object&)> destroy_object;
		destroy_object = [&](BlueprintInstance::Object& o) {
			if (o.original.type == BlueprintObjectNode && o.original.p.node->destructor)
				o.original.p.node->destructor(o.inputs.data(), o.outputs.data());
			for (auto& c : o.children)
				destroy_object(c);
		};
		destroy_object(g.root_object);
		for (auto& pair : g.datas)
		{
			if (pair.second.arg.data)
				pair.second.arg.type->destroy(pair.second.arg.data);
		}
	}

	BlueprintInstancePrivate::~BlueprintInstancePrivate()
	{
		for (auto& g : groups)
			destroy_group(g.second);
		Blueprint::release(blueprint);
	}

	void BlueprintInstancePrivate::build()
	{
		auto frame = frames;
		auto old_executing_stack = executing_stack;
		auto old_executing_object_id = executing_stack.empty() ? 0 : executing_object()->object_id;

		auto create_group_structure = [&](BlueprintGroupPtr src_g, Group& g, std::map<uint, Group::Data>& datas) {
			std::function<void(BlueprintBlockPtr, Object&)> create_object;
			create_object = [&](BlueprintBlockPtr block, Object& o) {
				for (auto& b : block->children)
				{
					auto& c = o.children.emplace_back();
					c.original = b;
					create_object(b, c);
				}
				std::vector<BlueprintObject> rest_objects;
				for (auto n : block->nodes)
					rest_objects.emplace_back(n);
				for (auto b : block->children)
					rest_objects.emplace_back(b);
				auto is_sub = [](BlueprintBlockPtr b1, BlueprintBlockPtr b2) {
					while (b2)
					{
						if (b1 == b2)
							return true;
						b2 = b2->parent;
					}
					return false;
				};
				while (!rest_objects.empty())
				{
					for (auto it = rest_objects.begin(); it != rest_objects.end();)
					{
						auto ok = true;
						for (auto& l : src_g->links)
						{
							if (l->from_slot->parent.get_locate_block() == block)
							{
								auto from_object = l->from_slot->parent;
								auto to_object = l->to_slot->parent;
								auto ok2 = false;
								switch (to_object.type)
								{
								case BlueprintObjectNode:
									ok2 = it->p.node == to_object.p.node;
									break;
								case BlueprintObjectBlock:
									ok2 = is_sub(it->p.block, to_object.get_locate_block());
									break;
								}
								if (ok2)
								{
									if (std::find_if(rest_objects.begin(), rest_objects.end(), [&](const auto& i) {
										return i.get_id() == from_object.get_id();
										}) != rest_objects.end())
									{
										ok = false;
										break;
									}
								}
							}
						}
						if (ok)
						{
							auto& new_o = o.children.emplace_back();
							auto& original = *it;
							new_o.original = original;
							new_o.object_id = original.get_id();
							it = rest_objects.erase(it);
						}
						else
							it++;
					}
				}
			};
			create_object(src_g->blocks.front().get(), g.root_object);

			g.object_map.clear();
			std::function<void(Object&)> create_map;
			create_map = [&](Object& o) {
				if (o.object_id)
					g.object_map[o.object_id] = &o;
				for (auto& c : o.children)
					create_map(c);
			};
			create_map(g.root_object);

			// create datas
			std::function<void(Object&)> create_datas;
			create_datas = [&](Object& obj) {
				for (auto i : obj.original.get_inputs())
				{
					auto linked = false;
					for (auto& l : src_g->links)
					{
						if (l->to_slot == i)
						{
							if (auto it = datas.find(l->from_slot->object_id); it != datas.end())
							{
								if (i->type->tag == TagPU && it->second.arg.type->tag == TagU)
								{
									Group::Data data;
									data.changed_frame = it->second.changed_frame;
									data.arg.type = i->type;
									data.arg.data = i->type->create();
									auto ptr = it->second.arg.data;
									memcpy((voidptr*)data.arg.data, &ptr, sizeof(voidptr));
									datas.emplace(i->object_id, data);

									obj.inputs.push_back(data.arg);
								}
								else
								{
									BlueprintArgument arg;
									arg.type = it->second.arg.type;
									arg.data = it->second.arg.data;
									obj.inputs.push_back(arg);
								}
							}
							else
								assert(0);
							linked = true;
						}
					}
					if (!linked)
					{
						Group::Data data;
						data.changed_frame = i->data_changed_frame;
						data.arg.type = i->type;
						if (data.arg.type)
						{
							data.arg.data = data.arg.type->create();
							if (i->data)
								data.arg.type->copy(data.arg.data, i->data);
						}
						else
							data.arg.data = nullptr;
						datas.emplace(i->object_id, data);

						obj.inputs.push_back(data.arg);
					}
				}
				for (auto& o : obj.original.get_outputs())
				{
					Group::Data data;
					data.changed_frame = o->data_changed_frame;
					data.arg.type = o->type;
					if (data.arg.type)
					{
						data.arg.data = data.arg.type->create();
						if (data.arg.type->tag == TagPU)
							memset(data.arg.data, 0, sizeof(voidptr));
					}
					else
						data.arg.data = nullptr;
					datas.emplace(o->object_id, data);

					obj.outputs.push_back(data.arg);
				}

				if (obj.original.type == BlueprintObjectNode)
				{
					if (obj.original.p.node->constructor)
						obj.original.p.node->constructor(obj.inputs.data(), obj.outputs.data());
				}
				for (auto& c : obj.children)
					create_datas(c);
			};
			create_datas(g.root_object);
		};

		// remove groups that are not in the blueprint anymore
		for (auto it = groups.begin(); it != groups.end();)
		{
			if (!blueprint->find_group(it->first))
			{
				destroy_group(it->second);
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
				std::map<uint, Group::Data> new_datas;
				g.second.root_object.children.clear();
				create_group_structure(src_g, g.second, new_datas);
				for (auto& d : new_datas)
				{
					if (auto it = g.second.datas.find(d.first); it != g.second.datas.end())
					{
						if (it->second.arg.type == d.second.arg.type && it->second.changed_frame >= d.second.changed_frame)
						{
							if (d.second.arg.type->tag != TagPU)
								d.second.arg.type->copy(d.second.arg.data, it->second.arg.data);
							d.second.changed_frame = it->second.changed_frame;
						}
					}
				}
				for (auto& pair : g.second.datas)
				{
					if (pair.second.arg.data)
						pair.second.arg.type->destroy(pair.second.arg.data);
				}
				g.second.datas = std::move(new_datas);
				g.second.structure_updated_frame = frame;
				g.second.data_updated_frame = frame;
			}
			else if (src_g->data_changed_frame > g.second.data_updated_frame)
			{
				for (auto& n : src_g->nodes)
				{
					for (auto& i : n->inputs)
					{
						if (auto it = g.second.datas.find(i->object_id); it != g.second.datas.end())
						{
							if (i->data_changed_frame > it->second.changed_frame)
							{
								assert(i->type == it->second.arg.type);
								i->type->copy(it->second.arg.data, i->data);
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
			g.name = src_g->name_hash;
			create_group_structure(src_g.get(), g, g.datas);
			g.structure_updated_frame = frame;
			g.data_updated_frame = frame;
		}

		executing_stack.clear();
		if (!old_executing_stack.empty() && old_executing_object_id)
		{
			auto root_object_id = old_executing_stack.front().block_object->object_id;
			for (auto it = groups.begin(); it != groups.end(); it++)
			{
				if (it->second.root_object.object_id == root_object_id)
				{
					std::function<void(std::vector<Object*>)> find_object;
					find_object = [&](std::vector<Object*> stack) {
						for (auto& c : stack.back()->children)
						{
							if (c.object_id == old_executing_object_id)
							{
								for (auto b : stack)
								{
									auto child_idx = 0;
									auto times = 0;

									for (auto& b2 : old_executing_stack)
									{
										if (b2.block_object->object_id == b->object_id)
										{
											child_idx = b2.child_index;
											times = b2.executed_times;
											break;
										}
									}
									executing_stack.emplace_back(b, child_idx, times);
								}
								return;
							}
							if (!c.children.empty())
							{
								auto s = stack;
								s.push_back(&c);
								find_object(s);
							}
						}
					};

					break;
				}
			}
		}

		built_frame = frames;
	}

	void BlueprintInstancePrivate::prepare_executing(Group* group)
	{
		assert(group->instance == this);
		if (built_frame < blueprint->dirty_frame)
		{
			auto name = group->name;
			executing_stack.clear();
			build();
			if (auto it = groups.find(name); it != groups.end())
				executing_stack.emplace_back(&it->second.root_object, 0, 0);
		}
		else
			executing_stack.emplace_back(&group->root_object, 0, 0);
		if (executing_stack.front().block_object->children.empty())
			executing_stack.clear();
	}

	void BlueprintInstancePrivate::run()
	{
		while (!executing_stack.empty())
		{
			if (auto debugger = BlueprintDebugger::current(); debugger && debugger->debugging)
				return;
			step();
		}
	}

	void BlueprintInstancePrivate::step()
	{
		if (built_frame < blueprint->dirty_frame)
			build();

		auto debugger = BlueprintDebugger::current();
		if (debugger && debugger->debugging)
			return;
		if (executing_stack.empty())
			return;

		auto& current_block = executing_stack.back();
		auto& current_object = current_block.block_object->children[current_block.child_index];
		switch (current_object.original.type)
		{
		case BlueprintObjectNode:
		{
			auto node = current_object.original.p.node;
			if (debugger && debugger->has_break_node(node))
			{
				debugger->debugging = this;
				return;
			}
			if (node->function)
				node->function(current_object.inputs.data(), current_object.outputs.data());
		}
			break;
		case BlueprintObjectBlock:
		{
			if (*(uint*)current_object.inputs[0].data)
			{
				executing_stack.emplace_back(&current_object, 0, 0);
				*(uint*)current_object.outputs[0].data = 1;
			}
			else
				*(uint*)current_object.outputs[0].data = 0;
		}
			break;
		}
		current_object.updated_frame = frames;

		while (true)
		{
			if (executing_stack.empty())
				break;
			auto& current_block = executing_stack.back();
			current_block.child_index++;
			if (current_block.child_index < current_block.block_object->children.size())
				break;
			current_block.executed_times++;
			if (!current_block.block_object->inputs.empty() && // not the root block
				current_block.executed_times < *(uint*)current_block.block_object->inputs[0].data)
			{
				current_block.child_index = 0;
				break;
			}
			executing_stack.pop_back();
			break;
		}
	}

	void BlueprintInstancePrivate::stop()
	{
		auto debugger = BlueprintDebugger::current();
		if (debugger && debugger->debugging)
			return;

		executing_stack.clear();
	}

	struct BlueprintInstanceCreate : BlueprintInstance::Create
	{
		BlueprintInstancePtr operator()(BlueprintPtr blueprint) override
		{
			return new BlueprintInstancePrivate(blueprint);
		}
	}BlueprintInstance_create;
	BlueprintInstance::Create& BlueprintInstance::create = BlueprintInstance_create;

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
