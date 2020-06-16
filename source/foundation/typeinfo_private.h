#pragma once

#include <flame/foundation/typeinfo.h>

namespace flame
{
	struct TypeInfoPrivate : TypeInfo
	{
		TypeTag tag;
		bool is_array;
		std::string base_name;
		uint base_hash;
		std::string name;  // tag[A]#base
		uint hash;

		TypeInfoPrivate(TypeTag tag, const std::string& base_name, bool is_array);
	};

	struct VariableInfoPrivate : VariableInfo
	{

	};

	struct EnumItemPrivate : EnumItem
	{

	};

	struct EnumInfoPrivate : EnumInfo
	{

	};

	struct FunctionInfoPrivate : FunctionInfo
	{

	};

	struct UdtInfoPrivate : UdtInfo
	{

	};

	struct TypeInfoDatabasePrivate : TypeInfoDatabase
	{

	};
}
