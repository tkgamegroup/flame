#pragma once

#include <flame/foundation/typeinfo.h>
#include <flame/network/network.h>
#include <flame/graphics/device.h>
#include <flame/graphics/buffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/graphics/command.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/window.h>
#include <flame/sound/device.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>
#include <flame/physics/device.h>
#include <flame/physics/scene.h>
#include <flame/script/script.h>
#include <flame/universe/component.h>
#include <flame/universe/entity.h>
#include <flame/universe/system.h>
#include <flame/universe/world.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/receiver.h>
#include <flame/universe/components/imgui.h>
#include <flame/universe/systems/scene.h>
#include <flame/universe/systems/dispatcher.h>
#include <flame/universe/systems/physics.h>
#include <flame/universe/systems/renderer.h>
#include <flame/universe/systems/imgui.h>

#if USE_IMGUI
#include <imgui.h>
#endif
#if USE_IM_FILE_DIALOG
#include <ImFileDialog.h>
#endif

namespace flame
{
	struct App
	{
		std::filesystem::path engine_path;
		std::filesystem::path resource_path;

		UniPtr<graphics::Swapchain> swapchain;
		UniPtr<graphics::CommandBuffer> commandbuffer;
		UniPtr<graphics::Fence> submit_fence;
		UniPtr<graphics::Semaphore> render_finished;

		UniPtr<World> world;
		sDispatcher* s_dispatcher = nullptr;
		sPhysics* s_physics = nullptr;
		sScene* s_scene = nullptr;
		sRenderer* s_renderer = nullptr;
		sImgui* s_imgui = nullptr;
		Entity* root = nullptr;

		graphics::Window* window = nullptr;

		void create(bool graphics_debug = true, bool always_render = false)
		{
			//{
			//	auto config = parse_ini_file(L"config.ini");
			//	for (auto& e : config.get_section_entries(""))
			//	{
			//		if (e.key == "resource_path")
			//			resource_path = e.value;
			//	}
			//}

			engine_path = getenv("FLAME_PATH");

			graphics::Device::create(graphics_debug);
			physics::Device::create();
			sound::Device::create();
			script::Instance::create();
			load_default_prefab_types();

			commandbuffer.reset(graphics::CommandBuffer::create(nullptr));
			submit_fence.reset(graphics::Fence::create(nullptr));
			render_finished.reset(graphics::Semaphore::create(nullptr));

			world.reset(World::create());
			s_imgui = sImgui::create();
			world->add_system(s_imgui);
			s_dispatcher = sDispatcher::create();
			world->add_system(s_dispatcher);
			s_physics = sPhysics::create();
			world->add_system(s_physics);
			s_scene = sScene::create();
			world->add_system(s_scene);
			s_renderer = sRenderer::create();
			if (always_render)
				s_renderer->set_always_update(true);
			world->add_system(s_renderer);

			root = world->get_root();

			auto scr_ins = script::Instance::get_default();
			scr_ins->push_object();
			scr_ins->push_pointer(world.get());
			scr_ins->set_member_name("p");
			scr_ins->set_object_type("flame::World");
			scr_ins->set_global_name("world");
			scr_ins->excute_file(L"world_setup.lua");

#if USE_IM_FILE_DIALOG
			ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void*
			{
				auto tex = graphics::Image::create(nullptr, fmt == 1 ? graphics::Format_R8G8B8A8_UNORM : graphics::Format_B8G8R8A8_UNORM,
					uvec2(w, h), 1, 1, graphics::SampleCount_1, graphics::ImageUsageSampled | graphics::ImageUsageTransferDst);

				graphics::StagingBuffer stag(nullptr, image_pitch(w * 4) * h, data);
				graphics::InstanceCB cb(nullptr);
				cb->image_barrier(tex, {}, graphics::ImageLayoutUndefined, graphics::ImageLayoutTransferDst);
				graphics::BufferImageCopy cpy;
				cpy.img_ext = uvec2(w, h);
				cb->copy_buffer_to_image(stag.get(), tex, 1, &cpy);
				cb->image_barrier(tex, {}, graphics::ImageLayoutTransferDst, graphics::ImageLayoutShaderReadOnly);
				return tex;
			};

			ifd::FileDialog::Instance().DeleteTexture = [](void* tex)
			{
				add_event([](Capture& c) {
					graphics::Queue::get(nullptr)->wait_idle();
					((graphics::Image*)c.thiz<void>())->release();
					}, Capture().set_thiz(tex));
			};
#endif
		}

		void set_main_window(graphics::Window* _window)
		{
			window = _window;

			auto native_window = window->get_native();

			s_dispatcher->setup(native_window);
			s_scene->setup(native_window);
			s_renderer->setup(window);
			s_imgui->setup(window);
		}

		void update()
		{
			world->update();
			auto w = window;
			while (w)
			{
				w->update();
				w = w->get_next();
			}
		}
	};
}
