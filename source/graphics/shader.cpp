#include "device_private.h"
#include "shader_private.h"

#include <flame/foundation/foundation.h>
#include <flame/foundation/serialize.h>

#if defined(FLAME_VULKAN)
#include <spirv_glsl.hpp>
#endif

namespace flame
{
	namespace graphics
	{
		static std::wstring shader_path(L"../shader/");
		static std::wstring conf_path(shader_path + L"src/config.conf");

#if defined(FLAME_VULKAN)
		static void serialize_members(spirv_cross::CompilerGLSL &glsl, uint32_t tid, SerializableNode *dst)
		{
			auto t = glsl.get_type(tid);
			auto cnt = t.member_types.size();
			for (auto i = 0; i < cnt; i++)
			{
				auto name = glsl.get_member_name(t.parent_type, i);
				auto offset = glsl.type_struct_member_offset(t, i);
				auto size = glsl.get_declared_struct_member_size(t, i);
				auto mid = t.member_types[i];
				auto mt = glsl.get_type(mid);
				auto count = mt.array.size() > 0 ? mt.array[0] : 1;
				auto array_stride = glsl.get_decoration(mid, spv::DecorationArrayStride);

				auto n = dst->new_node(name);
				n->new_attr("offset", std::to_string(offset));
				n->new_attr("size", std::to_string(size));
				n->new_attr("count", std::to_string(count));
				n->new_attr("array_stride", std::to_string(array_stride));
				serialize_members(glsl, mid, n);
			}
		}

		static void produce_shader_resource_file(const std::wstring &spv_file_in, const std::wstring &res_file_out)
		{
			auto spv_file = get_file_content(spv_file_in);

			std::vector<uint> spv_vec(spv_file.second / sizeof(uint));
			memcpy(spv_vec.data(), spv_file.first.get(), spv_file.second);

			spirv_cross::CompilerGLSL glsl(std::move(spv_vec));

			spirv_cross::ShaderResources resources = glsl.get_shader_resources();

			auto file = SerializableNode::create("res");

			for (auto &r : resources.uniform_buffers)
			{
				auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(r.id, spv::DecorationBinding);

				auto type = glsl.get_type(r.type_id);
				auto size = glsl.get_declared_struct_size(type);

				auto n = file->new_node("uniform_buffer");
				n->new_attr("set", std::to_string(set));
				n->new_attr("binding", std::to_string(binding));
				n->new_attr("size", std::to_string(size));
				n->new_attr("name", r.name);

				auto mn = n->new_node("members");
				serialize_members(glsl, r.type_id, mn);

			}
			for (auto &r : resources.storage_buffers)
			{
				auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(r.id, spv::DecorationBinding);

				auto type = glsl.get_type(r.type_id);
				auto size = glsl.get_declared_struct_size(type);

				auto n = file->new_node("storage_buffer");
				n->new_attr("set", std::to_string(set));
				n->new_attr("binding", std::to_string(binding));
				n->new_attr("size", std::to_string(size));
				n->new_attr("name", r.name);

				auto mn = n->new_node("members");
				serialize_members(glsl, r.type_id, mn);
			}
			for (auto &r : resources.sampled_images)
			{
				auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(r.id, spv::DecorationBinding);

				auto type = glsl.get_type(r.type_id);
				int count = type.array.size() > 0 ? type.array[0] : 1;

				auto n = file->new_node("sampled_image");
				n->new_attr("set", std::to_string(set));
				n->new_attr("binding", std::to_string(binding));
				n->new_attr("count", std::to_string(count));
				n->new_attr("name", r.name);
			}
			for (auto &r : resources.storage_images)
			{
				auto set = glsl.get_decoration(r.id, spv::DecorationDescriptorSet);
				auto binding = glsl.get_decoration(r.id, spv::DecorationBinding);

				auto type = glsl.get_type(r.type_id);
				int count = type.array.size() > 0 ? type.array[0] : 1;

				auto n = file->new_node("storage_image");
				n->new_attr("set", std::to_string(set));
				n->new_attr("binding", std::to_string(binding));
				n->new_attr("count", std::to_string(count));
				n->new_attr("name", r.name);
			}
			for (auto &r : resources.push_constant_buffers)
			{
				auto offset = glsl.get_decoration(r.id, spv::DecorationOffset);

				auto type = glsl.get_type(r.type_id);
				auto size = glsl.get_declared_struct_size(type);

				auto n = file->new_node("push_constant");
				n->new_attr("offset", std::to_string(offset));
				n->new_attr("size", std::to_string(size));
				n->new_attr("name", r.name);

				auto mn = n->new_node("members");
				serialize_members(glsl, r.type_id, mn);
			}

			file->save_xml(res_file_out);
			SerializableNode::destroy(file);
		}
#endif

		ShaderPrivate::ShaderPrivate(Device *_d, const std::wstring &filename, const std::string &prefix)
		{
			filename_ = filename;
			prefix_ = prefix;
			auto ext = std::fs::path(filename).extension();
			if (ext == L".vert")
				type = ShaderVert;
			else if (ext == L".tesc")
				type = ShaderTesc;
			else if (ext == L".tese")
				type = ShaderTese;
			else if (ext == L".geom")
				type = ShaderGeom;
			else if (ext == L".frag")
				type = ShaderFrag;
			else if (ext == L".comp")
				type = ShaderComp;

			d = (DevicePrivate*)_d;

			std::fs::remove(L"temp.spv"); // glslc cannot write to an existed file (well we did delete it when we finish compiling, but there can be one somehow)

			auto glsl_path = std::fs::path(shader_path + L"src/" + filename);

			auto hash = H(prefix.c_str());
			std::wstring spv_filename(filename + L"." + std::to_wstring(hash) + L".spv");
			spv_filename = shader_path + L"bin/" + spv_filename;

			if (!std::fs::exists(spv_filename) || std::fs::last_write_time(spv_filename) <= std::fs::last_write_time(glsl_path))
			{
				auto vk_sdk_path = s2w(getenv("VK_SDK_PATH"));
				assert(vk_sdk_path != L"");

				std::string pfx;
				pfx += "#version 450 core\n";
				pfx += "#extension GL_ARB_shading_language_420pack : enable\n";  // Allows the setting of uniform buffer object and sampler binding points directly from GLSL
				if (type != ShaderComp)
					pfx += "#extension GL_ARB_separate_shader_objects : enable\n";
				pfx += "\n" + prefix;
				auto temp_filename = glsl_path.parent_path().wstring() + L"/temp." + glsl_path.filename().wstring();
				{
					std::ofstream ofile(temp_filename);
					auto file = get_file_content(glsl_path.wstring());
					ofile.write(pfx.c_str(), pfx.size());
					ofile.write(file.first.get(), file.second);
					ofile.close();
				}
				std::wstring command_line(L" " + temp_filename + L" -flimit-file " + conf_path + L" -o temp.spv");
				auto output = exec_and_get_output((vk_sdk_path + L"/Bin/glslc.exe"), command_line);
				std::fs::remove(temp_filename);
				if (!std::fs::exists("temp.spv"))
					printf("shader \"%s\" compile error:\n\n%s\n\n", glsl_path.string().c_str(), *output.p);
				else
				{
					auto spv_dir = std::fs::path(spv_filename).parent_path();
					if (!std::fs::exists(spv_dir))
						std::fs::create_directories(spv_dir);
					std::fs::copy_file("temp.spv", spv_filename, std::fs::copy_options::overwrite_existing);
					std::fs::remove("temp.spv");
				}
				delete_mail(output);
			}

			auto spv_file = get_file_content(spv_filename);
			if (!spv_file.first)
				assert(0);

#if defined(FLAME_VULKAN)
			VkShaderModuleCreateInfo shader_info;
			shader_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shader_info.flags = 0;
			shader_info.pNext = nullptr;
			shader_info.codeSize = spv_file.second;
			shader_info.pCode = (uint32_t*)spv_file.first.get();
			chk_res(vkCreateShaderModule(d->v, &shader_info, nullptr, &v));

			auto res_filename = spv_filename + L".xml";
			if (!std::fs::exists(res_filename) || std::fs::last_write_time(res_filename) <= std::fs::last_write_time(spv_filename))
				produce_shader_resource_file(spv_filename.c_str(), res_filename.c_str());

#elif defined(FLAME_D3D12)

#endif
		}

		ShaderPrivate::~ShaderPrivate()
		{
#if defined(FLAME_VULKAN)
			if (v)
				vkDestroyShaderModule(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Shader *Shader::create(Device *d, const std::wstring &filename, const std::string &prefix)
		{
			return new ShaderPrivate(d, filename, prefix);
		}

		void Shader::destroy(Shader *s)
		{
			delete (ShaderPrivate*)s;
		}

		struct Shader$
		{
			void* device$i;
			//StringW filename$i;
			//String prefix$i;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS bool update$(float delta_time)
			{
				if (out$o)
				{
					Shader::destroy((Shader*)out$o);
					out$o = nullptr;
				}

				if (delta_time >= 0.f)
					out$o = Shader::create((Device*)device$i, L"", "");

				return false;
			}

		}bp_shader_unused;
	}
}
