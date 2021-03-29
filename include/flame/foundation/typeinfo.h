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
		TypeFunction,

		TypeTagCount
	};

	enum BasicType
	{
		VoidType,
		BooleanType,
		IntegerType,
		FloatingType,
		CharType,
		WideCharType,
		ElseType
	};

	struct EnumInfo;
	struct UdtInfo;
	struct Library;
	struct TypeInfoDataBase;

	struct TypeInfo
	{
		virtual TypeTag get_tag() const = 0;
		virtual const char* get_name() const = 0; // no space, 'unsigned ' will be replace to 'u'
		virtual uint get_size() const = 0;

		virtual BasicType get_basic() const = 0;
		virtual bool get_signed() const = 0;
		virtual uint get_vec_size() const = 0;
		virtual uint get_col_size() const = 0;
		virtual TypeInfo* get_pointed_type() const = 0;

		virtual void* create(bool create_pointing = true) const = 0;
		virtual void destroy(void* p, bool destroy_pointing = true) const = 0;
		virtual void copy(void* dst, const void* src) const = 0;
		virtual bool compare(void* dst, const void* src) const = 0;
		virtual void serialize(const void* src, void* str, char* (*str_allocator)(void* str, uint size)) const = 0;
		inline std::string serialize(const void* src)
		{
			std::string str;
			serialize(src, &str, [](void* _str, uint size) {
				auto& str = *(std::string*)_str;
				str.resize(size);
				return str.data();
			});
			return str;
		}
		virtual void unserialize(void* dst, const char* src) const = 0;

		FLAME_FOUNDATION_EXPORTS static TypeInfo* get(TypeTag tag, const char* name, TypeInfoDataBase* db = nullptr);
	};

	inline TypeInfo* ti_es(const char* name)
	{
		return TypeInfo::get(TypeEnumSingle, name);
	}

	struct ReflectMeta
	{
		virtual uint get_tokens_count() const = 0;
		virtual void get_token(char** name_dst, char** pvalue, uint idx) const = 0;
		virtual bool get_token(const char* name, char** pvalue = nullptr) const = 0;
	};

	struct VariableInfo
	{
		virtual UdtInfo* get_udt() const = 0;
		virtual uint get_index() const = 0;
		virtual TypeInfo* get_type() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_offset() const = 0;
		virtual uint get_array_size() const = 0;
		virtual uint get_array_stride() const = 0;
		virtual const char* get_default_value() const = 0;
		virtual ReflectMeta* get_meta() const = 0;
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
		virtual EnumItem* add_item(const char* name, int value, int idx = -1) = 0;
		virtual void remove_item(EnumItem* item) = 0;
	};

	struct FunctionInfo
	{
		virtual Library* get_library() const = 0;
		virtual UdtInfo* get_udt() const = 0;
		virtual uint get_index() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_rva() const = 0;
		virtual uint get_voff() const = 0;
		virtual TypeInfo* get_type() const = 0;

		virtual uint get_parameters_count() const = 0;
		virtual TypeInfo* get_parameter(uint idx) const = 0;
		virtual void add_parameter(TypeInfo* ti, int idx = -1) = 0;
		virtual void remove_parameter(uint idx) = 0;

		virtual const char* get_code() const = 0;

		virtual void* get_address(void* obj = nullptr /* for virtual fucntion */) const = 0;

		virtual bool check(TypeInfo* ret, uint parms_count = 0, TypeInfo* const* parms = nullptr) const = 0;

		virtual void call(void* obj, void* ret, void* parameters) const = 0;
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
		virtual VariableInfo* add_variable(TypeInfo* ti, const char* name, uint offset, uint array_size, uint array_stride, const char* default_value, const char* meta, int idx = -1) = 0;
		virtual void remove_variable(VariableInfo* vi) = 0;

		virtual uint get_functions_count() const = 0;
		virtual FunctionInfo* get_function(uint idx) const = 0;
		virtual FunctionInfo* find_function(const char* name) const = 0;
		virtual FunctionInfo* add_function(const char* name, uint rva, uint voff, TypeInfo* ti, int idx = -1) = 0;
		virtual void remove_function(FunctionInfo* fi) = 0;

		inline void serialize(const void* src, nlohmann::json& dst) const
		{
			auto count = get_variables_count();
			for (auto i = 0; i < count; i++)
			{
				auto v = get_variable(i);
				std::string str;
				v->get_type()->serialize(src, &str, [](void* _str, uint size) {
					auto& str = *(std::string*)_str;
					str.resize(size);
					return str.data();
				});
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

	struct TypeInfoDataBase
	{
		virtual void release() = 0;

		FLAME_FOUNDATION_EXPORTS static TypeInfoDataBase* create();
	};

	FLAME_FOUNDATION_EXPORTS EnumInfo* find_enum(const char* name, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS EnumInfo* add_enum(const char* name, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS UdtInfo* find_udt(const char* name, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS UdtInfo* add_udt(const char* name, uint size, const char* base_name, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS void traverse_enums(void (*callback)(Capture& c, EnumInfo* ei), const Capture& capture, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS void traverse_udts(void (*callback)(Capture& c, UdtInfo* ui), const Capture& capture, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS void load_typeinfo(const wchar_t* filename, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS void save_typeinfo(const wchar_t* filename, TypeInfoDataBase* db = nullptr);

	struct Library
	{
		virtual void release() = 0;

		virtual char* get_address() const = 0;
		virtual const wchar_t* get_filename() const = 0;
		virtual void* get_exported_function(const char* name) = 0;

		FLAME_FOUNDATION_EXPORTS static Library* load(const wchar_t* filename, bool require_typeinfo = true);
	};
}
