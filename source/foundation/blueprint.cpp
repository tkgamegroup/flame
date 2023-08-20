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

	static void init_node(BlueprintNodePtr n)
	{
		auto bp = n->group->blueprint;
		for (auto& i : n->inputs)
		{
			i.node = n;
			i.object_id = bp->next_object_id++;
			i.name_hash = sh(i.name.c_str());
			if (!i.allowed_types.empty())
			{
				i.type = i.allowed_types.front();
				i.data = i.type->create();
				if (!i.default_value.empty())
					i.type->unserialize(i.default_value, i.data);
			}
		}
		for (auto& o : n->outputs)
		{
			o.node = n;
			o.object_id = bp->next_object_id++;
			o.name_hash = sh(o.name.c_str());
			if (!o.allowed_types.empty())
				o.type = o.allowed_types.front();
		}
	}

	BlueprintNodePtr BlueprintPrivate::add_node(BlueprintGroupPtr group /*null means the main group*/, const std::string& name,
		const std::vector<BlueprintSlot>& inputs, const std::vector<BlueprintSlot>& outputs,
		BlueprintNodeFunction function, BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeInputSlotChangedCallback input_slot_changed_callback, BlueprintNodePreviewProvider preview_provider)
	{
		if (!group)
			group = groups[0].get();
		assert(group->blueprint == this);

		auto ret = new BlueprintNodePrivate;
		ret->group = group;
		ret->object_id = next_object_id++;
		ret->name = name;
		ret->name_hash = sh(name.c_str());
		ret->inputs = inputs;
		ret->outputs = outputs;
		ret->constructor = constructor;
		ret->destructor = destructor;
		ret->function = function;
		ret->input_slot_changed_callback = input_slot_changed_callback;
		ret->preview_provider = preview_provider;
		init_node(ret);
		group->nodes.emplace_back(ret);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	BlueprintNodePtr BlueprintPrivate::add_input_node(BlueprintGroupPtr group, uint name)
	{
		if (!group)
			group = groups[0].get();
		assert(group->blueprint == this);

		BlueprintGroupSlotPtr input = nullptr;
		for (auto& i : group->inputs)
		{
			if (i.name_hash == name)
			{
				input = &i;
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
		ret->object_id = next_object_id++;
		ret->name = input->name;
		ret->name_hash = sh(ret->name.c_str());
		{
			auto& o = ret->outputs.emplace_back();
			o.name = "Out";
			o.allowed_types.push_back(input->type);
		}

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	BlueprintNodePtr BlueprintPrivate::add_variable_node(BlueprintGroupPtr group, uint variable_group_name)
	{
		if (!group)
			group = groups[0].get();
		assert(group->blueprint == this);

		auto variable_group = find_group(variable_group_name);
		assert(variable_group);
		if (!variable_group)
		{
			printf("blueprint add_variable_node: cannot find variable group\n");
			return nullptr;
		}

		auto ret = new BlueprintNodePrivate;
		ret->group = group;
		ret->object_id = next_object_id++;
		ret->name = variable_group->name;
		ret->name_hash = sh(ret->name.c_str());
		if (!variable_group->outputs.empty())
		{
			auto& src_o = variable_group->outputs.front();
			auto& o = ret->outputs.emplace_back();
			o.name = "Out";
			o.allowed_types.push_back(src_o.type);

		}
		ret->function = [](BlueprintArgument* inputs, BlueprintArgument* outputs) {

		};
		init_node(ret);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	void BlueprintPrivate::remove_node(BlueprintNodePtr node)
	{
		auto group = node->group;
		assert(group->blueprint == this);
		for (auto it = group->nodes.begin(); it != group->nodes.end(); it++)
		{
			if (it->get() == node)
			{
				group->nodes.erase(it);
				break;
			}
		}
		for (auto it = group->links.begin(); it != group->links.end();)
		{
			if ((*it)->from_node == node || (*it)->to_node == node)
				it = group->links.erase(it);
			else
				it++;
		}
		if (auto debugger = BlueprintDebugger::current(); debugger)
			debugger->remove_break_node(node);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
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
				input_types[i] = n->inputs[i].type;
			for (auto i = 0; i < output_types.size(); i++)
				output_types[i] = n->outputs[i].type;

			if (n->input_slot_changed_callback)
			{
				n->input_slot_changed_callback(input_types.data(), output_types.data());

				for (auto i = 0; i < output_types.size(); i++)
				{
					auto& slot = n->outputs[i];
					if (slot.type != output_types[i])
					{
						change_slot_type(&slot, slot.allow_type(output_types[i]) ? output_types[i] : nullptr);

						for (auto& l : n->group->links)
						{
							if (l->from_slot == &slot)
							{
								change_slot_type(l->from_slot, l->from_slot->allow_type(slot.type) ? slot.type : nullptr);

								if (std::find(input_changed_nodes.begin(), input_changed_nodes.end(), l->to_node) == input_changed_nodes.end())
									input_changed_nodes.push_back(l->to_node);
							}
						}
					}
				}
			}
		}

		clear_invalid_links(n->group);
	}

	BlueprintLinkPtr BlueprintPrivate::add_link(BlueprintNodePtr from_node, uint from_slot, BlueprintNodePtr to_node, uint to_slot)
	{
		auto group = from_node->group;
		assert(group->blueprint == this && group == to_node->group);

		auto from_slot_ptr = from_node->find_output(from_slot);
		auto to_slot_ptr = to_node->find_input(to_slot);
		if (!from_slot_ptr || !to_slot_ptr)
			return nullptr;
		if (from_slot_ptr == to_slot_ptr)
			return nullptr;

		if (!to_slot_ptr->allow_type(from_slot_ptr->type))
			return nullptr;

		for (auto& l : group->links)
		{
			if (l->to_slot == to_slot_ptr)
			{
				remove_link(l.get());
				break;
			}
		}

		auto ret = new BlueprintLinkPrivate;
		ret->object_id = next_object_id++;
		ret->from_node = from_node;
		ret->from_slot = from_node->find_output(from_slot);
		ret->to_node = to_node;
		ret->to_slot = to_node->find_input(to_slot);
		group->links.emplace_back(ret);

		change_slot_type(to_slot_ptr, from_slot_ptr->type);
		update_node_output_types(to_node);

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return ret;
	}

	void BlueprintPrivate::remove_link(BlueprintLinkPtr link)
	{
		auto group = link->from_node->group;
		assert(group->blueprint == this && group == link->to_node->group);

		for (auto it = group->links.begin(); it != group->links.end(); it++)
		{
			if (it->get() == link)
			{
				auto to_node = link->to_node;
				auto to_slot = link->to_slot;
				group->links.erase(it);
				change_slot_type(to_slot, !to_slot->allowed_types.empty() ? to_slot->allowed_types.front() : nullptr);
				update_node_output_types(to_node);
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

	BlueprintGroupSlotPtr BlueprintPrivate::add_group_input(BlueprintGroupPtr group, const std::string& name, TypeInfo* type)
	{
		assert(group && group->blueprint == this);

		for (auto& i : group->inputs)
		{
			if (i.name == name)
			{
				printf("blueprint add_group_input: input already exists\n");
				return nullptr;
			}
		}

		auto& ret = group->inputs.emplace_back();
		ret.group = group;
		ret.object_id = next_object_id++;
		ret.name = name;
		ret.name_hash = sh(name.c_str());
		ret.type = type;
		ret.data = type->create();

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return &ret;
	}

	void BlueprintPrivate::remove_group_input(BlueprintGroupPtr group, BlueprintGroupSlotPtr slot)
	{
		assert(group && group->blueprint == this);

		for (auto it = group->inputs.begin(); it != group->inputs.end(); it++)
		{
			if (&*it == slot)
			{
				group->inputs.erase(it);
				break;
			}
		}

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;
	}

	BlueprintGroupSlotPtr BlueprintPrivate::add_group_output(BlueprintGroupPtr group, const std::string& name, TypeInfo* type)
	{
		assert(group && group->blueprint == this);

		for (auto& o : group->outputs)
		{
			if (o.name == name)
			{
				printf("blueprint add_group_output: output already exists\n");
				return nullptr;
			}
		}

		auto& ret = group->outputs.emplace_back();
		ret.group = group;
		ret.object_id = next_object_id++;
		ret.name = name;
		ret.name_hash = sh(name.c_str());
		ret.type = type;

		auto frame = frames;
		group->structure_changed_frame = frame;
		dirty_frame = frame;

		return &ret;
	}

	void BlueprintPrivate::remove_group_output(BlueprintGroupPtr group, BlueprintGroupSlotPtr slot)
	{
		assert(group && group->blueprint == this);

		for (auto it = group->outputs.begin(); it != group->outputs.end(); it++)
		{
			if (&*it == slot)
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
				n_node.append_attribute("name").set_value(n->name.c_str());
				n_node.append_attribute("position").set_value(str(n->position).c_str());
			}
			auto n_links = n_group.append_child("links");
			for (auto& l : g->links)
			{
				auto n_link = n_links.append_child("link");
				n_link.append_attribute("object_id").set_value(l->object_id);
				n_link.append_attribute("from_node").set_value(l->from_node->object_id);
				n_link.append_attribute("from_slot").set_value(l->from_slot->name_hash);
				n_link.append_attribute("to_node").set_value(l->to_node->object_id);
				n_link.append_attribute("to_slot").set_value(l->to_slot->name_hash);
			}
			auto n_inputs = n_group.append_child("inputs");
			for (auto& i : g->inputs)
			{
				auto n_input = n_inputs.append_child("input");
				n_input.append_attribute("object_id").set_value(i.object_id);
				n_input.append_attribute("name").set_value(i.name.c_str());
				write_ti(i.type, n_input.append_attribute("type"));
			}
			auto n_outputs = n_group.append_child("outputs");
			for (auto& o : g->outputs)
			{
				auto n_output = n_outputs.append_child("output");
				n_output.append_attribute("object_id").set_value(o.object_id);
				n_output.append_attribute("name").set_value(o.name.c_str());
				write_ti(o.type, n_output.append_attribute("type"));
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

					std::map<uint, BlueprintNodePtr> node_map;

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
									auto n = ret->add_node(g, t.name, t.inputs, t.outputs,
										t.function, t.constructor, t.destructor, t.input_slot_changed_callback, t.preview_provider);
									node_map[n_node.attribute("object_id").as_uint()] = n;
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
						auto from_node = node_map[n_link.attribute("from_node").as_uint()];
						auto from_slot = n_link.attribute("from_slot").as_uint();
						auto to_node = node_map[n_link.attribute("to_node").as_uint()];
						auto to_slot = n_link.attribute("to_slot").as_uint();
						ret->add_link(from_node, from_slot, to_node, to_slot);
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

	void BlueprintNodeLibraryPrivate::add_template(const std::string& name, const std::vector<BlueprintSlot>& inputs, const std::vector<BlueprintSlot>& outputs,
		BlueprintNodeFunction function, BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeInputSlotChangedCallback input_slot_changed_callback, BlueprintNodePreviewProvider preview_provider)
	{
		auto& t = node_templates.emplace_back();
		t.name = name;
		t.name_hash = sh(name.c_str());
		t.inputs = inputs;
		t.outputs = outputs;
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
		for (auto& n : g.nodes)
		{
			if (n.original->destructor)
				n.original->destructor(n.inputs.data(), n.outputs.data());
		}
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

		auto current_node_object_id = 0;
		if (auto pnode = current_node_ptr(); pnode)
			current_node_object_id = pnode->object_id;

		auto create_group_structure = [&](BlueprintGroupPtr src_g, Group& g, std::map<uint, Group::Data>& datas) {
			std::vector<BlueprintNodePtr> rest_nodes(src_g->nodes.size());
			for (auto i = 0; i < src_g->nodes.size(); i++)
				rest_nodes[i] = src_g->nodes[i].get();
			while (!rest_nodes.empty())
			{
				for (auto it = rest_nodes.begin(); it != rest_nodes.end();)
				{
					auto ok = true;
					for (auto& l : src_g->links)
					{
						if (l->to_node == *it)
						{
							if (std::find(rest_nodes.begin(), rest_nodes.end(), l->from_node) != rest_nodes.end())
							{
								ok = false;
								break;
							}
						}
					}
					if (ok)
					{
						auto& n = g.nodes.emplace_back();
						n.original = *it;
						n.object_id = n.original->object_id;
						it = rest_nodes.erase(it);
					}
					else
						it++;
				}
			}
			for (auto& n : g.nodes)
			{
				for (auto& i : n.original->inputs)
				{
					auto linked = false;
					for (auto& l : src_g->links)
					{
						if (l->to_node == n.original && l->to_slot == &i)
						{
							if (auto it = datas.find(l->from_slot->object_id); it != datas.end())
							{
								if (i.type->tag == TagPU && it->second.arg.type->tag == TagU)
								{
									Group::Data data;
									data.changed_frame = it->second.changed_frame;
									data.arg.type = i.type;
									data.arg.data = i.type->create();
									auto ptr = it->second.arg.data;
									memcpy((voidptr*)data.arg.data, &ptr, sizeof(voidptr));
									datas.emplace(i.object_id, data);

									n.inputs.push_back(data.arg);
								}
								else
								{
									BlueprintArgument arg;
									arg.type = it->second.arg.type;
									arg.data = it->second.arg.data;
									n.inputs.push_back(arg);
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
						data.changed_frame = i.data_changed_frame;
						data.arg.type = i.type;
						if (data.arg.type)
						{
							data.arg.data = data.arg.type->create();
							if (i.data)
								data.arg.type->copy(data.arg.data, i.data);
						}
						else
							data.arg.data = nullptr;
						datas.emplace(i.object_id, data);

						n.inputs.push_back(data.arg);
					}
				}
				for (auto& o : n.original->outputs)
				{
					Group::Data data;
					data.changed_frame = o.data_changed_frame;
					data.arg.type = o.type;
					if (data.arg.type)
					{
						data.arg.data = data.arg.type->create();
						if (data.arg.type->tag == TagPU)
							memset(data.arg.data, 0, sizeof(voidptr));
					}
					else
						data.arg.data = nullptr;
					datas.emplace(o.object_id, data);

					n.outputs.push_back(data.arg);
				}

				if (n.original->constructor)
					n.original->constructor(n.inputs.data(), n.outputs.data());
			}
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
				g.second.nodes.clear();
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
						if (auto it = g.second.datas.find(i.object_id); it != g.second.datas.end())
						{
							if (i.data_changed_frame > it->second.changed_frame)
							{
								assert(i.type == it->second.arg.type);
								i.type->copy(it->second.arg.data, i.data);
								it->second.changed_frame = i.data_changed_frame;
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
			create_group_structure(src_g.get(), g, g.datas);
			g.structure_updated_frame = frame;
			g.data_updated_frame = frame;
		}

		if (executing_group)
		{
			if (auto it = groups.find(executing_group); it == groups.end())
			{
				executing_group = 0;
				current_group = nullptr;
				current_node = -1;
			}
			else
			{
				current_group = &it->second;
				current_node = current_group->nodes.empty() ? -1 : 0;
				if (current_node_object_id)
				{
					for (auto i = 0; i < current_group->nodes.size(); i++)
					{
						if (current_group->nodes[i].object_id == current_node_object_id)
						{
							current_node = i;
							break;
						}
					}
				}
			}
		}

		built_frame = frames;
	}

	void BlueprintInstancePrivate::prepare_executing(uint group_name)
	{
		if (built_frame < blueprint->dirty_frame)
			build();

		if (auto it = groups.find(group_name); it == groups.end())
		{
			printf("blueprint instance prepare_executing: group %d not found\n", group_name);
			return;
		}
		else
		{
			executing_group = group_name;
			current_group = &it->second;
			current_node = 0;
		}
	}

	void BlueprintInstancePrivate::run()
	{
		while (current_group && current_node >= 0)
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
		if (!current_group || current_node < 0 || current_group->nodes.empty())
			return;

		auto& n = current_group->nodes[current_node];
		if (debugger && debugger->has_break_node(n.original))
		{
			debugger->debugging = this;
			return;
		}
		if (n.original->function)
		{
			n.original->function(n.inputs.empty() ? nullptr : n.inputs.data(),
				n.outputs.empty() ? nullptr : n.outputs.data());
		}
		n.updated_frame = frames;

		current_node++;
		if (current_node >= current_group->nodes.size())
		{
			executing_group = 0;
			current_group = nullptr;
			current_node = -1;
		}
	}

	void BlueprintInstancePrivate::stop()
	{
		auto debugger = BlueprintDebugger::current();
		if (debugger && debugger->debugging)
			return;

		executing_group = 0;
		current_group = nullptr;
		current_node = -1;
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
