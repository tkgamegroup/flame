#pragma once

#include <flame/blueprint/blueprint.h>

namespace flame
{
	struct TypeInfo;
	struct VariableInfo;
	struct UdtInfo;

	struct bpSlotPrivate;
	struct bpNodePrivate;
	struct bpScenePrivate;

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

		bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, TypeInfo* type, const std::string& name, uint offset, const void* default_value);
		bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, VariableInfo* vi);
		~bpSlotPrivate();

		bpNode* get_node() const override { return (bpNode*)node; }
		bpSlotIO get_io() const override { return io; }
		uint get_index() const override { return index; }
		TypeInfo* get_type() const override { return (TypeInfo*)type; }
		const char* get_name() const override { return name.c_str(); }
		uint get_offset() const override { return offset; }
		const void* get_data() const override { return data; }
		void set_data(const void* data) override;
		const void* get_default_value() const override { return default_value; }

		uint get_links_count() const override { return links.size(); }
		bpSlot* get_link(uint idx) const override { return links[idx]; }
		bool link_to(bpSlotPrivate* target);
		bool link_to(bpSlot* target) override { return link_to((bpSlotPrivate*)target); }
	};

	struct bpNodePrivate : bpNode
	{
		bpScenePrivate* scene;
		bpNodePrivate* parent;

		Guid guid;
		std::string id;
		Vec2f pos = Vec2f(0.f);

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

		bpNodePrivate(bpScenePrivate* scene, bpNodePrivate* parent, const std::string& id, bpNodeType type, const std::string& type_parameter, bpObjectRule object_rule);
		~bpNodePrivate();

		void _remove_child(bpNodePrivate* n);
		bpNodePrivate* _find_child(const std::string& name) const;
		bpNodePrivate* _find_child(const Guid& guid) const;

		void _update();

		bpScene* get_scene() const override { return (bpScene*)scene; }
		bpNode* get_parent() const override { return parent; }

		Guid get_guid() const override { return guid; }
		void set_guid(const Guid& _guid) override { guid = _guid; }
		const char* get_id() const override { return id.c_str(); }
		bool set_id__(const std::string& id);
		bool set_id(const char* id) override { return set_id__(id); }
		Vec2f get_pos() const override { return pos; }
		void set_pos(const Vec2f& _pos) override { pos = _pos; }

		bpNodeType get_type() const override { return type; }
		const char* get_type_parameter() const override { return type_parameter.c_str(); }
		bpObjectRule get_object_rule() const override { return object_rule; }
		UdtInfo* get_udt() const override { return (UdtInfo*)udt; }

		uint get_inputs_count() const override { return inputs.size(); }
		bpSlot* get_input(uint idx) const override { return inputs[idx].get(); }
		bpSlotPrivate* _find_input(const std::string& name) const;
		bpSlot* find_input(const char* name) const override { return _find_input(name); }
		uint get_outputs_count() const override { return outputs.size(); }
		bpSlot* get_output(uint idx) const override { return outputs[idx].get(); }
		bpSlotPrivate* _find_output(const std::string& name) const;
		bpSlot* find_output(const char* name) const override { return _find_output(name); }

		uint get_children_count() const override { return children.size(); }
		bpNode* get_child(uint idx) const override { return children[idx].get(); }
		bpNodePrivate* add_child__(const std::string& id, bpNodeType type, const std::string& type_parameter, bpObjectRule object_rule);
		bpNode* add_child(const char* id, bpNodeType type, const char* type_parameter, bpObjectRule object_rule) override { return add_child(id, type, type_parameter, object_rule); }
		void remove_child(bpNode* n) override { remove_child((bpNodePrivate*)n); }
		bpNode* find_child(const char* name) const override { return _find_child(name); }
		bpNode* find_child(const Guid& guid) const override { return _find_child(guid); }

		void update() override { _update(); }
	};

	struct bpScenePrivate : bpScene
	{
		std::filesystem::path _filename;
		float _time;
		std::unique_ptr<bpNodePrivate> _root;

		bpScenePrivate();

		void _update();
		void _save();

		void release() override { delete this; }

		const wchar_t* get_filename() const override { return _filename.c_str(); }
		float get_time() const override { return _time; }
		bpNode* get_root() const override { return _root.get(); }

		void update() override { _update(); }
		void save() override { _save(); }
	};
}
