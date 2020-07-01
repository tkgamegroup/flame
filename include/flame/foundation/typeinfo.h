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

	struct TypeInfo;
	struct EnumInfo;
	struct VariableInfo;
	struct FunctionInfo;
	struct UdtInfo;
	struct TypeInfoDatabase;

#define FLAME_TYPE_HASH(tag, name_hash) hash_update(name_hash, tag)
#define FLAME_TYPE_HASH_n(tag, name) hash_update(FLAME_CHASH(name), tag)
#define FLAME_TYPE_HASH_dn(name) FLAME_TYPE_HASH_n(TypeData, name)
#define FLAME_TYPE_HASH_pn(name) FLAME_TYPE_HASH_n(TypePointer, name)

	struct TypeInfo
	{
		virtual TypeTag get_tag() const = 0;
		virtual const char* get_name() const = 0; // no space, 'unsigned ' will be replace to 'u'
		virtual uint get_name_hash() const = 0;
		virtual uint get_size() const = 0;

		virtual void construct(void* p) const = 0;
		virtual void destruct(void* p) const = 0;
		virtual void copy(const void* src, void* dst) const = 0;
		// in the callback: return a space that will be fill with the string, which size must bigger than size
		virtual void serialize(char* (*callback)(Capture& c, uint size), const Capture& capture, const void* src) const = 0;
		inline std::string serialize_s(const void* src) const
		{
			std::string str;
			serialize([](Capture& c, uint size) {
				auto& str = *c.thiz<std::string>();
				str.resize(size);
				return str.data();
			}, Capture().set_thiz(&str), src);
			return str;
		}
		virtual void unserialize(const char* src, void* dst) const = 0;

		// name hash, not type hash
		FLAME_FOUNDATION_EXPORTS static TypeInfo* get_basic_type(uint name_hash);
		// in the callback: return a space that will be fill with the typeinfos, which size must bigger than sizeof(void*) * size
		FLAME_FOUNDATION_EXPORTS static void get_basic_types(TypeInfo** (*callback)(Capture& c, uint size), const Capture& capture);
	};

	enum VariableFlags
	{
		VariableFlagInput = 1 << 0,
		VariableFlagOutput = 1 << 2,
		VariableFlagEnumMulti = 1 << 3
	};

	struct VariableInfo
	{
		virtual UdtInfo* get_udt() const = 0;
		virtual uint get_index() const = 0;
		virtual TypeInfo* get_type() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_name_hash() const = 0;
		virtual uint get_flags() const = 0;
		virtual uint get_offset() const = 0;
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
		virtual TypeInfoDatabase* get_database() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_items_count() const = 0;
		virtual EnumItem* get_item(uint idx) const = 0;
		virtual EnumItem* find_item(const char* name) const = 0;
		virtual EnumItem* find_item(int value) const = 0;
	};

	struct FunctionInfo
	{
		virtual TypeInfoDatabase* get_database() const = 0;
		virtual UdtInfo* get_udt() const = 0;
		virtual uint get_index() const = 0;
		virtual const char* get_name() const = 0;
		virtual const void* get_rva() const = 0;
		virtual TypeInfo* get_type() const = 0;
		virtual uint get_parameters_count() const = 0;
		virtual TypeInfo* get_parameter(uint idx) const = 0;
		virtual const char* get_code() const = 0;

		// first is return type, next followed by all parameters, parameters are end by 0
		virtual bool check(uint type_hash, ...) const = 0;
	};

	struct UdtInfo
	{
		virtual TypeInfoDatabase* get_database() const = 0;
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
				dst[v->get_name()] = v->get_type()->serialize_s(src);
			}
		}

		inline void unserialize(const nlohmann::json& src, const void* dst) const
		{
			auto count = get_variables_count();
			for (auto i = 0; i < count; i++)
			{
				auto v = get_variable(i);
				v->get_type()->unserialize(src[v->get_name()].get<std::string>().c_str(), (char*)dst + v->get_offset());
			}
		}
	};

	struct TypeInfoDatabase
	{
		virtual void release() = 0;

		virtual const void* get_library() const = 0;
		virtual const wchar_t* get_library_name() const = 0;

		FLAME_FOUNDATION_EXPORTS static TypeInfoDatabase* load(const wchar_t* library_filename);
	};

	FLAME_FOUNDATION_EXPORTS EnumInfo* find_enum(uint hash);
	FLAME_FOUNDATION_EXPORTS UdtInfo* find_udt(uint hash);
}
