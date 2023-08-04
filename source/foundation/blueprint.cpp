#include "../xml.h"
#include "typeinfo.h"
#include "blueprint_private.h"

namespace flame
{
	struct NodeTemplate
	{
		std::string name;
		uint name_hash = 0;
		std::vector<BlueprintSlot> inputs;
		std::vector<BlueprintSlot> outputs;
		BlueprintNodeFunction function = nullptr;
		BlueprintNodeInputSlotChangedCallback input_changed_callback = nullptr;
	};

	std::vector<NodeTemplate> node_library;
	std::vector<std::unique_ptr<BlueprintT>> loaded_blueprints;

	void init_blueprint()
	{
		{
			auto& t = node_library.emplace_back();
			t.name = "Add";
			{
				auto& i = t.inputs.emplace_back();
				i.name = "A";
				i.allowed_types.push_back(TypeInfo::get<float>());
			}
			{
				auto& i = t.inputs.emplace_back();
				i.name = "B";
				i.allowed_types.push_back(TypeInfo::get<float>());
			}
			{
				auto& o = t.outputs.emplace_back();
				o.name = "Out";
				o.allowed_types.push_back(TypeInfo::get<float>());
			}
			t.function = [](const BlueprintArgument* inputs, BlueprintArgument* outputs) {
				*(float*)outputs[0].data = *(float*)inputs[0].data + *(float*)inputs[1].data;
			};

		}

		for (auto& t : node_library)
			t.name_hash = sh(t.name.c_str());
	}

	BlueprintPrivate::BlueprintPrivate()
	{
		add_group("main");

		dirty_frame = frames;
	}

	BlueprintNodePtr BlueprintPrivate::add_node(BlueprintGroupPtr group, const std::string& name,
		const std::vector<BlueprintSlot>& inputs, const std::vector<BlueprintSlot>& outputs, BlueprintNodeFunction function)
	{
		if (!group)
			group = groups[0].get();
		assert(group->blueprint == this);

		auto n = new BlueprintNodePrivate;
		n->group = group;
		n->name = name;
		n->name_hash = sh(name.c_str());
		if (auto it = std::find_if(node_library.begin(), node_library.end(), [n](const auto& t) {
				return t.name_hash == n->name_hash;
			}); it != node_library.end())
		{
			n->inputs = it->inputs;
			n->outputs = it->outputs;
			n->function = it->function;
			n->input_changed_callback = it->input_changed_callback;
		}
		else
		{
			n->inputs = inputs;
			n->outputs = outputs;
			n->function = function;
		}
		group->nodes.emplace_back(n);

		dirty_frame = frames;

		return n;
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
				group->links.erase(it);
			else
				it++;
		}

		dirty_frame = frames;
	}

	BlueprintLinkPtr BlueprintPrivate::add_link(BlueprintNodePtr from_node, uint from_slot, BlueprintNodePtr to_node, uint to_slot)
	{
		auto group = from_node->group;
		assert(group->blueprint == this && group == to_node->group);
		auto l = new BlueprintLinkPrivate;
		l->from_node = from_node;
		l->from_slot = from_node->find_output(from_slot);
		l->to_node = to_node;
		l->to_slot = to_node->find_input(to_slot);
		group->links.emplace_back(l);

		dirty_frame = frames;

		return l;
	}

	void BlueprintPrivate::remove_link(BlueprintLinkPtr link)
	{
		auto group = link->from_node->group;
		assert(group->blueprint == this && group == link->to_node->group);
		for (auto it = group->links.begin(); it != group->links.end(); it++)
		{
			if (it->get() == link)
			{
				group->links.erase(it);
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

	BlueprintInstancePrivate::Group::~Group()
	{
		for (auto& pair : datas)
			pair.second.type->destroy(pair.second.data);
	}

	BlueprintInstancePrivate::BlueprintInstancePrivate()
	{
		update();
	}

	void BlueprintInstancePrivate::update()
	{
		groups.clear();
		current_group = nullptr;
		current_stack = -1;

		for (auto& src : blueprint->groups)
		{
			Group g;
			g.nodes.emplace_back();
			std::vector<BlueprintNodePtr> rest_nodes(src->nodes.size());
			for (auto i = 0; i < src->nodes.size(); i++)
				rest_nodes[i] = src->nodes[i].get();
			while (!rest_nodes.empty())
			{
				for (auto it = rest_nodes.begin(); it != rest_nodes.end();)
				{
					auto ok = true;
					for (auto& l : src->links)
					{
						if (l->to_node == *it)
						{
							if (std::find(rest_nodes.begin(), rest_nodes.end(), l->from_node) == rest_nodes.end())
							{
								ok = false;
								break;
							}
						}
					}
					if (ok)
					{
						auto& n = g.nodes.back().emplace_back();
						n.original = *it;
						rest_nodes.erase(it);
					}
					else
						it++;
				}
			}
			for (auto& s : g.nodes)
			{
				for (auto& n : s)
				{
					for (auto& i : n.original->inputs)
					{
						for (auto& l : src->links)
						{
							if (l->to_node == n.original && l->to_slot == &i)
							{
								if (auto it = g.datas.find(l->from_slot); it == g.datas.end())
									n.inputs.push_back(it->second);
								else
									assert(0);
							}
							else
							{
								BlueprintArgument arg;
								arg.type = i.type;
								arg.data = i.type->create();
								g.datas.emplace(&i, arg);

								n.inputs.push_back(arg);
							}
						}
					}
					for (auto& o : n.original->outputs)
					{
						BlueprintArgument arg;
						arg.type = o.type;
						arg.data = o.type->create();
						g.datas.emplace(&o, arg);

						n.outputs.push_back(arg);
					}
				}
			}
			groups.emplace(src->name_hash, g);
		}

		updated_frame = frames;
	}

	void BlueprintInstancePrivate::set_group(uint group_name)
	{
		if (updated_frame < blueprint->dirty_frame)
			update();

		if (auto it = groups.find(group_name); it == groups.end())
		{
			printf("blueprint instance set_group: group %d not found\n", group_name);
			return;
		}
		else
		{
			current_group = &it->second;
			current_stack = 0;
		}
	}

	void BlueprintInstancePrivate::run()
	{
		while (current_stack > 0)
			step();
	}

	void BlueprintInstancePrivate::step()
	{
		if (updated_frame < blueprint->dirty_frame)
			update();

		if (!current_group)
			return;
		if (current_stack < 0)
			return;
		if (current_stack >= current_group->nodes.size())
		{
			current_stack = -1;
			return;
		}

		for (auto& n : current_group->nodes[current_stack])
		{
			if (n.original->function)
			{
				n.original->function(n.inputs.empty() ? nullptr : n.inputs.data(),
					n.outputs.empty() ? nullptr : n.outputs.data());
			}
		}
	}

	struct BlueprintInstanceCreate : BlueprintInstance::Create
	{
		BlueprintInstancePtr operator()(BlueprintPtr blueprint) override
		{
			auto ret = new BlueprintInstancePrivate;

			return ret;
		}
	}BlueprintInstance_create;
	BlueprintInstance::Create& BlueprintInstance::create = BlueprintInstance_create;
}
