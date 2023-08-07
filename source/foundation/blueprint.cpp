#include "../xml.h"
#include "typeinfo.h"
#include "blueprint_private.h"

namespace flame
{
	std::vector<std::unique_ptr<BlueprintT>>			loaded_blueprints;
	std::vector<std::unique_ptr<BlueprintNodeLibraryT>> loaded_blueprint_node_libraries;
	static BlueprintNodeLibraryPtr standard_library = nullptr;

	BlueprintPrivate::BlueprintPrivate()
	{
		add_group("main");

		dirty_frame = frames;
	}

	BlueprintNodePtr BlueprintPrivate::add_node(BlueprintGroupPtr group /*null means the main group*/, const std::string& name,
		const std::vector<BlueprintSlot>& inputs, const std::vector<BlueprintSlot>& outputs,
		BlueprintNodeFunction function, BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeInputSlotChangedCallback input_slot_changed_callback, BlueprintNodePreviewer previewer)
	{
		if (!group)
			group = groups[0].get();
		assert(group->blueprint == this);

		auto ret = new BlueprintNodePrivate;
		ret->group = group;
		ret->name = name;
		ret->name_hash = sh(name.c_str());
		ret->inputs = inputs;
		ret->outputs = outputs;
		ret->constructor = constructor;
		ret->destructor = destructor;
		ret->function = function;
		ret->input_slot_changed_callback = input_slot_changed_callback;
		ret->previewer = previewer;
		for (auto& i : ret->inputs)
		{
			i.node = ret;
			i.name_hash = sh(i.name.c_str());
			i.type_idx = i.allowed_types.empty() ? -1 : 0;
			if (auto type = i.get_type(); type)
				i.data = type->create();
		}
		for (auto& o : ret->outputs)
		{
			o.node = ret;
			o.name_hash = sh(o.name.c_str());
			o.type_idx = o.allowed_types.empty() ? -1 : 0;
			if (auto type = o.get_type(); type)
				o.data = type->create();
		}
		group->nodes.emplace_back(ret);

		dirty_frame = frames;

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

		dirty_frame = frames;
	}

	static void change_slot_type(BlueprintSlotPtr slot, int type_idx)
	{
		if (slot->type_idx == type_idx)
			return;
		if (auto old_type = slot->get_type(); old_type)
		{
			old_type->destroy(slot->data);
			slot->data = nullptr;
		}
		slot->type_idx = type_idx;
		if (auto new_type = slot->get_type(); new_type)
			slot->data = new_type->create();
	}

	static void clear_invalid_links(BlueprintGroupPtr group)
	{
		auto done = false;
		while (!done)
		{
			done = true;
			for (auto& l : group->links)
			{
				if (l->from_slot->type_idx == -1 || l->to_slot->type_idx == -1)
				{
					if (l->to_slot->type_idx == -1 && !l->to_slot->allowed_types.empty())
						change_slot_type(l->to_slot, 0);
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
				input_types[i] = n->inputs[i].get_type();
			for (auto i = 0; i < output_types.size(); i++)
				output_types[i] = n->outputs[i].get_type();

			if (n->input_slot_changed_callback)
			{
				n->input_slot_changed_callback(input_types.data(), output_types.data());

				for (auto i = 0; i < output_types.size(); i++)
				{
					auto& slot = n->outputs[i];
					if (slot.get_type() != output_types[i])
					{
						change_slot_type(&slot, slot.find_type(output_types[i]));

						for (auto& l : n->group->links)
						{
							if (l->from_slot == &slot)
							{
								change_slot_type(l->from_slot, l->from_slot->find_type(slot.get_type()));

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

		auto type_idx = to_slot_ptr->find_type(from_slot_ptr->get_type());
		if (type_idx == -1)
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
		ret->from_node = from_node;
		ret->from_slot = from_node->find_output(from_slot);
		ret->to_node = to_node;
		ret->to_slot = to_node->find_input(to_slot);
		group->links.emplace_back(ret);

		change_slot_type(to_slot_ptr, type_idx);
		update_node_output_types(to_node);

		dirty_frame = frames;

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
				change_slot_type(to_slot, to_slot->allowed_types.empty() ? -1 : 0);
				update_node_output_types(to_node);
				break;
			}
		}

		dirty_frame = frames;
	}

	BlueprintGroupPtr BlueprintPrivate::add_group(const std::string& name)
	{
		auto g = new BlueprintGroupPrivate;
		g->blueprint = this;
		g->name = name;
		g->name_hash = sh(name.c_str());
		groups.emplace_back(g);

		dirty_frame = frames;

		return g;
	}

	void BlueprintPrivate::remove_group(BlueprintGroupPtr group) 
	{
		assert(group && group->blueprint == this);
		if (group->name_hash == "main"_h)
		{
			printf("blueprint remove_group: cannot remove the 'main' group\n");
			return;
		}
	}

	void BlueprintPrivate::move_to_group(const std::vector<BlueprintNodePtr>& nodes)
	{

	}

	void BlueprintPrivate::save()
	{
		pugi::xml_document doc;

		auto doc_root = doc.append_child("blueprint");
		auto n_groups = doc_root.append_child("groups");
		for (auto& g : groups)
		{
			auto n_group = n_groups.append_child("group");

		}

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

			if (!std::filesystem::exists(filename))
			{
				auto ret = new BlueprintPrivate;
				ret->save();
				return ret;
			}

			auto ret = new BlueprintPrivate;

			pugi::xml_document doc;
			pugi::xml_node doc_root;

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

		}
	}Blueprint_release;
	Blueprint::Release& Blueprint::release = Blueprint_release;

	void BlueprintNodeLibraryPrivate::add_template(const std::string& name, const std::vector<BlueprintSlot>& inputs, const std::vector<BlueprintSlot>& outputs,
		BlueprintNodeFunction function, BlueprintNodeConstructor constructor, BlueprintNodeDestructor destructor,
		BlueprintNodeInputSlotChangedCallback input_slot_changed_callback, BlueprintNodePreviewer previewer)
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
		t.previewer = previewer;
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

	BlueprintInstancePrivate::Group::~Group()
	{
		for (auto& pair : datas)
		{
			if (pair.second.second)
				pair.second.first->destroy(pair.second.second);
		}
	}

	BlueprintInstancePrivate::BlueprintInstancePrivate(BlueprintPtr _blueprint)
	{
		blueprint = _blueprint;
		build();
	}

	void BlueprintInstancePrivate::build()
	{
		groups.clear();
		executing_group = 0;
		current_group = nullptr;
		current_node = -1;

		for (auto& src_g : blueprint->groups)
		{
			auto& g = groups.emplace(src_g->name_hash, Group()).first->second;

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
							if (auto it = g.datas.find(l->from_slot); it != g.datas.end())
							{
								BlueprintArgument arg;
								arg.type_idx = i.find_type(it->second.first);
								arg.data = it->second.second;
								n.inputs.push_back(arg);
							}
							else
								assert(0);
							linked = true;
						}
					}
					if (!linked)
					{
						BlueprintArgument arg;
						arg.type_idx = i.type_idx;
						auto type = i.get_type();
						if (type)
						{
							arg.data = type->create();
							type->copy(arg.data, i.data);
						}
						else
							arg.data = nullptr;
						g.datas.emplace(&i, std::make_pair(type, arg.data));

						n.inputs.push_back(arg);
					}
				}
				for (auto& o : n.original->outputs)
				{
					BlueprintArgument arg;
					arg.type_idx = o.type_idx;
					auto type = o.get_type();
					if (type)
						arg.data = type->create();
					else
						arg.data = nullptr;
					g.datas.emplace(&o, std::make_pair(type, arg.data));

					n.outputs.push_back(arg);
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
