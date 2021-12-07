#pragma once

#ifdef FLAME_BLUEPRINT_MODULE
#define FLAME_BLUEPRINT_EXPORTS __declspec(dllexport)
template<class T, class U>
struct FlameBlurprintTypeSelector
{
	typedef U result;
};
#else
#define FLAME_BLUEPRINT_EXPORTS __declspec(dllimport)
template<class T, class U>
struct FlameBlurprintTypeSelector
{
	typedef T result;
};
#endif

#define FLAME_BLUEPRINT_TYPE(name) struct name; struct name##Private; \
	typedef FlameBlurprintTypeSelector<name*, name##Private*>::result name##Ptr;

#include "../foundation/typeinfo.h"

namespace flame
{
	FLAME_BLUEPRINT_TYPE(bpSlot);
	FLAME_BLUEPRINT_TYPE(bpNode);
	FLAME_BLUEPRINT_TYPE(bpScene);

	enum bpSlotIO
	{
		bpSlotIn,
		bpSlotOut
	};

	enum bpNodeType
	{
		bpNodeEnumSingle,
		bpNodeEnumMulti,
		bpNodeVariable,
		bpNodeArray,
		bpNodeGroup,
		bpNodeUdt
	};

	enum bpObjectRule
	{
		bpObjectEntity,
		bpObjectRefRead,
		bpObjectRefWrite
	};

	struct bpNode;
	struct bpScene;

	inline bool bp_can_link(const TypeInfo* in_type, const TypeInfo* out_out)
	{
		if (in_type == out_out)
			return true;
		if (in_type->get_tag() != TagPointer || in_type->get_tag() != TagD)
			return false;
		auto in_name = std::string(in_type->get_name());
		return in_name == out_out->get_name() || in_name == "void";
	}

	struct bpSlot
	{
		void* user_data;

		virtual bpNodePtr get_node() const = 0;
		virtual bpSlotIO get_io() const = 0;
		virtual uint get_index() const = 0;
		virtual TypeInfoPtr get_type() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_offset() const = 0;
		virtual const void* get_data() const const = 0;
		inline const void* get_data_p() const { return *(void**)get_data(); }
		virtual void set_data(const void* data) = 0;
		inline void set_data_i(int i) { set_data(&i); }
		inline void set_data_f(float f) { set_data(&f); }
		inline void set_data_p(const void* p) { set_data(&p); }
		virtual const void* get_default_value() const = 0;

		virtual uint get_links_count() const = 0;
		virtual bpSlotPtr get_link(uint idx) const = 0;
		virtual bool link_to(bpSlotPtr target) = 0;
	};

	struct bpNode
	{
		void* user_data;

		virtual bpScenePtr get_scene() const = 0;
		virtual bpNodePtr get_parent() const = 0;

		virtual Guid get_guid() const = 0;
		virtual void set_guid(const Guid& guid) = 0;
		virtual const char* get_id() const = 0;
		virtual bool set_id(const char* id) = 0;
		virtual vec2 get_pos() const = 0;
		virtual void set_pos(const vec2& pos) = 0;

		virtual bpNodeType get_type() const = 0;
		virtual const char* get_type_parameter() const = 0;
		virtual bpObjectRule get_object_rule() const = 0;
		virtual UdtInfoPtr get_udt() const = 0;

		virtual uint get_inputs_count() const = 0;
		virtual bpSlotPtr get_input(uint idx) const = 0;
		virtual bpSlotPtr find_input(const char* name) const = 0;
		virtual uint get_outputs_count() const = 0;
		virtual bpSlotPtr get_output(uint idx) const = 0;
		virtual bpSlotPtr find_output(const char* name) const = 0;

		virtual uint get_children_count() const = 0;
		virtual bpNodePtr get_child(uint idx) const = 0;
		virtual bpNodePtr add_child(const char* id, bpNodeType type, const char* type_parameter, bpObjectRule object_rule = bpObjectEntity) = 0;
		virtual void remove_child(bpNodePtr n) = 0;
		virtual bpNodePtr find_child(const char* name) const = 0;
		virtual bpNodePtr find_child(const Guid& guid) const = 0;

		virtual void update() = 0;
	};

	struct bpScene
	{
		virtual void release() = 0;

		virtual const wchar_t* get_filename() const = 0;
		virtual float get_time() const = 0;
		virtual bpNodePtr get_root() const = 0;

		virtual void update() = 0;
		virtual void save() = 0;

		FLAME_BLUEPRINT_EXPORTS static bpScene* create(const wchar_t* filename);
	};
}

