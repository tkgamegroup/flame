#pragma once

#include <flame/foundation/blueprint.h>

namespace flame
{
	struct TypeInfoPrivate;
	struct VariableInfoPrivate;

	struct bpSlotPrivate;
	struct bpNodePrivate;
	struct bpScenePrivate;

	struct bpSlotPrivate : bpSlot
	{
		bpNodePrivate* _node;
		bpSlotIO _io;
		uint _index;
		TypeInfoPrivate* _type;
		std::string _name;
		uint _offset;
		uint _size;
		void* _data = nullptr;
		void* _default_value = nullptr;

		std::vector<bpSlotPrivate*> _links;

		Setter* _setter = nullptr;
		void* _listener = nullptr;

		bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, TypeInfoPrivate* type, const std::string& name, uint offset, uint size, const void* default_value);
		bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, VariableInfoPrivate* vi);
		~bpSlotPrivate();

		void _set_data(const void* data);
		bool _link_to(bpSlotPrivate* target);

		bpNode* get_node() const override { return (bpNode*)_node; }
		bpSlotIO get_io() const override { return _io; }
		uint get_index() const override { return _index; }
		TypeInfo* get_type() const override { return (TypeInfo*)_type; }
		const char* get_name() const override { return _name.c_str(); }
		uint get_offset() const override { return _offset; }
		uint get_size() const override { return _size; }
		const void* get_data() const override { return _data; }
		void set_data(const void* data) override { _set_data(data); }
		const void* get_default_value() const override { return _default_value; }

		uint get_links_count() const override { return _links.size(); }
		bpSlot* get_link(uint idx) const override { return _links[idx]; }
		bool link_to(bpSlot* target) override { return _link_to((bpSlotPrivate*)target); }
	};

	struct bpNodePrivate : bpNode
	{
		bpScenePrivate* scene;
		bpNodePrivate* parent;

		Guid guid;
		std::string id;
		Vec2f pos;

		bpNodeType node_type;
		std::string type;
		UdtInfo* udt;

		std::vector<std::unique_ptr<bpSlotPrivate>> inputs;
		std::vector<std::unique_ptr<bpSlotPrivate>> outputs;

		std::vector<std::unique_ptr<bpNodePrivate>> children;

		void* object;
		void* library;

		void* dtor_addr;
		void* update_addr;

		uint order;

		std::vector<bpNodePrivate*> update_list;
		bool need_rebuild_update_list;

		bpNodePrivate(bpScenePrivate* scene, bpNodePrivate* parent, const std::string& id, bpNodeType node_type, const std::string& type);
		~bpNodePrivate();

		bpScene* get_scene() const override { return (bpScene*)scene; }
		bpNode* get_parent() const override { return parent; }

		Guid get_guid() const override { return guid; }
		void set_guid(const Guid& _guid) override { guid = _guid; }
		const char* get_id() const override { return id.c_str(); }
		bool set_id(const char* id) override { return _set_id(id); }
		bool _set_id(const std::string& id);
		Vec2f get_pos() const override { return pos; }
		void set_pos(const Vec2f& _pos) override { pos = _pos; }

		bpNodeType get_node_type() const override { return node_type; }
		const char* get_type() const override { return type.c_str(); }
		UdtInfo* get_udt() const override { return udt; }

		uint get_inputs_count() const override { return inputs.size(); }
		bpSlot* get_input(uint idx) const override { return inputs[idx].get(); }
		bpSlot* find_input(const char* name) const override { return _find_input(name); }
		bpSlotPrivate* _find_input(const std::string& name) const;
		uint get_outputs_count() const override { return outputs.size(); }
		bpSlot* get_output(uint idx) const override { return outputs[idx].get(); }
		bpSlot* find_output(const char* name) const override { return _find_output(name); }
		bpSlotPrivate* _find_output(const std::string& name) const;

		uint get_children_count() const override { return children.size(); }
		bpNode* get_child(uint idx) const override { return children[idx].get(); }
		bpNode* add_child(const char* id, const char* type, bpNodeType node_type) override { return _add_child(id, std::string(type), node_type); }
		bpNodePrivate* _add_child(const std::string& id, const std::string& type, bpNodeType node_type);
		void remove_child(bpNode* n) override { remove_child((bpNodePrivate*)n); }
		void _remove_child(bpNodePrivate* n);
		bpNode* find_child(const char* name) const override { return _find_child(name); }
		bpNodePrivate* _find_child(const std::string& name) const;
		bpNode* find_child(const Guid& guid) const override { return _find_child(guid); }
		bpNodePrivate* _find_child(const Guid& guid) const;

		void update();
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
		bpNode* get_root() const override { return root.get(); }

		void update() override;
		void save() override;
	};
}
