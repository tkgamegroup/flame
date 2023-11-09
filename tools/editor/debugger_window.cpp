#include "debugger_window.h"

#include <flame/graphics/image.h>
#include <flame/graphics/debug.h>

DebuggerWindow debugger_window;

DebuggerView::DebuggerView() :
	DebuggerView(debugger_window.views.empty() ? "Debugger" : "Debugger##" + str(rand()))
{
}

DebuggerView::DebuggerView(const std::string& name) :
	View(&debugger_window, name)
{
}

void DebuggerView::on_draw()
{
	bool opened = true;
	ImGui::SetNextWindowSize(vec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &opened);
	imgui_window = ImGui::GetCurrentWindow();

	title_context_menu();

	if (ImGui::BeginTable("main", 2, ImGuiTableFlags_Resizable))
	{
		ImGui::TableSetupColumn("left", ImGuiTableColumnFlags_WidthFixed, 200.f);
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);

		auto found_last_buffer = false;
		if (ImGui::TreeNode("Buffers"))
		{
			auto all_buffers = graphics::Debug::get_buffers();
			for (auto b : all_buffers)
			{
				auto display_name = std::format("{} {}", b->ui ? b->ui->name : "", str(b));
				if (ImGui::Selectable(display_name.c_str(), b == selected_buffer))
				{
					selected_buffer = b;
					selected_image = nullptr;
				}
				if (b == selected_buffer)
					found_last_buffer = true;
			}
			ImGui::TreePop();
		}
		if (!found_last_buffer)
			selected_buffer = nullptr;
		auto found_last_image = false;
		if (ImGui::TreeNode("Images"))
		{
			auto all_images = graphics::Debug::get_images();
			for (auto i : all_images)
			{
				auto display_name = std::format("{} {}", i->filename.string(), str(i));
				if (ImGui::Selectable(display_name.c_str(), i == selected_image))
				{
					selected_buffer = nullptr;
					selected_image = i;
				}
				if (i == selected_image)
					found_last_image = true;
			}
			ImGui::TreePop();
		}
		if (!found_last_image)
			selected_image = nullptr;
		if (ImGui::TreeNode("Shaders"))
		{
			ImGui::TreePop();
		}

		ImGui::TableSetColumnIndex(1);
		if (selected_buffer)
		{
			ImGui::Text("Size: %s", str(selected_buffer->size).c_str());
			ImGui::SameLine();
			ImGui::Text("Usage: %s", TypeInfo::serialize_t(selected_buffer->usage).c_str());

			if (selected_buffer->ui)
			{
				std::function<void(UdtInfo* ui, uchar* data)> show_udt;
				show_udt = [&](UdtInfo* ui, uchar* data) {
					if (ImGui::TreeNode(ui->name.c_str()))
					{
						for (auto& vi : ui->variables)
						{
							if (ImGui::TreeNode(vi.name.c_str()))
							{
								ImGui::Text("Offset: %d, Size: %d", vi.offset, vi.type->size);

								auto show_data = [](TypeInfo_Data* ti, uchar* data) {
									switch (ti->data_type)
									{
									case DataFloat:
										switch (ti->vec_size)
										{
										case 1:
											ImGui::TextUnformatted(str(*(float*)data).c_str());
											break;
										case 2:
											switch (ti->col_size)
											{
											case 1:
												ImGui::TextUnformatted(str(*(vec2*)data).c_str());
												break;
											case 2:
												break;
											}
											break;
										case 3:
											switch (ti->col_size)
											{
											case 1:
												ImGui::TextUnformatted(str(*(vec3*)data).c_str());
												break;
											case 3:
											{
												auto& mat = *(mat3*)data;
												for (auto j = 0; j < 3; j++)
													ImGui::TextUnformatted(std::format("[{}]: {}", j, str(mat[j])).c_str());
											}
											break;
											}
											break;
										case 4:
											switch (ti->col_size)
											{
											case 1:
												ImGui::TextUnformatted(str(*(vec4*)data).c_str());
												break;
											case 4:
											{
												auto& mat = *(mat4*)data;
												for (auto j = 0; j < 4; j++)
													ImGui::TextUnformatted(std::format("[{}]: {}", j, str(mat[j])).c_str());
											}
											break;
											}
											break;
										}
										break;
									case DataInt:

										break;
									}
								};

								auto tag = vi.type->tag;
								auto vdata = data ? data + vi.offset : nullptr;
								switch (tag)
								{
								case TagD:
									if (data)
										show_data((TypeInfo_Data*)vi.type, vdata);
									break;
								case TagU:
									break;
								case TagAD:
								{
									auto ti = (TypeInfo_ArrayOfData*)vi.type;
									ImGui::SameLine();
									ImGui::Text("Extent: %s", str(ti->extent).c_str());
									ImGui::SameLine();
									ImGui::Text("Stride: %d", ti->stride);
									for (auto i = 0; i < ti->extent; i++)
									{
										if (ImGui::TreeNode(str(i).c_str()))
										{
											auto idata = vdata ? vdata + i * ti->stride : nullptr;
											if (idata)
												show_data(ti->ti, idata);
											ImGui::TreePop();
										}
									}
								}
									break;
								case TagAU:
								{
									auto ti = (TypeInfo_ArrayOfUdt*)vi.type;
									ImGui::SameLine();
									ImGui::Text("Extent: %s", str(ti->extent).c_str());
									ImGui::SameLine();
									ImGui::Text("Stride: %d", ti->stride);
									for (auto i = 0; i < ti->extent; i++)
									{
										if (ImGui::TreeNode(str(i).c_str()))
										{
											show_udt(ti->retrive_ui(), vdata ? vdata + i * ti->stride : nullptr);
											ImGui::TreePop();
										}
									}
								}
									break;
								}
								ImGui::TreePop();
							}
						}
						ImGui::TreePop();
					}
				};
				show_udt(selected_buffer->ui, (uchar*)selected_buffer->mapped);
			}
		}
		if (selected_image)
			view_image(selected_image, &view_swizzle, &view_sampler, &view_level, &view_layer, &view_zoom, &view_range_min, &view_range_max);

		ImGui::EndTable();
	}

	ImGui::End();
	if (!opened)
		delete this;
}

DebuggerWindow::DebuggerWindow() :
	Window("Debugger")
{
}

View* DebuggerWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		return new DebuggerView;
	return nullptr;
}

View* DebuggerWindow::open_view(const std::string& name)
{
	return new DebuggerView(name);
}
