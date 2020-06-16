#pragma once

#include <flame/foundation/blueprint.h>

namespace flame
{
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
		uint size;
		void* data;
		void* default_value;

		std::vector<bpSlotPrivate*> links;

		Setter* setter;
		void* listener;

		bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, TypeInfo* type, const std::string& name, uint offset, uint size, const void* _default_value);
		bpSlotPrivate(bpNodePrivate* node, bpSlotIO io, uint index, VariableInfo* vi);
		~bpSlotPrivate();

		bpNode* get_node() const override;
		bpSlotIO get_io() const override;
		uint get_index() const override;
		TypeInfo* get_type() const override;
		const char* get_name() const override;
		uint get_offset() const override;
		uint get_size() const override;
		const void* get_data() const override;
		void set_data(const void* data) override;
		const void* get_default_value() const override;

		uint get_links_count() const override;
		bpSlot* get_link(uint idx) const override;
		bool link_to(bpSlot* target) override;
		bool _link_to(bpSlotPrivate* target);
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

		bpScene* get_scene() const override;
		bpNode* get_parent() const override;

		Guid get_guid() const override;
		void set_guid(const Guid& guid) override;
		const char* get_id() const override;
		bool set_id(const char* id) override;
		bool set_id(const std::string& id);
		Vec2f get_pos() const override;
		void set_pos(const Vec2f& pos) override;

		bpNodeType get_node_type() const override;
		const char* get_type() const override;
		UdtInfo* get_udt() const override;

		uint get_inputs_count() const override;
		bpSlot* get_input(uint idx) const override;
		bpSlot* find_input(const char* name) const override;
		bpSlotPrivate* _find_input(const std::string& name) const;
		uint get_outputs_count() const override;
		bpSlot* get_output(uint idx) const override;
		bpSlot* find_output(const char* name) const override;
		bpSlotPrivate* _find_output(const std::string& name) const;

		uint get_children_count() const override;
		bpNode* get_child(uint idx) const override;
		bpNode* add_child(const char* id, const char* type, bpNodeType node_type) override;
		bpNodePrivate* add_child(const std::string& id, const std::string& type, bpNodeType node_type);
		void remove_child(bpNode* n) override;
		void remove_child(bpNodePrivate* n);
		bpNode* find_child(const char* name) const override;
		bpNodePrivate* _find_child(const std::string& name) const;
		bpNode* find_child(const Guid& guid) const override;
		bpNodePrivate* _find_child(const Guid& guid) const;

		void update();
	};

	struct bpScenePrivate : bpScene
	{
		std::filesystem::path filename;
		float time;
		std::unique_ptr<bpNodePrivate> root;

		bpScenePrivate();

		void release() override;

		const wchar_t* get_filename() const override;
		float get_time() const override;
		bpNode* get_root() const override;

		void update() override;
		void save() override;
	};
}
