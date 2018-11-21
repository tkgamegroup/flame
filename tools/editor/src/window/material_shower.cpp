#include <flame/filesystem.h>
#include <flame/engine/graphics/texture.h>
#include <flame/engine/graphics/material.h>
#include <flame/engine/ui/ui.h>
#include "material_shower.h"

MaterialShower::MaterialShower(flame::Material *_m) :
	m(_m)
{
	{
		auto t = m->get_albedo_alpha_map();
		if (t)
			flame::ui::increase_texture_ref(t);
	}
	{
		auto t = m->get_spec_roughness_map();
		if (t)
			flame::ui::increase_texture_ref(t);
	}
	{
		auto t = m->get_normal_height_map();
		if (t)
			flame::ui::increase_texture_ref(t);
	}
}

MaterialShower::~MaterialShower()
{
	{
		auto t = m->get_albedo_alpha_map();
		if (t)
			flame::ui::decrease_texture_ref(t);
	}
	{
		auto t = m->get_spec_roughness_map();
		if (t)
			flame::ui::decrease_texture_ref(t);
	}
	{
		auto t = m->get_normal_height_map();
		if (t)
			flame::ui::decrease_texture_ref(t);
	}
}

static std::string show_map(flame::Texture *m, const char *tooltip)
{
	ImGui::Image_s(m, ImVec2(16, 16), ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(ImGuiCol_Border)));

	std::string str;

	if (ImGui::IsItemHovered())
	{
		std::string str = tooltip;
		if (m)
			str = m->filename + "\n" + str;
		ImGui::SetTooltip(str.c_str());
		if (ImGui::IsMouseClicked(1))
			str = "[remove]";
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
		{
			static char filename[260];
			strcpy(filename, (char*)payload->Data);
			std::filesystem::path path(filename);
			auto ext = path.extension();
			if (flame::is_image_file(ext.string()))
				str = filename;
		}
		ImGui::EndDragDropTarget();
	}

	return str;
}

void show_material(flame::Material *m)
{
	{
		auto filename = show_map(m->get_albedo_alpha_map(), "albedo(rgb) alpha(a) map, [right click] to remove");
		if (filename != "")
		{
			if (filename == "[remove]")
				m->set_albedo_alpha_map("");
			else
			{
				auto t = m->get_spec_roughness_map();
				if (t)
					flame::ui::decrease_texture_ref(t);
				m->set_albedo_alpha_map(filename);
				t = m->get_spec_roughness_map();
				if (t)
					flame::ui::increase_texture_ref(t);
			}
		}
	}
	ImGui::SameLine();
	auto albedo_alpha = m->get_albedo_alpha();
	if (ImGui::ColorEdit4("albedo alpha", &albedo_alpha[0], ImGuiColorEditFlags_NoInputs))
		m->set_albedo_alpha(albedo_alpha);
	{
		auto filename = show_map(m->get_spec_roughness_map(), "spec(r) roughness(g) map, [right click] to remove");
		if (filename != "")
		{
			if (filename == "[remove]")
				m->set_albedo_alpha_map("");
			else
			{
				auto t = m->get_spec_roughness_map();
				if (t)
					flame::ui::decrease_texture_ref(t);
				m->set_albedo_alpha_map(filename);
				t = m->get_spec_roughness_map();
				if (t)
					flame::ui::increase_texture_ref(t);
			}
		}
	}
	ImGui::SameLine();
	auto spec = m->get_spec();
	if (ImGui::DragFloat("spec", &spec, 0.01f, 0.f, 1.f))
		m->set_spec(spec);
	auto roughness = m->get_roughness();
	ImGui::Dummy(ImVec2(18, 0));
	ImGui::SameLine();
	if (ImGui::DragFloat("roughness", &roughness, 0.01f, 0.f, 1.f))
		m->set_roughness(roughness);
	{
		auto filename = show_map(m->get_normal_height_map(), "normal(rgb) height(a) map, [right click] to remove");
		if (filename != "")
		{
			if (filename == "[remove]")
				m->set_albedo_alpha_map("");
			else
			{
				auto t = m->get_spec_roughness_map();
				if (t)
					flame::ui::decrease_texture_ref(t);
				m->set_albedo_alpha_map(filename);
				t = m->get_spec_roughness_map();
				if (t)
					flame::ui::increase_texture_ref(t);
			}
		}
	}
	ImGui::SameLine();
	ImGui::TextUnformatted("normal height");
}

void MaterialShower::show()
{

}
