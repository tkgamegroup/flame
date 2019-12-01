#pragma once

#include <flame/foundation/foundation.h>
#include <flame/foundation/serialize.h>

namespace flame
{
	void* calc_rva(void* p, void* module)
	{
		return (void*)((char*)p - module);
	}

	template<class T>
	struct BP_Var
	{
		AttributeD<T> in;

		AttributeD<T> out;

		void update()
		{
			if (in.b.frame > out.b.frame)
			{
				out.v = in.v;
				out.b.frame = looper().frame;
			}
		}

		static void add_udt_info(TypeinfoDatabase* db, const std::string& template_parameters, void* module)
		{
			auto type_name = std::string(template_parameters.begin() + 1, template_parameters.end() - 1);

			auto u = db->add_udt(TypeInfo(TypeData, "Var" + template_parameters), sizeof(BP_Var));

			u->add_variable(TypeInfo(TypeData, type_name, true), "in", "i", offsetof(BP_Var, in), sizeof(AttributeD<T>));
			u->add_variable(TypeInfo(TypeData, type_name, true), "out", "o", offsetof(BP_Var, out), sizeof(AttributeD<T>));

			u->add_function("update", calc_rva(f2v(&BP_Var::update), module), TypeInfo(TypeData, "void"), "");
		}
	};

	template<class T>
	struct BP_Enum
	{
		AttributeD<int> in;

		AttributeD<int> out;

		void update()
		{
			if (in.b.frame > out.b.frame)
			{
				out.v = in.v;
				out.b.frame = looper().frame;
			}
		}

		static void add_udt_info(TypeinfoDatabase* db, const std::string& template_parameters, void* module)
		{
			auto type_name = std::string(template_parameters.begin() + 1, template_parameters.end() - 1);

			auto u = db->add_udt(TypeInfo(TypeData, "Enum" + template_parameters), sizeof(BP_Enum));

			u->add_variable(TypeInfo(TypeEnumSingle, type_name, true), "in", "i", offsetof(BP_Enum, in), sizeof(AttributeD<int>));
			u->add_variable(TypeInfo(TypeEnumSingle, type_name, true), "out", "o", offsetof(BP_Enum, out), sizeof(AttributeD<int>));

			u->add_function("update", calc_rva(f2v(&BP_Enum::update), module), TypeInfo(TypeData, "void"), "");
		}
	};

	template<uint N, class T>
	struct BP_Vec
	{
		AttributeD<T> in[N];

		AttributeD<Vec<N, T>> out;

		void update()
		{
			for (auto i = 0; i < N; i++)
			{
				if (in[i].b.frame > out.b.frame)
				{
					out.v.v_[i] = in[i].v;
					out.b.frame = looper().frame;
				}
			}
		}

		static void add_udt_info(TypeinfoDatabase* db, const std::string& template_parameters, void* module)
		{
			auto pos_plus = template_parameters.find('+');
			assert(pos_plus != std::string::npos);
			auto type_name = std::string(template_parameters.begin() + pos_plus + 1, template_parameters.end() - 1);

			auto u = db->add_udt(TypeInfo(TypeData, "Vec" + template_parameters), sizeof(BP_Vec));

			for (auto i = 0; i < N; i++)
				u->add_variable(TypeInfo(TypeData, type_name, true), std::string(1, "xyzw"[i]), "i", sizeof(AttributeD<T>) * i, sizeof(AttributeD<T>));
			u->add_variable(TypeInfo(TypeData, "Vec" + template_parameters, true), "v", "o", offsetof(BP_Vec, out), sizeof(BP_Vec::out));

			u->add_function("update", calc_rva(f2v(&BP_Vec::update), module), TypeInfo(TypeData, "void"), "");
		}
	};

	template<uint N, class T>
	struct BP_Array
	{
		AttributeD<T> in[N];

		AttributeD<std::vector<T>> out;

		void update()
		{
			auto last_out_frame = out.b.frame;
			out.v.resize(N);
			for (auto i = 0; i < N; i++)
			{
				if (in[i].b.frame > last_out_frame)
				{
					out.v[i] = in[i].v;
					out.b.frame = looper().frame;
				}
			}
		}

		static void add_udt_info(TypeinfoDatabase* db, const std::string& template_parameters, void* module)
		{
			auto pos_plus = template_parameters.find('+');
			assert(pos_plus != std::string::npos);
			auto type_name = std::string(template_parameters.begin() + pos_plus + 1, template_parameters.end() - 1);

			auto u = db->add_udt(TypeInfo(TypeData, "Array" + template_parameters), sizeof(BP_Array));

			auto tag = TypeData;
			auto in_type_name = type_name;
			if (type_name.back() == '*')
			{
				tag = TypePointer;
				in_type_name.resize(in_type_name.size() - 1);
			}
			for (auto i = 0; i < N; i++)
				u->add_variable(TypeInfo(tag, in_type_name, true), std::to_string(i + 1), "i", sizeof(AttributeD<T>) * i, sizeof(AttributeD<T>));
			u->add_variable(TypeInfo(TypeData, type_name, true, true), "v", "o", offsetof(BP_Array, out), sizeof(BP_Array::out));

			u->add_function("ctor", calc_rva(cf2v<BP_Array>(), module), TypeInfo(TypeData, "void"), "");
			u->add_function("dtor", calc_rva(df2v<BP_Array>(), module), TypeInfo(TypeData, "void"), "");
			u->add_function("update", calc_rva(f2v(&BP_Array::update), module), TypeInfo(TypeData, "void"), "");
		}
	};

	template<class T>
	struct BP_ArraySize
	{
		AttributeP<std::vector<T>> in;

		AttributeD<uint> out;

		void update()
		{
			if (in.b.frame > out.b.frame)
			{
				out.v = in.v.size();
				out.b.frame = looper().frame;
			}
		}

		static void add_udt_info(TypeinfoDatabase* db, const std::string& template_parameters, void* module)
		{
			auto type_name = std::string(template_parameters.begin() + 1, template_parameters.end() - 1);

			auto u = db->add_udt(TypeInfo(TypeData, "ArraySize" + template_parameters), sizeof(BP_Pointer));

			u->add_variable(TypeInfo(TypePointer, type_name, true, true), "in", "i", offsetof(BP_ArraySize, in), sizeof(AttributeP<std::vector<T>>));
			u->add_variable(TypeInfo(TypeData, "uint", true), "out", "o", offsetof(BP_ArraySize, out), sizeof(AttributeD<uint>));

			u->add_function("update", calc_rva(f2v(&BP_ArraySize::update), module), TypeInfo(TypeData, "void"), "");
		}
	};
}
