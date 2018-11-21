struct NewImageDialog : flame::ui::FileSelector
{
	int cx = 512;
	int cy = 512;

	NewImageDialog() :
		FileSelector("New Image", flame::ui::FileSelectorSave, "", flame::ui::WindowModal | flame::ui::WindowNoSavedSettings)
	{
		first_cx = 800;
		first_cy = 600;

		callback = [this](std::string s) {
			if (std::experimental::filesystem::exists(s))
				return false;

			auto i = flame::create_image(cx, cy, 4, 32);
			flame::save_image(i, s);
			delete i;
			
			return true;
		};
	}

	virtual void on_right_area_show() override
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
		ImGui::PushItemWidth(200);
		ImGui::DragInt("cx", &cx);
		ImGui::DragInt("cy", &cy);
		const char *typeNames[] = {
			"color R8G8B8A8"
		};
		static int type = 0;
		ImGui::Combo("type", &type, typeNames, TK_ARRAYSIZE(typeNames));
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
	}
};

struct NewSceneDialog : flame::ui::FileSelector
{
	NewSceneDialog() :
		FileSelector("New Scene", flame::ui::FileSelectorSave, "", flame::ui::WindowModal | flame::ui::WindowNoSavedSettings)
	{
		first_cx = 800;
		first_cy = 600;

		callback = [this](std::string s) {
			if (std::experimental::filesystem::exists(s))
				return false;

			flame::XMLDoc xml("scene");
			flame::save_xml(&xml, s);
			return true;
		};
	}
};

void show_menu()
{
	bool open_windows_popup = false;
	bool open_device_popup = false;
	bool open_preferences_popup = false;
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::BeginMenu("New"))
		{
			if (ImGui::MenuItem("Scene"))
				;
			if (ImGui::MenuItem("Image"))
				new NewImageDialog;

			ImGui::EndMenu();
		}

		if (scene_editor)
			scene_editor->on_file_menu();

		ImGui::EndMenu();
	}
	if (scene_editor)
		scene_editor->on_menu_bar();
	if (ImGui::BeginMenu("View"))
	{
		static bool fullscreen = false;
		static int cx;
		static int cy;
		if (ImGui::MenuItem("Fullscreen", "", &fullscreen))
		{
			if (fullscreen)
			{
				cx = flame::surface->cx;
				cy = flame::surface->cy;
				flame::surface->set_window_size(0, 0, flame::surface->style | flame::SurfaceStyleFullscreen);
			}
			else
				flame::surface->set_window_size(cx, cy, flame::surface->style & (~flame::SurfaceStyleFullscreen));
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Resource Explorer"))
		{
			if (!resourceExplorer)
				resourceExplorer = new ResourceExplorer;
			resourceExplorer->_need_focus = true;
		}
		if (ImGui::MenuItem("Hierarchy"))
		{
			if (!hierarchy_window)
				hierarchy_window = new HierarchyWindow;
			hierarchy_window->_need_focus = true;
		}
		if (ImGui::MenuItem("Inspector"))
		{
			if (!inspector_window)
				inspector_window = new InspectorWindow;
			inspector_window->_need_focus = true;
		}
		if (ImGui::MenuItem("Shader Editor"))
		{
			if (!shader_editor)
				shader_editor = new ShaderEditor;
			shader_editor->_need_focus = true;
		}
		if (ImGui::MenuItem("Log Dog"))
		{
			if (!log_dog)
				log_dog = new LogDog;
			log_dog->_need_focus = true;
		}
		if (ImGui::MenuItem("Blueprint Editor"))
		{
			if (!blueprint_editor)
				blueprint_editor = new BlueprintEditor;
			blueprint_editor->_need_focus = true;
		}

		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Window"))
	{
		if (ImGui::MenuItem("Close All Windows"))
			;
		ImGui::Separator();
		if (ImGui::MenuItem("Windows"))
			open_windows_popup = true;

		ImGui::EndMenu();
	}
	static bool ui_demo_opened = false;
	if (ImGui::BeginMenu("Help"))
	{
		if (ImGui::MenuItem("Preferences"))
			open_preferences_popup = true;
		if (ImGui::MenuItem("Device Properties"))
			open_device_popup = true;
		if (ImGui::MenuItem("UI Demo"))
			ui_demo_opened = true;

		ImGui::EndMenu();
	}
	if (ui_demo_opened)
		ImGui::ShowDemoWindow(&ui_demo_opened);
	ImGui::EndMainMenuBar();

	static bool windows_popup_opened;
	static flame::ui::Window *windows_popup_w;
	if (open_windows_popup)
	{
		ImGui::OpenPopup("Windows");
		ImGui::SetNextWindowSize(ImVec2(400, 300));
		windows_popup_opened = true;
		windows_popup_w = nullptr;
	}
	if (ImGui::BeginPopupModal("Windows", &windows_popup_opened))
	{
		bool need_exit_window_popup = false;
		ImGui::BeginChild("##list", ImVec2(300, 0), true);
		for (auto &w : flame::ui::get_windows())
		{
			if (ImGui::Selectable(w->title.c_str(), w.get() == windows_popup_w))
				windows_popup_w = w.get();
		}
		ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginGroup();
		if (ImGui::Button("Active"))
			;
		if (ImGui::Button("Close"))
			;
		ImGui::Separator();
		if (windows_popup_w)
		{
			if (windows_popup_w->layout)
			{
				if (ImGui::Button("Undock"))
				{
					flame::ui::undock(windows_popup_w);
					need_exit_window_popup = true;
				}
			}
			else
			{
				static flame::ui::Window* target;
				if (ImGui::Button("Dock To..."))
				{
					target = nullptr;
					ImGui::OpenPopup("Dock To...");
				}
				if (ImGui::BeginPopupModal("Dock To...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
					if (ImGui::BeginCombo("window", target ? target->title.c_str() : ""))
					{
						if (flame::ui::main_layout->is_empty(0))
						{
							if (ImGui::Selectable("Main Layout", target == nullptr))
								target = nullptr;
						}
						for (auto &w : flame::ui::get_windows())
						{
							if (w->layout)
							{
								if (ImGui::Selectable(w->title.c_str(), target == w.get()))
									target = w.get();
							}
						}
						ImGui::EndCombo();
					}
					ImGui::PopStyleVar();
					static flame::ui::DockDirection dir = flame::ui::DockCenter;
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
					if (ImGui::BeginCombo("dir", flame::ui::get_dock_dir_name(dir)))
					{
						if (ImGui::Selectable(flame::ui::get_dock_dir_name(flame::ui::DockCenter), dir == flame::ui::DockCenter))
							dir = flame::ui::DockCenter;
						if (ImGui::Selectable(flame::ui::get_dock_dir_name(flame::ui::DockLeft), dir == flame::ui::DockLeft))
							dir = flame::ui::DockLeft;
						if (ImGui::Selectable(flame::ui::get_dock_dir_name(flame::ui::DockRight), dir == flame::ui::DockRight))
							dir = flame::ui::DockRight;
						if (ImGui::Selectable(flame::ui::get_dock_dir_name(flame::ui::DockTop), dir == flame::ui::DockTop))
							dir = flame::ui::DockTop;
						if (ImGui::Selectable(flame::ui::get_dock_dir_name(flame::ui::DockBottom), dir == flame::ui::DockBottom))
							dir = flame::ui::DockBottom;
						ImGui::EndCombo();
					}
					ImGui::PopStyleVar();
					if (ImGui::Button("OK"))
					{
						if (target || flame::ui::main_layout->is_empty(0))
						{
							flame::ui::dock(windows_popup_w, target, dir);
							need_exit_window_popup = true;
						}
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel"))
						ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
				}
			}
		}
		ImGui::EndGroup();
		if (need_exit_window_popup)
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	static glm::vec4 bg_color;
	if (open_preferences_popup)
	{
		bg_color = flame::ui::get_bg_color();

		ImGui::OpenPopup("Preferences");
		ImGui::SetNextWindowSize(ImVec2(400, 300));
	}
	if (ImGui::BeginPopupModal("Preferences"))
	{
		ImGui::BeginChild("##content", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() - 1), true);
		ImGui::ColorEdit4("background color", &bg_color[0], ImGuiColorEditFlags_NoInputs);
		ImGui::EndChild();

		if (ImGui::Button("Ok"))
		{
			flame::ui::set_bg_color(bg_color);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

	static bool device_popup_opened;
	if (open_device_popup)
	{
		ImGui::OpenPopup("Device Properties");
		ImGui::SetNextWindowSize(ImVec2(400, 300));
		device_popup_opened = true;
	}
	if (ImGui::BeginPopupModal("Device Properties", &device_popup_opened))
	{
		static char filter[260];
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
		ImGui::InputText(ICON_FA_SEARCH, filter, 260);
		ImGui::PopStyleVar();

		auto fText = [](const char* fmt, ...) {
			va_list args;
			va_start(args, fmt);
			ImGui::TextVFilted(fmt, filter, args);
			va_end(args);
		};

		if (ImGui::TreeNode("Physical Device Properties"))
		{
			auto &p = flame::vk_physical_device_properties;
			fText("api version: %d", p.apiVersion);
			fText("driver version: %d", p.driverVersion);
			fText("vendor ID: %d", p.vendorID);
			fText("device ID: %d", p.deviceID);
			fText("device type: %s", flame::vk_device_type_names[p.deviceType]);
			fText("device name: %s", p.deviceName);

			if (ImGui::TreeNode("Limits"))
			{
				auto &l = p.limits;
				fText("max image dimension 1D: %d", l.maxImageDimension1D);
				fText("max image dimension 2D: %d", l.maxImageDimension2D);
				fText("max image dimension 3D: %d", l.maxImageDimension3D);
				fText("max image dimension cube: %d", l.maxImageDimensionCube);
				fText("max image array layers: %d", l.maxImageArrayLayers);
				fText("max texel buffer range: %d", l.maxTexelBufferElements);
				fText("max uniform buffer range: %d", l.maxUniformBufferRange);
				fText("max storage buffer range: %d", l.maxStorageBufferRange);
				fText("max push constants size: %d", l.maxPushConstantsSize);
				fText("max memory allocation count: %d", l.maxMemoryAllocationCount);
				fText("max sampler allocation count: %d", l.maxSamplerAllocationCount);
				fText("buffer image granularity: %lld", l.bufferImageGranularity);
				fText("sparse address space size: %lld", l.sparseAddressSpaceSize);
				fText("max bound descriptor sets: %d", l.maxBoundDescriptorSets);
				fText("max per stage descriptor samplers: %d", l.maxPerStageDescriptorSamplers);
				fText("max per stage descriptor uniform buffers: %d", l.maxPerStageDescriptorUniformBuffers);
				fText("max per stage descriptor storage buffers: %d", l.maxPerStageDescriptorStorageBuffers);
				fText("max per stage descriptor sampled images: %d", l.maxPerStageDescriptorSampledImages);
				fText("max per stage descriptor storage images: %d", l.maxPerStageDescriptorStorageImages);
				fText("max per stage descriptor input attachments: %d", l.maxPerStageDescriptorInputAttachments);
				fText("max per stage resources: %d", l.maxPerStageResources);
				fText("max descriptor set samplers: %d", l.maxDescriptorSetSamplers);
				fText("max descriptor set uniform buffers: %d", l.maxDescriptorSetUniformBuffers);
				fText("max descriptor set uniform buffers dynamic: %d", l.maxDescriptorSetUniformBuffersDynamic);
				fText("max descriptor set storage buffers: %d", l.maxDescriptorSetStorageBuffers);
				fText("max descriptor set storage buffers dynamic: %d", l.maxDescriptorSetStorageBuffersDynamic);
				fText("max descriptor set sampled images: %d", l.maxDescriptorSetSampledImages);
				fText("max descriptor set storage images: %d", l.maxDescriptorSetStorageImages);
				fText("max descriptor set input attachments: %d", l.maxDescriptorSetInputAttachments);
				fText("max vertex input attributes: %d", l.maxVertexInputAttributes);
				fText("max vertex input bindings: %d", l.maxVertexInputBindings);
				fText("max vertex input attribute offset: %d", l.maxVertexInputAttributeOffset);
				fText("max vertex input binding stride: %d", l.maxVertexInputBindingStride);
				fText("max vertex output components: %d", l.maxVertexOutputComponents);
				fText("max tesellation generation level: %d", l.maxTessellationGenerationLevel);
				fText("max tesellation patch size: %d", l.maxTessellationPatchSize);
				fText("max tesellation control per vertex input components: %d", l.maxTessellationControlPerVertexInputComponents);
				fText("max tesellation control per vertex output components: %d", l.maxTessellationControlPerVertexOutputComponents);
				fText("max tesellation control per patch output components: %d", l.maxTessellationControlPerPatchOutputComponents);
				fText("max tesellation control per total output components: %d", l.maxTessellationControlTotalOutputComponents);
				fText("max tesellation evaluation input components: %d", l.maxTessellationEvaluationInputComponents);
				fText("max tesellation evaluation output components: %d", l.maxTessellationEvaluationOutputComponents);
				fText("max geometry shader invocations: %d", l.maxGeometryShaderInvocations);
				fText("max geometry input components: %d", l.maxGeometryInputComponents);
				fText("max geometry output components: %d", l.maxGeometryOutputComponents);
				fText("max geometry output vertices: %d", l.maxGeometryOutputVertices);
				fText("max geometry total output components: %d", l.maxGeometryTotalOutputComponents);
				fText("max fragment input components: %d", l.maxFragmentInputComponents);
				fText("max fragment output attachments: %d", l.maxFragmentOutputAttachments);
				fText("max fragment dual src attachments: %d", l.maxFragmentDualSrcAttachments);
				fText("max fragment combine output resources: %d", l.maxFragmentCombinedOutputResources);
				fText("max compute shader memory size: %d", l.maxComputeSharedMemorySize);
				fText("max compute work group count: %d, %d, %d", l.maxComputeWorkGroupCount[0], l.maxComputeWorkGroupCount[1], l.maxComputeWorkGroupCount[2]);
				fText("max compute work group invocations: %d", l.maxComputeWorkGroupInvocations);
				fText("max compute work group size: %d, %d, %d", l.maxComputeWorkGroupSize[0], l.maxComputeWorkGroupSize[1], l.maxComputeWorkGroupSize[2]);
				fText("sub pixel precision bits: %d", l.subPixelPrecisionBits);
				fText("sub texel precision bits: %d", l.subTexelPrecisionBits);
				fText("mipmap precision bits: %d", l.mipmapPrecisionBits);
				fText("max draw indexed index value: %d", l.maxDrawIndexedIndexValue);
				fText("max draw indirect count: %d", l.maxDrawIndirectCount);
				fText("max sampler lod bias: %f", l.maxSamplerLodBias);
				fText("max sampler anisotropy: %f", l.maxSamplerAnisotropy);
				fText("max viewports: %d", l.maxViewports);
				fText("max viewport dimensions: %d, %d", l.maxViewportDimensions[0], l.maxViewportDimensions[1]);
				fText("viewport bounds range: %f, %f", l.viewportBoundsRange[0], l.viewportBoundsRange[1]);
				fText("viewport sub pixel bits: %d", l.viewportSubPixelBits);
				fText("min memory map alignment: %lld", l.minMemoryMapAlignment);
				fText("min texel buffer offset alignment: %lld", l.minTexelBufferOffsetAlignment);
				fText("min uniform buffer offset alignment: %lld", l.minUniformBufferOffsetAlignment);
				fText("min storage buffer offset alignment: %lld", l.minStorageBufferOffsetAlignment);
				fText("min texel offset: %d", l.minTexelOffset);
				fText("max texel offset: %d", l.maxTexelOffset);
				fText("min texel gather offset: %d", l.minTexelGatherOffset);
				fText("max texel gather offset: %d", l.maxTexelGatherOffset);
				fText("min interpolation offset: %f", l.minInterpolationOffset);
				fText("max interpolation offset: %f", l.maxInterpolationOffset);
				fText("sun pixel interpolation offset bits: %d", l.subPixelInterpolationOffsetBits);
				fText("max framebuffer width: %d", l.maxFramebufferWidth);
				fText("max framebuffer height: %d", l.maxFramebufferHeight);
				fText("max framebuffer layers: %d", l.maxFramebufferLayers);
				fText("framebuffer color sample counts: %d", l.framebufferColorSampleCounts);
				fText("framebuffer depth sample counts: %d", l.framebufferDepthSampleCounts);
				fText("framebuffer stencil sample counts: %d", l.framebufferStencilSampleCounts);
				fText("framebuffer no attachments sample counts: %d", l.framebufferNoAttachmentsSampleCounts);
				fText("max color attachments: %d", l.maxColorAttachments);
				fText("sampled image color sample counts: %d", l.sampledImageColorSampleCounts);
				fText("sampled image integer sample counts: %d", l.sampledImageIntegerSampleCounts);
				fText("sampled image depth sample counts: %d", l.sampledImageDepthSampleCounts);
				fText("sampled image stencil sample counts: %d", l.sampledImageStencilSampleCounts);
				fText("storage image sample counts: %d", l.storageImageSampleCounts);
				fText("max sample mask words: %d", l.maxSampleMaskWords);
				fText("timestamp compute and graphics: %s", l.timestampComputeAndGraphics ? "true" : "false");
				fText("timestamp period: %f", l.timestampPeriod);
				fText("max clip distances: %d", l.maxClipDistances);
				fText("max cull distances: %d", l.maxCullDistances);
				fText("max combined clip and cull distances: %d", l.maxCombinedClipAndCullDistances);
				fText("discrete queue priorities: %d", l.discreteQueuePriorities);
				fText("point size range: %f, %f", l.pointSizeRange[0], l.pointSizeRange[1]);
				fText("line width range: %f, %f", l.lineWidthRange[0], l.lineWidthRange[1]);
				fText("point size granularity: %f", l.pointSizeGranularity);
				fText("line width granularity: %f", l.lineWidthGranularity);
				fText("strict lines: %s", l.strictLines ? "true" : "false");
				fText("standard sample locations: %s", l.standardSampleLocations ? "true" : "false");
				fText("optimal buffer copy offset alignment: %lld", l.optimalBufferCopyOffsetAlignment);
				fText("optimal buffer copy row pitch alignment: %lld", l.optimalBufferCopyRowPitchAlignment);
				fText("non coherent atom size: %lld", l.nonCoherentAtomSize);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Sparse Properties"))
			{
				auto &s = p.sparseProperties;
				fText("residency standard 2D block shape: %s", s.residencyStandard2DBlockShape ? "true" : "false");
				fText("residency standard 2D multisample block shape: %s", s.residencyStandard2DMultisampleBlockShape ? "true" : "false");
				fText("residency standard 3D block shape: %s", s.residencyStandard3DBlockShape ? "true" : "false");
				fText("residency aligned mip size: %s", s.residencyAlignedMipSize ? "true" : "false");
				fText("residency non resident strict: %s", s.residencyNonResidentStrict ? "true" : "false");

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Physical Device Features"))
		{
			auto &f = flame::vk_physical_device_features;
			fText("robust buffer access: %s", f.robustBufferAccess ? "true" : "false");
			fText("full draw index uint32: %s", f.fullDrawIndexUint32 ? "true" : "false");
			fText("image cube array: %s", f.imageCubeArray ? "true" : "false");
			fText("independent blend: %s", f.independentBlend ? "true" : "false");
			fText("geometry shader: %s", f.geometryShader ? "true" : "false");
			fText("tessellation shader: %s", f.tessellationShader ? "true" : "false");
			fText("sample rate shading: %s", f.sampleRateShading ? "true" : "false");
			fText("dua src blend: %s", f.dualSrcBlend ? "true" : "false");
			fText("logic op: %s", f.logicOp ? "true" : "false");
			fText("multi draw indirect: %s", f.multiDrawIndirect ? "true" : "false");
			fText("draw indirect first instance: %s", f.drawIndirectFirstInstance ? "true" : "false");
			fText("depth clamp: %s", f.depthClamp ? "true" : "false");
			fText("depth bias clamp: %s", f.depthBiasClamp ? "true" : "false");
			fText("fill mode non solid: %s", f.fillModeNonSolid ? "true" : "false");
			fText("depth bounds: %s", f.depthBounds ? "true" : "false");
			fText("wide lines: %s", f.wideLines ? "true" : "false");
			fText("large points: %s", f.largePoints ? "true" : "false");
			fText("alpha to one: %s", f.alphaToOne ? "true" : "false");
			fText("multi viewport: %s", f.multiViewport ? "true" : "false");
			fText("sampler anisotropy: %s", f.samplerAnisotropy ? "true" : "false");
			fText("texture compression ETC2: %s", f.textureCompressionETC2 ? "true" : "false");
			fText("texture compression ASTC_LDR: %s", f.textureCompressionASTC_LDR ? "true" : "false");
			fText("texture compression BC: %s", f.textureCompressionBC ? "true" : "false");
			fText("occlusion query precise: %s", f.occlusionQueryPrecise ? "true" : "false");
			fText("pipeline statistics query: %s", f.pipelineStatisticsQuery ? "true" : "false");
			fText("vertex pipeline stores and atomics: %s", f.vertexPipelineStoresAndAtomics ? "true" : "false");
			fText("fragment stores and atomics: %s", f.fragmentStoresAndAtomics ? "true" : "false");
			fText("shader tessellation and geometry point size: %s", f.shaderTessellationAndGeometryPointSize ? "true" : "false");
			fText("shader image gather extended: %s", f.shaderImageGatherExtended ? "true" : "false");
			fText("shader storage image extended formats: %s", f.shaderStorageImageExtendedFormats ? "true" : "false");
			fText("shader storage image multisample: %s", f.shaderStorageImageMultisample ? "true" : "false");
			fText("shader storage image read without format: %s", f.shaderStorageImageReadWithoutFormat ? "true" : "false");
			fText("shader storage image write without format: %s", f.shaderStorageImageWriteWithoutFormat ? "true" : "false");
			fText("shader uniform buffer array dynamic indexing: %s", f.shaderUniformBufferArrayDynamicIndexing ? "true" : "false");
			fText("shader sample image array dynamic indexing: %s", f.shaderSampledImageArrayDynamicIndexing ? "true" : "false");
			fText("shader storage buffer array dynamic indexing: %s", f.shaderStorageBufferArrayDynamicIndexing ? "true" : "false");
			fText("shader storage image array dynamic indexing: %s", f.shaderStorageImageArrayDynamicIndexing ? "true" : "false");
			fText("shader clip distance: %s", f.shaderClipDistance ? "true" : "false");
			fText("shader cull distance: %s", f.shaderCullDistance ? "true" : "false");
			fText("shader float64: %s", f.shaderFloat64 ? "true" : "false");
			fText("shader int64: %s", f.shaderInt64 ? "true" : "false");
			fText("shader int16: %s", f.shaderInt16 ? "true" : "false");
			fText("shader resource residency: %s", f.shaderResourceResidency ? "true" : "false");
			fText("shader resource min lod: %s", f.shaderResourceMinLod ? "true" : "false");
			fText("sparse binding: %s", f.sparseBinding ? "true" : "false");
			fText("sparse residency buffer: %s", f.sparseResidencyBuffer ? "true" : "false");
			fText("sparse residency image2D: %s", f.sparseResidencyImage2D ? "true" : "false");
			fText("sparse residency image3D: %s", f.sparseResidencyImage3D ? "true" : "false");
			fText("sparse residency 2 samples: %s", f.sparseResidency2Samples ? "true" : "false");
			fText("sparse residency 4 samples: %s", f.sparseResidency4Samples ? "true" : "false");
			fText("sparse residency 8 samples: %s", f.sparseResidency8Samples ? "true" : "false");
			fText("sparse residency 16 samples: %s", f.sparseResidency16Samples ? "true" : "false");
			fText("sparse residency aliased: %s", f.sparseResidencyAliased ? "true" : "false");
			fText("variable multisample rate: %s", f.variableMultisampleRate ? "true" : "false");
			fText("inherited queries: %s", f.inheritedQueries ? "true" : "false");

			ImGui::TreePop();
		}

		ImGui::EndPopup();
	}
}

void show_toolbar()
{
	ImGui::BeginToolBar();
	if (scene_editor)
		scene_editor->on_toolbar();
	ImGui::EndToolBar();
}

static flame::ui::Window *test_window;

int main(int argc, char** argv)
{
	flame::init(1280, 720, 1, 1280, 720, flame::SurfaceStyleFrame | flame::SurfaceStyleResizable, "Flame Engine Editor");
	flame::setup_shader_file_watcher();

	{
		for (auto &f : std::filesystem::directory_iterator("ui/windows"))
		{
			auto p = f.path();
			auto ext = p.extension().string();
			if (ext == ".dll")
			{
				auto mod = LoadLibrary(p.string().c_str());
				auto create_func = (PF_FLAME_CREATE_UI_WINDOW)(GetProcAddress(mod, "flame_create_ui_window"));
				auto destroy_func = (PF_FLAME_DESTROY_UI_WINDOW)(GetProcAddress(mod, "flame_destroy_ui_window"));

				test_window = create_func();
			}
		}
	}

	{
		auto xml = flame::load_xml("ui", "ui.xml");
		if (xml)
		{
			for (auto &n : xml->children)
			{
				if (n->name == "resource_explorer")
					resourceExplorer = new ResourceExplorer;
				else if (n->name == "hierarchy_window")
					hierarchy_window = new HierarchyWindow;
				else if (n->name == "inspector_window")
					inspector_window = new InspectorWindow;
				else if (n->name == "scene_editor")
				{
					auto s = flame::create_scene(n->find_attribute("filename")->value);
					if (s)
					{
						s->name = "scene";
						if (!scene_editor)
							scene_editor = new SceneEditor(s);
					}
				}
			}
			flame::release_xml(xml);
		}
	}

	flame::add_destroy_listener([]() {
		flame::XMLDoc xml("ui");
		if (resourceExplorer)
			xml.children.emplace_back(new flame::XMLNode("resource_explorer"));
		if (hierarchy_window)
			xml.children.emplace_back(new flame::XMLNode("hierarchy_window"));
		if (inspector_window)
			xml.children.emplace_back(new flame::XMLNode("inspector_window"));
		if (scene_editor)
		{
			auto n = new flame::XMLNode("scene_editor");
			n->attributes.emplace_back(new flame::XMLAttribute("filename", scene_editor->scene->get_filename()));
			xml.children.emplace_back(n);

			flame::save_scene(scene_editor->scene);
		}
		if (SelectObject)
			xml.children.emplace_back(new flame::XMLNode("select"));
		flame::save_xml(&xml, "ui.xml");

		flame::ui::save_layout();
	});

	//flame::app->set_window_maximized(true);
	flame::run([]() {
		//flame::ui::draw_text("Hello 2018", 100, 400, 150); // test for sdf
		show_menu();
		show_toolbar();

		//test_window->show();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::Begin("Profile");
		ImGui::Text("FPS:%d", flame::FPS);
		ImGui::Text("total:%dk", int(flame::p_total_time / 1000));
		ImGui::Text("head:%dk %d%%", int(flame::p_head_time / 1000), 100 * flame::p_head_time / flame::p_total_time);
		ImGui::Text("ui begin:%dk %d%%", int(flame::p_ui_begin_time / 1000), 100 * flame::p_ui_begin_time / flame::p_total_time);
		ImGui::Text("ui end:%dk %d%%", int(flame::p_ui_end_time / 1000), 100 * flame::p_ui_end_time / flame::p_total_time);
		ImGui::Text("render:%dk %d%%", int(flame::p_render_time / 1000), 100 * flame::p_render_time / flame::p_total_time);
		ImGui::Text("tail:%dk %d%%", int(flame::p_tail_time / 1000), 100 * flame::p_tail_time / flame::p_total_time);
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::BeginStatusBar();
		ImGui::Text("FPS:%d", flame::FPS);
		ImGui::EndStatusBar();
	});

	return 0;
}
