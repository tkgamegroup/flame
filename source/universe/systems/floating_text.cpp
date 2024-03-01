#include "../../graphics/canvas.h"
#include "../entity_private.h"
#include "../components/node_private.h"
#include "../components/camera_private.h"
#include "../systems/renderer_private.h"
#include "floating_text_private.h"

namespace flame
{
	uint FloatingTextPrivate::add(const std::wstring& text, uint font_size, const cvec4& color, float duration, cNodePtr bind_target,
		const vec3& offset_3d, const vec2& offset_2d, const vec2& speed)
	{
		auto& item = items.emplace_back();
		auto id = next_id++;
		item.id = id;
		item.time = duration;
		item.text = text;
		item.font_size = font_size;
		item.color = color;
		item.bind_target = bind_target;
		item.offset_3d = offset_3d;
		item.offset_2d = offset_2d;
		item.speed = speed;
		if (bind_target)
		{
			bind_target->entity->message_listeners.add([this, id](uint h, void*, void*) {
				if (h == "destroyed"_h)
					remove(id);
			});
		}
		if (!ev)
		{
			ev = add_event([this]() {
				update();
				if (items.empty())
				{
					ev = nullptr;
					return false;
				}
				return true; 
			});
		}
		return item.id;
	}

	void FloatingTextPrivate::remove(uint id)
	{
		std::erase_if(items, [=](const auto& i) {
			return i.id == id;
		});
		if (items.empty() && ev)
		{
			remove_event(ev);
			ev = nullptr;
		}
	}

	void FloatingTextPrivate::update()
	{
		for (auto it = items.begin(); it != items.end(); )
		{
			if (it->time > 0)
			{
				auto renderer = sRenderer::instance();
				auto render_task = renderer->render_tasks.front().get();
				auto canvas = render_task->canvas;
				auto pos = vec2(0.f);
				pos = render_task->camera->world_to_screen(it->bind_target ? it->bind_target->global_pos() + it->offset_3d : it->offset_3d);
				pos += it->offset_2d;
				it->offset_2d += it->speed;
				canvas->add_text(canvas->default_font_atlas, it->font_size, pos, it->text, it->color, 0.5f, 0.2f);

				it->time -= delta_time;
				++it;
			}
			else
				it = items.erase(it);
		}
	}

	static FloatingTextPtr _instance = nullptr;

	struct FloatingTextInstance : FloatingText::Instance
	{
		FloatingTextPtr operator()() override
		{
			if (_instance == nullptr)
				_instance = new FloatingTextPrivate();
			return _instance;
		}
	}FloatingText_instance;
	FloatingText::Instance& FloatingText::instance = FloatingText_instance;
}
