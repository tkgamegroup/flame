#pragma once

#include <flame/foundation/foundation.h>
#include <flame/foundation/serialize.h>

// when a type followed by a 'N', means it is not a template type, and bp will not put "<>" on it

namespace flame
{
	void* calc_rva(void* p, void* module)
	{
		return (void*)((char*)p - module);
	}

	struct BP_EnumSN
	{
		AttributeV<int> in;

		AttributeV<int> out;

		void update()
		{
			if (in.frame > out.frame)
			{
				out.v = in.v;
				out.frame = in.frame;
			}
		}

		static void add_udt_info(const std::wstring& module_name, const std::string& template_parameters, void* module)
		{
			auto type_name = std::string(template_parameters.begin() + 1, template_parameters.end() - 1);

			auto u = add_udt(module_name, "EnumSN" + template_parameters, sizeof(BP_EnumSN));

			u->add_variable(TypeTagAttributeES, type_name, "in", "i", offsetof(BP_EnumSN, in), sizeof(AttributeV<int>));
			u->add_variable(TypeTagAttributeES, type_name, "out", "o", offsetof(BP_EnumSN, out), sizeof(AttributeV<int>));

			u->add_function("update", calc_rva(f2v(&BP_EnumSN::update), module), TypeTagVariable, "void", "");
		}
	};

	template<uint N, class T>
	struct BP_Vec
	{
		AttributeV<T> in[N];

		AttributeV<Vec<N, T>> out;

		void update()
		{
			for (auto i = 0; i < N; i++)
			{
				if (in[i].frame > out.frame)
				{
					out.v.v_[i] = in[i].v;
					out.frame = in[i].frame;
				}
			}
		}

		static void add_udt_info(const std::wstring& module_name, const std::string& template_parameters, void* module)
		{
			auto pos_plus = template_parameters.find('+');
			assert(pos_plus != std::string::npos);
			auto type_name = std::string(template_parameters.begin() + pos_plus + 1, template_parameters.end() - 1);

			auto u = add_udt(module_name, "Vec" + template_parameters, sizeof(BP_Vec));

			for (auto i = 0; i < N; i++)
				u->add_variable(TypeTagAttributeV, type_name, std::string(1, "xyzw"[i]), "i", sizeof(AttributeV<T>) * i, sizeof(AttributeV<T>));
			u->add_variable(TypeTagAttributeV, "Vec" + template_parameters, "v", "o", offsetof(BP_Vec, out), sizeof(BP_Vec::out));

			u->add_function("update", calc_rva(f2v(&BP_Vec::update), module), TypeTagVariable, "void", "");
		}
	};

	template<uint N, class T>
	struct BP_Array
	{
		AttributeV<T> in[N];

		AttributeV<std::vector<T>> out;

		void update()
		{
			auto last_out_frame = out.frame;
			out.v.resize(N);
			for (auto i = 0; i < N; i++)
			{
				if (in[i].frame > last_out_frame)
				{
					out.v[i] = in[i].v;
					out.frame = max(out.frame, in[i].frame);
				}
			}
		}

		static void add_udt_info(const std::wstring& module_name, const std::string& template_parameters, void* module)
		{
			auto pos_plus = template_parameters.find('+');
			assert(pos_plus != std::string::npos);
			auto type_name = std::string(template_parameters.begin() + pos_plus + 1, template_parameters.end() - 1);

			auto u = add_udt(module_name, "Array" + template_parameters, sizeof(BP_Array));

			auto tag = TypeTagAttributeV;
			auto in_type_name = type_name;
			if (type_name.back() == '*')
			{
				tag = TypeTagAttributeP;
				in_type_name.resize(in_type_name.size() - 1);
			}
			for (auto i = 0; i < N; i++)
				u->add_variable(tag, in_type_name, std::to_string(i + 1), "i", sizeof(AttributeV<T>) * i, sizeof(AttributeV<T>));
			u->add_variable(TypeTagAttributeV, "std::vector(" + type_name + ")", "v", "o", offsetof(BP_Array, out), sizeof(BP_Array::out));

			u->add_function("ctor", calc_rva(cf2v<BP_Array>(), module), TypeTagVariable, "void", "");
			u->add_function("dtor", calc_rva(df2v<BP_Array>(), module), TypeTagVariable, "void", "");
			u->add_function("update", calc_rva(f2v(&BP_Array::update), module), TypeTagVariable, "void", "");
		}
	};

	template<class T>
	struct BP_ArraySize
	{
		AttributeP<std::vector<T>> in;

		AttributeV<uint> out;

		void update()
		{
			if (in.frame > out.frame)
			{
				out.v = in.v.size();
				out.frame = in.frame;
			}
		}

		static void add_udt_info(const std::wstring& module_name, const std::string& template_parameters, void* module)
		{
			auto type_name = std::string(template_parameters.begin() + 1, template_parameters.end() - 1);

			auto u = add_udt(module_name, "Pointer" + template_parameters, sizeof(BP_Pointer));

			u->add_variable(TypeTagAttributeP, "std::vector(" + type_name + ")", "in", "i", offsetof(BP_ArraySize, in), sizeof(AttributeP<std::vector<T>>));
			u->add_variable(TypeTagAttributeV, "uint", "out", "o", offsetof(BP_ArraySize, out), sizeof(AttributeV<uint>));

			u->add_function("update", calc_rva(f2v(&BP_ArraySize::update), module), TypeTagVariable, "void", "");
		}
	};
}
