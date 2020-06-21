#pragma once

#include <flame/foundation/blueprint.h>

namespace flame
{
	struct TypeInfoPrivate;
	struct VariableInfoPrivate;
	struct UdtInfoPrivate;

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
		bpScenePrivate* _scene;
		bpNodePrivate* _parent;

		Guid _guid;
		std::string _id;
		Vec2f _pos = Vec2f(0.f);

		bpNodeType _node_type;
		std::string _type;
		UdtInfoPrivate* _udt = nullptr;

		std::vector<std::unique_ptr<bpSlotPrivate>> _inputs;
		std::vector<std::unique_ptr<bpSlotPrivate>> _outputs;

		std::vector<std::unique_ptr<bpNodePrivate>> _children;

		void* _object = nullptr;
		void* _library = nullptr;

		void* _dtor_addr = nullptr;
		void* _update_addr = nullptr;

		uint _order = 0xffffffff;

		std::vector<bpNodePrivate*> _update_list;
		bool _need_rebuild_update_list = true;

		bpNodePrivate(bpScenePrivate* scene, bpNodePrivate* parent, const std::string& id, bpNodeType node_type, const std::string& type);
		~bpNodePrivate();

		bool _set_id(const std::string& id);

		bpSlotPrivate* _find_input(const std::string& name) const;
		bpSlotPrivate* _find_output(const std::string& name) const;

		bpNodePrivate* _add_child(const std::string& id, const std::string& type, bpNodeType node_type);
		void _remove_child(bpNodePrivate* n);
		bpNodePrivate* _find_child(const std::string& name) const;
		bpNodePrivate* _find_child(const Guid& guid) const;

		void _update();

		bpScene* get_scene() const override { return (bpScene*)_scene; }
		bpNode* get_parent() const override { return _parent; }

		Guid get_guid() const override { return _guid; }
		void set_guid(const Guid& guid) override { _guid = guid; }
		const char* get_id() const override { return _id.c_str(); }
		bool set_id(const char* id) override { return _set_id(id); }
		Vec2f get_pos() const override { return _pos; }
		void set_pos(const Vec2f& pos) override { _pos = pos; }

		bpNodeType get_node_type() const override { return _node_type; }
		const char* get_type() const override { return _type.c_str(); }
		UdtInfo* get_udt() const override { return (UdtInfo*)_udt; }

		uint get_inputs_count() const override { return _inputs.size(); }
		bpSlot* get_input(uint idx) const override { return _inputs[idx].get(); }
		bpSlot* find_input(const char* name) const override { return _find_input(name); }
		uint get_outputs_count() const override { return _outputs.size(); }
		bpSlot* get_output(uint idx) const override { return _outputs[idx].get(); }
		bpSlot* find_output(const char* name) const override { return _find_output(name); }

		uint get_children_count() const override { return _children.size(); }
		bpNode* get_child(uint idx) const override { return _children[idx].get(); }
		bpNode* add_child(const char* id, const char* type, bpNodeType node_type) override { return _add_child(id, std::string(type), node_type); }
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
