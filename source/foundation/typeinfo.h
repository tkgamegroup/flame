#pragma once

#include "../serialize.h"
#include "foundation.h"

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

	enum TypeMeta
	{
		MetaBpInput,
		MetaBpOutput,
		MetaSecondaryAttribute
	};

	struct TypeInfo
	{
		virtual TypeTag get_tag() const = 0;
		virtual const char* get_name() const = 0; // no space, 'unsigned ' will be replace to 'u'
		virtual uint get_hash() const = 0;
		virtual uint get_size() const = 0;

		virtual BasicType get_basic() const = 0;
		virtual bool get_signed() const = 0;
		virtual uint get_vec_size() const = 0;
		virtual uint get_col_size() const = 0;
		virtual TypeInfoPtr get_pointed_type() const = 0;

		virtual void* create(bool create_pointing = true) const = 0;
		virtual void destroy(void* p, bool destroy_pointing = true) const = 0;
		virtual void copy(void* dst, const void* src) const = 0;
		virtual bool compare(void* dst, const void* src) const = 0;
		virtual void serialize(const void* src, char* dst) const = 0;
		inline std::string serialize(const void* src) const
		{
			char str[256];
			serialize(src, str);
			return str;
		}
		virtual void unserialize(void* dst, const char* src) const = 0;

		FLAME_FOUNDATION_EXPORTS static TypeInfo* get(TypeTag tag, const char* name, TypeInfoDataBase* = nullptr);
	};

	struct VariableInfo
	{
		virtual UdtInfoPtr get_udt() const = 0;
		virtual uint get_index() const = 0;
		virtual TypeInfoPtr get_type() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_offset() const = 0;
		virtual uint get_array_size() const = 0;
		virtual uint get_array_stride() const = 0;
		virtual void* get_default_value() const = 0;
		virtual bool get_meta(TypeMeta m, LightCommonValue* v) const = 0;
	};

	struct EnumItemInfo
	{
		virtual EnumInfoPtr get_enum() const = 0;
		virtual uint get_index() const = 0;
		virtual const char* get_name() const = 0;
		virtual int get_value() const = 0;
	};

	struct EnumInfo
	{
		virtual const char* get_name() const = 0;
		virtual uint get_items_count() const = 0;
		virtual EnumItemInfoPtr get_item(uint idx) const = 0;
		virtual EnumItemInfoPtr find_item(const char* name) const = 0;
		virtual EnumItemInfoPtr find_item(int value) const = 0;
		virtual EnumItemInfoPtr add_item(const char* name, int value, int idx = -1) = 0;
		virtual void remove_item(EnumItemInfoPtr item) = 0;
	};

	struct FunctionInfo
	{
		virtual UdtInfoPtr get_udt() const = 0;
		virtual void* get_library() const = 0;
		virtual uint get_index() const = 0;
		virtual const char* get_name() const = 0;
		virtual const char* get_full_name() = 0;
		virtual uint get_rva() const = 0;
		virtual int get_voff() const = 0;
		virtual bool get_is_static() const = 0;
		virtual TypeInfoPtr get_type() const = 0;
		virtual const char* get_code() const = 0;
		virtual bool get_meta(TypeMeta m, LightCommonValue* v) const = 0;

		virtual uint get_parameters_count() const = 0;
		virtual TypeInfoPtr get_parameter(uint idx) const = 0;
		virtual void add_parameter(TypeInfoPtr ti, int idx = -1) = 0;
		virtual void remove_parameter(uint idx) = 0;

		virtual bool check(TypeInfoPtr ret, uint parms_count = 0, TypeInfoPtr const* parms = nullptr) const = 0;

		virtual void* get_address(void* obj = nullptr /* for virtual fucntion */) const = 0;

		virtual void call(void* obj, void* ret, void** parameters) = 0;
	};

	struct UdtInfo
	{
		virtual void* get_library() const = 0;
		virtual const char* get_name() const = 0;
		virtual uint get_size() const = 0;
		virtual const char* get_base_name() const = 0; // base class name

		virtual uint get_variables_count() const = 0;
		virtual VariableInfoPtr get_variable(uint idx) const = 0;
		virtual VariableInfoPtr find_variable(const char* name) const = 0;
		virtual VariableInfoPtr add_variable(TypeInfoPtr ti, const char* name, uint offset, uint array_size, uint array_stride, 
			const char* default_value_str, const char* metas, int idx = -1) = 0;
		virtual void remove_variable(VariableInfoPtr vi) = 0;

		virtual uint get_functions_count() const = 0;
		virtual FunctionInfoPtr get_function(uint idx) const = 0;
		virtual FunctionInfoPtr find_function(const char* name) const = 0;
		virtual FunctionInfoPtr add_function(const char* name, uint rva, int voff, bool is_static, TypeInfoPtr ti, const char* metas, int idx = -1) = 0;
		virtual void remove_function(FunctionInfoPtr fi) = 0;
	};

	struct TypeInfoDataBase
	{
		virtual void release() = 0;

		FLAME_FOUNDATION_EXPORTS static TypeInfoDataBase* create();
	};

	FLAME_FOUNDATION_EXPORTS void get_types(TypeInfo** dst, uint* len, TypeInfoDataBase* db = nullptr);

	inline std::vector<TypeInfo*> get_types()
	{
		std::vector<TypeInfo*> ret;
		uint len;
		get_types(nullptr, &len);
		ret.resize(len);
		get_types(ret.data(), nullptr);
		return ret;
	}

	FLAME_FOUNDATION_EXPORTS EnumInfo* find_enum(const char* name, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS EnumInfo* add_enum(const char* name, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS void get_enums(EnumInfo** dst, uint* len, TypeInfoDataBase* db = nullptr);

	inline std::vector<EnumInfo*> get_enums()
	{
		std::vector<EnumInfo*> ret;
		uint len;
		get_enums(nullptr, &len);
		ret.resize(len);
		get_enums(ret.data(), nullptr);
		return ret;
	}

	FLAME_FOUNDATION_EXPORTS UdtInfo* find_udt(const char* name, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS UdtInfo* add_udt(const char* name, uint size, const char* base_name, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS void get_udts(UdtInfo** dst, uint* len, TypeInfoDataBase* db = nullptr);

	inline std::vector<UdtInfo*> get_udts()
	{
		std::vector<UdtInfo*> ret;
		uint len;
		get_udts(nullptr, &len);
		ret.resize(len);
		get_udts(ret.data(), nullptr);
		return ret;
	}

	FLAME_FOUNDATION_EXPORTS void load_typeinfo(const wchar_t* filename, TypeInfoDataBase* db = nullptr);
	FLAME_FOUNDATION_EXPORTS void save_typeinfo(const wchar_t* filename, TypeInfoDataBase* db = nullptr);
}
