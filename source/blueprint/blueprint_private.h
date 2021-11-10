#pragma once

#include "blueprint.h"

namespace flame
{
	struct bpSlotPrivate : bpSlot
	{
		bpNodePrivate* node;
		bpSlotIO io;
		uint index;
		TypeInfo* type;
		std::string name;
		uint offset;
		void* data = nullptr;
		void* default_value = nullptr;

		std::vector<bpSlotPrivate*> links;

		void* listener = nullptr;

		bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, TypeInfo* type, std::string_view name, uint offset, const void* default_value);
		bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, VariableInfo* vi);
		~bpSlotPrivate();

		bpNodePtr get_node() const override { return node; }
		bpSlotIO get_io() const override { return io; }
		uint get_index() const override { return index; }
		TypeInfo* get_type() const override { return type; }
		const char* get_name() const override { return name.c_str(); }
		uint get_offset() const override { return offset; }
		const void* get_data() const override { return data; }
		void set_data(const void* data) override;
		const void* get_default_value() const override { return default_value; }

		uint get_links_count() const override { return links.size(); }
		bpSlotPtr get_link(uint idx) const override { return links[idx]; }
		bool link_to(bpSlotPtr target) override;
	};

	struct bpNodePrivate : bpNode
	{
		bpScenePrivate* scene;
		bpNodePrivate* parent;

		Guid guid;
		std::string id;
		vec2 pos = vec2(0.f);

		bpNodeType type;
		std::string type_parameter;
		bpObjectRule object_rule;
		UdtInfo* udt = nullptr;

		std::vector<std::unique_ptr<bpSlotPrivate>> inputs;
		std::vector<std::unique_ptr<bpSlotPrivate>> outputs;

		std::vector<std::unique_ptr<bpNodePrivate>> children;

		void* object = nullptr;
		void* library_address = nullptr;

		void* dtor_addr = nullptr;
		void* update_addr = nullptr;

		uint order = 0xffffffff;

		std::vector<bpNodePrivate*> update_list;
		bool need_rebuild_update_list = true;

		bpNodePrivate(bpScenePrivate* scene, bpNodePrivate* parent, std::string_view id, bpNodeType type, std::string_view type_parameter, bpObjectRule object_rule);
		~bpNodePrivate();

		bpScenePtr get_scene() const override { return scene; }
		bpNodePtr get_parent() const override { return parent; }

		Guid get_guid() const override { return guid; }
		void set_guid(const Guid& _guid) override { guid = _guid; }
		const char* get_id() const override { return id.c_str(); }
		bool set_id(std::string_view id);
		bool set_id(const char* id) override { return set_id(std::string(id)); }
		vec2 get_pos() const override { return pos; }
		void set_pos(const vec2& _pos) override { pos = _pos; }

		bpNodeType get_type() const override { return type; }
		const char* get_type_parameter() const override { return type_parameter.c_str(); }
		bpObjectRule get_object_rule() const override { return object_rule; }
		UdtInfo* get_udt() const override { return (UdtInfo*)udt; }

		uint get_inputs_count() const override { return inputs.size(); }
		bpSlotPtr get_input(uint idx) const override { return inputs[idx].get(); }
		bpSlotPrivate* find_input(std::string_view name) const;
		bpSlotPtr find_input(const char* name) const override { return find_input(std::string(name)); }
		uint get_outputs_count() const override { return outputs.size(); }
		bpSlotPtr get_output(uint idx) const override { return outputs[idx].get(); }
		bpSlotPrivate* find_output(std::string_view name) const;
		bpSlotPtr find_output(const char* name) const override { return find_output(std::string(name)); }

		uint get_children_count() const override { return children.size(); }
		bpNodePtr get_child(uint idx) const override { return children[idx].get(); }
		bpNodePrivate* add_child(std::string_view id, bpNodeType type, std::string_view type_parameter, bpObjectRule object_rule);
		bpNodePtr add_child(const char* id, bpNodeType type, const char* type_parameter, bpObjectRule object_rule) override { return add_child(std::string(id), type, std::string(type_parameter), object_rule); }
		void remove_child(bpNodePtr n) override;
		bpNodePrivate* find_child(std::string_view name) const;
		bpNodePtr find_child(const char* name) const override { return find_child(std::string(name)); }
		bpNodePtr find_child(const Guid& guid) const override;

		void update() override;
	};

	struct bpScenePrivate : bpScene
	{
		std::filesystem::path filename;
		float time;
		std::unique_ptr<bpNodePrivate> root;

		bpScenePrivate();

		void release() override { delete this; }

		const wchar_t* get_filename() const override { return filename.c_str(); }
		float get_time() const override { return time; }
		bpNodePtr get_root() const override { return root.get(); }

		void update() override;
		void save() override;
	};
}
