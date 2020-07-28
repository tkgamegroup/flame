#pragma once

#include <flame/serialize.h>
#include <flame/foundation/foundation.h>

namespace flame
{
	enum TypeTag
	{
		TypeEnumSingle,
		TypeEnumMulti,
		TypeData,
		TypePointer,
		TypeArrayOfData,
		TypeArrayOfPointer,

		TypeTagCount
	};

	struct EnumInfo;
	struct UdtInfo;
	struct Library;

	struct TypeInfo
	{
		virtual TypeTag get_tag() const = 0;
		virtual const char* get_name() const = 0; // no space, 'unsigned ' will be replace to 'u'
		virtual uint get_size() const = 0;

		// p = null to allocate new memory
		virtual void* create(void* p = nullptr) const = 0;
		virtual void destroy(void* p, bool free_memory = true) const = 0;
		virtual void copy(void* dst, const void* src) const = 0;
		virtual void serialize(void* str, const void* src) const = 0;
		virtual void unserialize(void* dst, const char* src) const = 0;

		FLAME_FOUNDATION_EXPORTS static TypeInfo* get(TypeTag, const char* name);
		// in the callback: return a space that will be fill with the typeinfos, which size must bigger than sizeof(void*) * size
		FLAME_FOUNDATION_EXPORTS static void get_basic_types(TypeInfo** (*callback)(Capture& c, uint size), const Capture& capture);
	};

	struct ReflectMeta
	{
		virtual uint get_tokens_count() const = 0;
		virtual void get_token(void* str, uint idx) const = 0;
		virtual bool has_token(const char* str) const = 0;
	};

	struct VariableInfo
	{
		virtual UdtInfo* get_udt() const = 0;
		virtual uint get_index() const = 0;
		virtual TypeInfo* get_type() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_offset() const = 0;
		virtual ReflectMeta* get_meta() const = 0;
		virtual const void* get_default_value() const = 0;
	};

	struct EnumItem
	{
		virtual EnumInfo* get_enum() const = 0;
		virtual uint get_index() const = 0;
		virtual const char* get_name() const = 0;
		virtual int get_value() const = 0;
	};

	struct EnumInfo
	{
		virtual Library* get_library() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_items_count() const = 0;
		virtual EnumItem* get_item(uint idx) const = 0;
		virtual EnumItem* find_item(const char* name) const = 0;
		virtual EnumItem* find_item(int value) const = 0;
	};

	struct FunctionInfo
	{
		virtual Library* get_library() const = 0;
		virtual UdtInfo* get_udt() const = 0;
		virtual uint get_index() const = 0;
		virtual const char* get_name() const = 0;
		virtual void* get_address(void* obj = nullptr /* for virtual fucntion */) const = 0;
		virtual TypeInfo* get_type() const = 0;
		virtual uint get_parameters_count() const = 0;
		virtual TypeInfo* get_parameter(uint idx) const = 0;
		virtual const char* get_code() const = 0;

		// first is return type, next followed by all parameters, parameters are end by null
		virtual bool check(void* type, ...) const = 0;
		virtual void call(void* obj, void* ret, void** parameters) const = 0;
	};

	struct UdtInfo
	{
		virtual Library* get_library() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_size() const = 0;
		virtual const char* get_base_name() const = 0; // base class name

		virtual uint get_variables_count() const = 0;
		virtual VariableInfo* get_variable(uint idx) const = 0;
		virtual VariableInfo* find_variable(const char* name) const = 0;
		virtual uint get_functions_count() const = 0;
		virtual FunctionInfo* get_function(uint idx) const = 0;
		virtual FunctionInfo* find_function(const char* name) const = 0;

		inline void serialize(const void* src, nlohmann::json& dst) const
		{
			auto count = get_variables_count();
			for (auto i = 0; i < count; i++)
			{
				auto v = get_variable(i);
				std::string str;
				v->get_type()->serialize(&str, src);
				dst[v->get_name()] = str;
			}
		}

		inline void unserialize(const nlohmann::json& src, const void* dst) const
		{
			auto count = get_variables_count();
			for (auto i = 0; i < count; i++)
			{
				auto v = get_variable(i);
				v->get_type()->unserialize((char*)dst + v->get_offset(), src[v->get_name()].get<std::string>().c_str());
			}
		}
	};

	struct Library
	{
		virtual void release() = 0;

		virtual char* get_address() const = 0;
		virtual const wchar_t* get_filename() const = 0;
		virtual void* get_exported_function(const char* name) = 0;

		FLAME_FOUNDATION_EXPORTS static Library* load(const wchar_t* filename, bool require_typeinfo = true);
	};

	FLAME_FOUNDATION_EXPORTS EnumInfo* find_enum(const char* name);
	FLAME_FOUNDATION_EXPORTS UdtInfo* find_udt(const char* name);
}
