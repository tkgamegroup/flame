// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/ui/icon.h>
#include <flame/ui/style.h>
#include <flame/ui/canvas.h>
#include "instance_private.h"

#include <flame/file.h>
#include <flame/math.h>
#include <flame/system.h>
#include <flame/font.h>
#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/framebuffer.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/pipeline.h>
#include <flame/graphics/descriptor.h>
#include <flame/graphics/swapchain.h>

#include <stdarg.h>
#include <list>
#include <sstream>

namespace flame
{
	namespace ui
	{
		void ShareData::create(graphics::Device *_d, graphics::SampleCount _sample_count)
		{
			d = _d;

			font_atlas = FontAtlas::create_from_file(L"UI/font.xml");
			if (!font_atlas)
			{
				wchar_t default_code_begin;
				wchar_t default_code_end;
				get_default_char_range(default_code_begin, default_code_end);

				std::vector<FontDescription> descs;
				descs.resize(2);

				descs[0].filename = L"c:/windows/fonts/msyh.ttc";
				descs[0].ranges.emplace_back(default_code_begin, default_code_end, true);
				descs[0].ranges.emplace_back(0x4e00, 0x9FAF, false);

				descs[1].filename = L"UI/font_awesome.ttf";
				descs[1].ranges.emplace_back(IconMin, IconMax, true);

				font_atlas = FontAtlas::create(descs, 14, 2.f);
				font_atlas->save(L"UI/font.xml");
			}

			sample_count = _sample_count;
			auto swapchain_format = graphics::get_swapchain_format();

			{
				graphics::RenderpassInfo info;
				if (sample_count != graphics::SampleCount_1)
					info.attachments.emplace_back(swapchain_format, true, sample_count);
				info.attachments.emplace_back(swapchain_format, false, graphics::SampleCount_1);
				info.subpasses[0].color_attachments.push_back(0);
				if (sample_count != graphics::SampleCount_1)
					info.subpasses[0].resolve_attachments.push_back(1);
				renderpass = graphics::Renderpass::get(d, info);
			}

			white_image = graphics::Image::create_from_file(d, L"UI/imgs/white.bmp");
			font_stroke_image = graphics::Image::create_from_bitmap(d, font_atlas->get_stroke_image());
			font_sdf_image = graphics::Image::create_from_bitmap(d, font_atlas->get_sdf_image());

			auto vib = graphics::VertexInputBufferInfo({
					graphics::Format_R32G32_SFLOAT,
					graphics::Format_R32G32_SFLOAT,
					graphics::Format_R8G8B8A8_UNORM });

			{
				graphics::GraphicsPipelineInfo info;
				info.shaders.resize(2);
				info.shaders[0].filename = L"ui/plain.vert";
				info.shaders[1].filename = L"ui/plain.frag";
				info.vi_buffers.push_back(vib);
				info.cull_mode = graphics::CullModeNone;
				info.sample_count = sample_count;
				info.blend_states[0] = graphics::BlendInfo(
					graphics::BlendFactorSrcAlpha, graphics::BlendFactorOneMinusSrcAlpha,
					graphics::BlendFactorZero, graphics::BlendFactorOneMinusSrcAlpha);
				info.renderpass = renderpass;
				pl_plain = graphics::Pipeline::create(d, info);
			}
			ds_plain = graphics::Descriptorset::create(d->dp, pl_plain->layout()->dsl(0));
			white_imageview = graphics::Imageview::get(white_image);
			for (auto i = 0; i < MaxImageviewCount; i++)
			{
				image_views[i] = white_imageview;
				ds_plain->set_imageview(0, i, white_imageview, d->sp_bi_linear);
			}

			{
				graphics::GraphicsPipelineInfo info;
				info.shaders.resize(2);
				info.shaders[0].filename = L"ui/text_stroke.vert";
				info.shaders[1].filename = L"ui/text_stroke.frag";
				info.vi_buffers.push_back(vib);
				info.cull_mode = graphics::CullModeNone;
				info.sample_count = sample_count;
				info.blend_states[0] = graphics::BlendInfo(
					graphics::BlendFactorSrc1Color, graphics::BlendFactorOneMinusSrc1Color,
					graphics::BlendFactorZero, graphics::BlendFactorZero);
				info.renderpass = renderpass;
				pl_text_stroke = graphics::Pipeline::create(d, info);
			}

			ds_text_stroke = graphics::Descriptorset::create(d->dp, pl_text_stroke->layout()->dsl(0));
			font_stroke_imageview =  graphics::Imageview::get(font_stroke_image);
			ds_text_stroke->set_imageview(0, 0, font_stroke_imageview, d->sp_bi_linear);

			{
				graphics::GraphicsPipelineInfo info;
				info.shaders.resize(2);
				info.shaders[0].filename = L"ui/text_sdf.vert";
				info.shaders[1].filename = L"ui/text_sdf.frag";
				info.vi_buffers.push_back(vib);
				info.cull_mode = graphics::CullModeNone;
				info.sample_count = sample_count;
				info.blend_states[0] = graphics::BlendInfo(
					graphics::BlendFactorSrcAlpha, graphics::BlendFactorOneMinusSrcAlpha,
					graphics::BlendFactorZero, graphics::BlendFactorOneMinusSrcAlpha);
				info.renderpass = renderpass;
				pl_text_sdf = graphics::Pipeline::create(d, info);
			}

			ds_text_sdf = graphics::Descriptorset::create(d->dp, pl_text_sdf->layout()->dsl(0));
			font_sdf_imageview = graphics::Imageview::get(font_sdf_image);
			ds_text_sdf->set_imageview(0, 0, font_sdf_imageview, d->sp_bi_linear);
		}

		void ShareData::destroy()
		{
			FontAtlas::destroy(font_atlas);

			graphics::Renderpass::release(renderpass);

			graphics::Image::destroy(white_image);
			graphics::Image::destroy(font_stroke_image);
			if (font_sdf_image)
				graphics::Image::destroy(font_sdf_image);

			graphics::Pipeline::destroy(pl_plain);
			graphics::Descriptorset::destroy(ds_plain);

			graphics::Pipeline::destroy(pl_text_stroke);
			graphics::Descriptorset::destroy(ds_text_stroke);

			graphics::Pipeline::destroy(pl_text_sdf);
			graphics::Descriptorset::destroy(ds_text_sdf);
		}

		ShareData share_data;

		void init(graphics::Device *d, graphics::SampleCount sample_count)
		{
			share_data.create(d, sample_count);
			printf("ui initialized\n");
		}

		void deinit()
		{
			share_data.destroy();
		}

		FLAME_REGISTER_FUNCTION_BEG(SwapchainData_resize, FLAME_GID(24526), "i2")
			auto &size = d[0].i2();
			auto &sd = *(SwapchainDataPrivate**)&d[1].p();

			if (size.x == 0)
				size.x = 1;
			if (size.y == 0)
				size.y = 1;

			for (auto i = 0; i < 2; i++)
			{
				if (sd->framebuffers[i])
					graphics::Framebuffer::release(sd->framebuffers[i]);

				graphics::FramebufferInfo fb_info;
				fb_info.rp = share_data.renderpass;
				if (share_data.sample_count != graphics::SampleCount_1)
				{
					if (!sd->image_ms || sd->image_ms->size != size)
					{
						if (sd->image_ms)
							graphics::Image::destroy(sd->image_ms);
						sd->image_ms = graphics::Image::create(share_data.d, graphics::get_swapchain_format(), size, 1, 1, share_data.sample_count,
							graphics::ImageUsageAttachment, graphics::MemPropDevice);
					}
					fb_info.views.push_back(graphics::Imageview::get(sd->image_ms));
				}
				fb_info.views.push_back(graphics::Imageview::get(sd->sc->get_image(i)));
				sd->framebuffers[i] = graphics::Framebuffer::get(share_data.d, fb_info);
			}
		FLAME_REGISTER_FUNCTION_END(SwapchainData_resize)

		SwapchainDataPrivate::SwapchainDataPrivate(graphics::Swapchain *_sc)
		{
			sc = _sc;
			w = sc->window();

			w->add_listener(cH("resize"), SwapchainData_resize::v, { this });

			auto surface_size = w->size;
			auto swapchain_format = graphics::get_swapchain_format();

			if (share_data.sample_count != graphics::SampleCount_1)
			{
				image_ms = graphics::Image::create(share_data.d, swapchain_format, surface_size, 1, 1, share_data.sample_count,
					graphics::ImageUsageAttachment, graphics::MemPropDevice);
			}
			else
				image_ms = nullptr;

			for (int i = 0; i < 2; i++)
			{
				graphics::FramebufferInfo fb_info;
				fb_info.rp = share_data.renderpass;
				if (share_data.sample_count != graphics::SampleCount_1)
					fb_info.views.push_back(graphics::Imageview::get(image_ms));
				fb_info.views.push_back(graphics::Imageview::get(sc->get_image(i)));
				framebuffers[i] = graphics::Framebuffer::get(share_data.d, fb_info);
			}
		}

		SwapchainDataPrivate::~SwapchainDataPrivate()
		{
			for (auto i = 0; i < 2; i++)
				graphics::Framebuffer::release(framebuffers[i]);
			if (image_ms)
				graphics::Image::destroy(image_ms);
		}

		SwapchainData *SwapchainData::create(graphics::Swapchain *sc)
		{
			return new SwapchainDataPrivate(sc);
		}

		void SwapchainData::destroy(SwapchainData *s)
		{
			delete (SwapchainDataPrivate*)s;
		}

		InstancePrivate::InstancePrivate()
		{
			root_ = std::unique_ptr<WidgetPrivate>((WidgetPrivate*)Widget::create(this));
			root_->name$ = "root";
			root_->size_policy_hori$ = SizeFitLayout;
			root_->size_policy_vert$ = SizeFitLayout;
			root_->event_attitude$ = EventAccept;
			root_->want_key_focus$ = true;
			hovering_widget_ = nullptr;
			focus_widget_ = nullptr;
			key_focus_widget_ = root_.get();
			dragging_widget_ = nullptr;
			popup_widget_ = nullptr;
			potential_doubleclick_widget_ = nullptr;
			doubleclick_timer_ = 0.f;
			char_input_compelete_ = true;

			for (auto i = 0; i < FLAME_ARRAYSIZE(key_states); i++)
				key_states[i] = KeyStateUp;

			mouse_pos = Ivec2(0);
			mouse_prev_pos = Ivec2(0);
			mouse_disp = Ivec2(0);
			mouse_scroll = 0;

			for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
				mouse_buttons[i] = KeyStateUp;

			elp_time_ = 0.f;
			total_time_ = 0.f;
		}

		FLAME_REGISTER_FUNCTION_BEG(Instance_keydown, FLAME_GID(28755), "i")
			((InstancePrivate*)d[1].p())->on_keydown(d[0].i1());
		FLAME_REGISTER_FUNCTION_END(Instance_keydown)

		FLAME_REGISTER_FUNCTION_BEG(Instance_keyup, FLAME_GID(4617), "i")
			((InstancePrivate*)d[1].p())->on_keyup(d[0].i1());
		FLAME_REGISTER_FUNCTION_END(Instance_keyup)

		FLAME_REGISTER_FUNCTION_BEG(Instance_char, FLAME_GID(31685), "i")
			((InstancePrivate*)d[1].p())->on_char(d[0].i1());
		FLAME_REGISTER_FUNCTION_END(Instance_char)

		FLAME_REGISTER_FUNCTION_BEG(Instance_mousedown, FLAME_GID(19621), "i i2")
			((InstancePrivate*)d[2].p())->on_mousedown(d[0].i1(), d[1].i2());
		FLAME_REGISTER_FUNCTION_END(Instance_mousedown)

		FLAME_REGISTER_FUNCTION_BEG(Instance_mouseup, FLAME_GID(32698), "i i2")
			((InstancePrivate*)d[2].p())->on_mouseup(d[0].i1(), d[1].i2());
		FLAME_REGISTER_FUNCTION_END(Instance_mouseup)

		FLAME_REGISTER_FUNCTION_BEG(Instance_mousemove, FLAME_GID(17801), "i2")
			((InstancePrivate*)d[1].p())->on_mousemove(d[0].i2());
		FLAME_REGISTER_FUNCTION_END(Instance_mousemove)

		FLAME_REGISTER_FUNCTION_BEG(Instance_mousescroll, FLAME_GID(9829), "i")
			((InstancePrivate*)d[1].p())->on_mousescroll(d[0].i1());
		FLAME_REGISTER_FUNCTION_END(Instance_mousescroll)

		FLAME_REGISTER_FUNCTION_BEG(Instance_resize, FLAME_GID(16038), "i2")
			((InstancePrivate*)d[1].p())->on_resize(d[0].i2());
		FLAME_REGISTER_FUNCTION_END(Instance_resize)

		InstancePrivate::InstancePrivate(Window *w) :
			InstancePrivate()
		{
			root_->size$ = w->size;

			w->add_listener(cH("key down"), Instance_keydown::v, { this });
			w->add_listener(cH("key up"), Instance_keyup::v, { this });
			w->add_listener(cH("char"), Instance_char::v, { this });
			w->add_listener(cH("mouse down"), Instance_mousedown::v, { this });
			w->add_listener(cH("mouse up"), Instance_mouseup::v, { this });
			w->add_listener(cH("mouse move"), Instance_mousemove::v, { this });
			w->add_listener(cH("mouse scroll"), Instance_mousescroll::v, { this });
			w->add_listener(cH("resize"), Instance_resize::v, { this });
		}

		inline void InstancePrivate::on_keydown(int code)
		{
			key_states[code] = KeyStateDown | KeyStateJust;
			keydown_inputs_.push_back(code);
		}

		inline void InstancePrivate::on_keyup(int code)
		{
			key_states[code] = KeyStateUp | KeyStateJust;
			keyup_inputs_.push_back(code);
		}

		inline void InstancePrivate::on_char(int ch) 
		{
			if (!char_input_compelete_ && !char_inputs_.empty())
			{
				std::string ansi;
				ansi += char_inputs_.back();
				ansi += ch;
				auto wstr = a2w(ansi);
				char_inputs_.back() = wstr[0];
				char_input_compelete_ = true;
			}
			else
			{
				char_inputs_.push_back(ch);
				if (ch >= 0x80)
					char_input_compelete_ = false;
			}
		}

		inline void InstancePrivate::on_mousedown(int mouse, const Ivec2 &pos)
		{
			mouse_buttons[mouse] = KeyStateDown | KeyStateJust;
			mouse_pos = pos;
		}

		inline void InstancePrivate::on_mouseup(int mouse, const Ivec2 &pos)
		{
			mouse_buttons[mouse] = KeyStateUp | KeyStateJust;
			mouse_pos = pos;
		}

		inline void InstancePrivate::on_mousemove(const Ivec2 &pos)
		{
			mouse_pos = pos;
		}

		inline void InstancePrivate::on_mousescroll(int scroll)
		{
			mouse_scroll = scroll;
		}

		inline void InstancePrivate::on_resize(const Ivec2 &size)
		{
			root_->set_size(Vec2(size));
		}

		inline void InstancePrivate::set_hovering_widget(WidgetPrivate *w)
		{
			if (w == hovering_widget_)
				return;
			if (hovering_widget_)
				hovering_widget_->on_mouseleave();
			hovering_widget_ = w;
			if (hovering_widget_)
				hovering_widget_->on_mouseenter();
		}

		void InstancePrivate::set_key_focus_widget(WidgetPrivate *w)
		{
			if (w == nullptr)
			{
				key_focus_widget_ = root_.get();
				return;
			}
			if (w->want_key_focus$)
			{
				key_focus_widget_ = w;
				return;
			}
			set_key_focus_widget(w->parent);
		}

		inline void InstancePrivate::set_focus_widget(WidgetPrivate *w)
		{
			focus_widget_ = w;
			set_key_focus_widget(w);
		}

		inline void InstancePrivate::set_dragging_widget(WidgetPrivate *w)
		{
			dragging_widget_ = w;
		}

		inline void InstancePrivate::set_popup_widget(WidgetPrivate *w)
		{
			popup_widget_ = w;
		}

		inline void InstancePrivate::close_popup()
		{
			if (popup_widget_)
			{
				switch (popup_widget_->class_hash$)
				{
				case cH("menubar"):
					for (auto i_c = 0; i_c < popup_widget_->children_1$.size; i_c++)
					{
						auto c = popup_widget_->children_1$[i_c];

						if (c->class_hash$ == cH("menu"))
							((wMenu*)c)->close();
					}
					break;
				case cH("menu items"):
					((wMenu*)popup_widget_->parent)->close();
					break;
				case cH("combo"):
					((wMenu*)popup_widget_)->close();
					break;
				}
				popup_widget_ = nullptr;
			}
		}

		inline void InstancePrivate::begin(float elp_time) 
		{
			processed_mouse_input = false;
			processed_keyboard_input = false;

			elp_time_ = elp_time;
			total_time_ += elp_time;
		}

		struct _Package
		{
			Vec2 mpos;
			bool mljustdown;
			bool mljustup;
			bool mrjustdown;
			int mscroll;
			Ivec2 mdisp;
			Widget *temp_dragging_widget;
			Rect curr_scissor;
			Vec2 surface_size;
			bool hovering_any_widget;
			bool clicking_nothing;
			Vec2 popup_off;
			float popup_scl;
			bool meet_popup_first;
			bool ban_event;
			Canvas *canvas;
			Vec2 show_off;
		};

		void InstancePrivate::preprocessing_children(void *__p, WidgetPrivate *w, const Array<Widget*> &children, const Vec2 &off, float scl)
		{
			auto &p = *(_Package*)__p;

			if (children.size == 0)
				return;

			if (w->clip$)
				p.curr_scissor = Rect(Vec2(0.f), w->size$ * w->global_scale) + w->global_pos;

			auto _off = w->pos$ * scl + off;
			auto _scl = w->scale$ * scl;

			for (auto i_c = children.size - 1; i_c >= 0; i_c--)
				preprocessing(&p, (WidgetPrivate*)children[i_c], w->showed, _off, _scl);

			if (w->clip$)
				p.curr_scissor = Rect(Vec2(0.f), p.surface_size);
		}

		void InstancePrivate::preprocessing(void *__p, WidgetPrivate *w, bool visible, const Vec2 &off, float scl)
		{
			auto &p = *(_Package*)__p;

			if (w == popup_widget_ && p.meet_popup_first)
			{
				p.popup_off = off;
				p.popup_scl = scl;
				p.meet_popup_first = false;
				return;
			}

			w->global_pos = w->pos$ * scl + off;
			w->global_scale = w->scale$ * scl;
			w->showed = w->visible$ && visible;

			preprocessing_children(__p, w, w->children_2$, off, scl);
			preprocessing_children(__p, w, w->children_1$, off, scl);

			if (!p.ban_event && visible && w->event_attitude$ != EventIgnore)
			{
				auto mhover = p.curr_scissor.contains(p.mpos) &&
					(Rect(w->pos$ * scl, (w->pos$ + w->size$) * scl * w->scale$) + off).contains(p.mpos);
				if (w->event_attitude$ == EventBlackHole || mhover)
				{
					if (!p.hovering_any_widget)
					{
						set_hovering_widget(w);
						p.hovering_any_widget = true;
					}
					if (p.mdisp.x != 0 || p.mdisp.y != 0)
					{
						w->on_mousemove(Vec2(p.mdisp));
						p.mdisp = Ivec2(0);
					}
					if (p.mljustdown)
					{
						p.clicking_nothing = false;
						set_focus_widget(w);
						if (mhover)
							dragging_widget_ = w;
						w->on_lmousedown(p.mpos);
						p.mljustdown = false;
					}
					if (p.mrjustdown)
					{
						w->on_rmousedown(p.mpos);
						p.mrjustdown = false;
					}
					if (p.mljustup)
					{
						if (focus_widget_ == w)
						{
							w->on_clicked();
							if (potential_doubleclick_widget_ == w)
							{
								w->on_doubleclicked();
								potential_doubleclick_widget_ = nullptr;
								doubleclick_timer_ = 0.f;
							}
							else
								potential_doubleclick_widget_ = w;
						}
						if (p.temp_dragging_widget && w != p.temp_dragging_widget)
							w->on_drop(p.temp_dragging_widget);
						p.mljustup = false;
					}
					if (p.mscroll)
					{
						w->on_mousescroll(p.mscroll);
						p.mscroll = 0;
					}
				}
			}
		}

		void InstancePrivate::show_children(void *__p, WidgetPrivate *w, const Array<Widget*> &children, bool visible, const Vec2 &off, float scl)
		{
			auto &p = *(_Package*)__p;

			if (children.size == 0)
				return;

			if (w->clip$)
			{
				p.curr_scissor = Rect(Vec2(0.f), w->size$ * w->global_scale) + w->global_pos;
				p.canvas->set_scissor(p.curr_scissor);
			}

			auto _off = w->pos$ * scl + off;
			auto _scl = w->scale$ * scl;

			for (auto i_c = 0; i_c < children.size; i_c++)
			{
				auto c = (WidgetPrivate*)children[i_c];
				show(&p, c, c->visible$ && visible, _off, _scl);
			}

			if (w->clip$)
			{
				p.curr_scissor = Rect(Vec2(0.f), p.surface_size);
				p.canvas->set_scissor(p.curr_scissor);
			}
		}

		void InstancePrivate::show(void *__p, WidgetPrivate *w, bool visible, const Vec2 &off, float scl)
		{
			auto &p = *(_Package*)__p;

			for (auto i_s = 0; i_s < w->styles$.size; i_s++)
			{
				auto s = w->styles$[i_s];

				s->datas[0].v.p = w;
				s->exec();
			}

			for (auto i_a = 0; i_a < w->animations$.size; )
			{
				auto f = w->animations$[i_a];
				auto d = f->datas;
				d[0].p() = w;

				d[1].f1() += elp_time_;
				if (d[1].f1() >= d[2].f1())
				{
					d[1].f1() = -1.f;
					f->exec();
					Function::destroy(f);
					w->animations$.remove(i_a);
				}
				else
				{
					f->exec();
					i_a++;
				}
			}

			if (visible && ((w->size$.x == 0.f && w->size$.y == 0.f) || (Rect(Vec2(0.f), w->size$ * w->global_scale) + w->global_pos).overlapping(p.curr_scissor)))
			{
				if (w == popup_widget_ && p.meet_popup_first)
				{
					p.popup_off = off;
					p.popup_scl = scl;
					p.meet_popup_first = false;
					return;
				}
				else
					w->on_draw(p.canvas, off + p.show_off, scl);
			}

			show_children(__p, w, w->children_1$, visible, off, scl);
			show_children(__p, w, w->children_2$, visible, off, scl);
		}

		void InstancePrivate::postprocessing_children(const Array<Widget*> &children)
		{
			if (children.size == 0)
				return;

			for (auto i_c = children.size - 1; i_c >= 0; i_c--)
			{
				auto c = (WidgetPrivate*)children[i_c];
				if (c->visible$)
					postprocessing(c);
			}
		}

		void InstancePrivate::postprocessing(WidgetPrivate *w)
		{
			postprocessing_children(w->children_2$);
			postprocessing_children(w->children_1$);

			w->state = StateNormal;
			if (dragging_widget_)
			{
				if (dragging_widget_ == w && hovering_widget_ == w)
					w->state = StateActive;
			}
			else
			{
				if (hovering_widget_ == w)
					w->state = StateHovering;
			}

			if (!w->delay_listener_remove.empty())
			{
				for (auto &t : w->delay_listener_remove)
					w->remove_listener(t.first, t.second);
				w->delay_listener_remove.clear();
			}

			if (!w->delay_takes_by_idx.empty())
			{
				for (auto &t : w->delay_takes_by_idx)
					w->take_child(t.first, t.second);
				w->delay_takes_by_idx.clear();
			}
			if (!w->delay_takes_by_ptr.empty())
			{
				for (auto &t : w->delay_takes_by_ptr)
					w->take_child(t);
				w->delay_takes_by_ptr.clear();
			}
			if (!w->delay_removes_by_idx.empty())
			{
				for (auto &r : w->delay_removes_by_idx)
					w->remove_child(r.first, r.second);
				w->delay_removes_by_idx.clear();
			}
			if (!w->delay_removes_by_ptr.empty())
			{
				for (auto &r : w->delay_removes_by_ptr)
					w->remove_child(r);
				w->delay_removes_by_ptr.clear();
			}
			if (!w->delay_clears.empty())
			{
				for (auto &r : w->delay_clears)
					w->clear_children(std::get<0>(r), std::get<1>(r), std::get<2>(r));
				w->delay_clears.clear();
			}
			if (!w->delay_takes.empty())
			{
				for (auto &t : w->delay_takes)
					w->take_children(std::get<0>(t), std::get<1>(t), std::get<2>(t));
				w->delay_takes.clear();
			}
			if (!w->delay_adds.empty())
			{
				for (auto &a : w->delay_adds)
				{
					w->add_child(std::get<0>(a), std::get<1>(a), std::get<2>(a));
					if (std::get<3>(a))
					{
						std::get<3>(a)->exec();
						Function::destroy(std::get<3>(a));
					}
				}
				w->delay_adds.clear();
			}
		}

		inline void InstancePrivate::end(Canvas *canvas, const Vec2 &show_off)
		{
			mouse_disp = mouse_pos - mouse_prev_pos;

			_Package p;
			p.mpos = Vec2(mouse_pos);
			p.mljustdown = just_down_M(0);
			p.mljustup = just_up_M(0);
			p.mrjustdown = just_down_M(1);
			p.mscroll = mouse_scroll;
			p.mdisp = mouse_disp;
			p.temp_dragging_widget = dragging_widget_;

			if (dragging_widget_)
			{
				if (!dragging_widget_->visible$ || !pressing_M(0))
					dragging_widget_ = nullptr;
				else if (dragging_widget_->event_attitude$ != EventIgnore)
				{
					dragging_widget_->on_mousemove(Vec2(p.mdisp));
					p.mdisp = Ivec2(0);
				}
			}

			if (focus_widget_)
			{
				if (!focus_widget_->visible$)
					focus_widget_ = nullptr;
			}
			if (key_focus_widget_)
			{
				if (!key_focus_widget_->visible$)
					key_focus_widget_ = nullptr;
				else if (key_focus_widget_->event_attitude$ != EventIgnore)
				{
					for (auto &code : keydown_inputs_)
						key_focus_widget_->on_keydown(code);
					for (auto &code : keyup_inputs_)
						key_focus_widget_->on_keyup(code);
					for (auto &ch : char_inputs_)
						key_focus_widget_->on_char(ch);
				}
			}
			keydown_inputs_.clear();
			keyup_inputs_.clear();
			char_inputs_.clear();

			if (p.mljustdown)
			{
				focus_widget_ = nullptr;
				key_focus_widget_ = nullptr;
			}

			p.surface_size = root_->size$;
			p.curr_scissor = Rect(Vec2(0.f), p.surface_size);
			p.hovering_any_widget = false;
			p.clicking_nothing = p.mljustdown;
			p.meet_popup_first = true;
			p.ban_event = popup_widget_;
			p.canvas = canvas;
			p.show_off = show_off;

			preprocessing(&p, root_.get(), true, Vec2(0.f), 1.f);
			p.ban_event = false;
			if (popup_widget_)
				preprocessing(&p, popup_widget_, true, p.popup_off, p.popup_scl);
			if (!p.hovering_any_widget)
				set_hovering_widget(nullptr);
			if (p.clicking_nothing && popup_widget_)
				close_popup();

			if (dragging_widget_)
			{
				if (!dragging_widget_->visible$ || !pressing_M(0))
					dragging_widget_ = nullptr;
			}

			if (potential_doubleclick_widget_)
			{
				doubleclick_timer_ += elp_time_;
				if (doubleclick_timer_ > 0.5f)
				{
					potential_doubleclick_widget_ = nullptr;
					doubleclick_timer_ = 0.f;
				}
			}

			p.curr_scissor = Rect(Vec2(0.f), p.surface_size);

			p.meet_popup_first = true;
			show(&p, root_.get(), true, Vec2(0.f), 1.f);
			if (popup_widget_)
				show(&p, popup_widget_, true, p.popup_off, p.popup_scl);

			postprocessing(root_.get());

			for (int i = 0; i < FLAME_ARRAYSIZE(key_states); i++)
				key_states[i] &= ~KeyStateJust;

			for (auto i = 0; i < FLAME_ARRAYSIZE(mouse_buttons); i++)
				mouse_buttons[i] &= ~KeyStateJust;

			mouse_prev_pos = mouse_pos;
			mouse_scroll = 0;
		}

		//void Drawlist::draw_grid(const Vec2 &wnd_off, const Vec2 &off, const Vec2 &size)
		//{
		//	for (auto i = mod((int)off.x, 100); i.y < size.x; i.y += 100, i.x--)
		//	{
		//		if (i.y < 0)
		//			continue;
		//		add_line(Vec2(i.y, 0.f) + wnd_off, Vec2(i.y, size.y) + wnd_off, Vec4(1.f));
		//		add_text_stroke(Vec2(i.y + 4, 0.f) + wnd_off, Vec4(1.f), "%d", i.x * -100);
		//	}
		//	for (auto i = mod((int)off.y, 100); i.y < size.y; i.y += 100, i.x--)
		//	{
		//		if (i.y < 0)
		//			continue;
		//		add_line(Vec2(0.f, i.y) + wnd_off, Vec2(size.x, i.y) + wnd_off, Vec4(1.f));
		//		add_text_stroke(Vec2(4.f, i.y) + wnd_off, Vec4(1.f), "%d", i.x * -100);
		//	}
		//}

		Ivec2 Instance::size() const
		{
			return Ivec2(((InstancePrivate*)this)->root_->size$);
		}

		void Instance::on_keydown(int code)
		{
			((InstancePrivate*)this)->on_keydown(code);
		}

		void Instance::on_keyup(int code)
		{
			((InstancePrivate*)this)->on_keyup(code);
		}

		void Instance::on_char(int ch)
		{
			((InstancePrivate*)this)->on_char(ch);
		}

		void Instance::on_mousedown(int mouse, const Ivec2 &pos)
		{
			((InstancePrivate*)this)->on_mousedown(mouse, pos);
		}

		void Instance::on_mouseup(int mouse, const Ivec2 &pos)
		{
			((InstancePrivate*)this)->on_mouseup(mouse, pos);
		}

		void Instance::on_mousemove(const Ivec2 &pos)
		{
			((InstancePrivate*)this)->on_mousemove(pos);
		}

		void Instance::on_mousescroll(int scroll)
		{
			((InstancePrivate*)this)->on_mousescroll(scroll);
		}

		void Instance::on_resize(const Ivec2 &size)
		{
			((InstancePrivate*)this)->on_resize(size);
		}

		graphics::Imageview *Instance::imageview(int index)
		{
			auto v = share_data.image_views[index];
			if (v == share_data.white_imageview)
				v = nullptr;
			return v;
		}

		void Instance::set_imageview(int index, graphics::Imageview *v)
		{
			if (!v)
				v = share_data.white_imageview;
			share_data.image_views[index] = v;
			share_data.ds_plain->set_imageview(0, index, v, share_data.d->sp_bi_linear);
		}

		Widget *Instance::root()
		{
			return ((InstancePrivate*)this)->root_.get();
		}

		Widget *Instance::hovering_widget()
		{
			return ((InstancePrivate*)this)->hovering_widget_;
		}

		Widget *Instance::focus_widget()
		{
			return ((InstancePrivate*)this)->focus_widget_;
		}

		Widget *Instance::key_focus_widget() 
		{
			return ((InstancePrivate*)this)->key_focus_widget_;
		}

		Widget *Instance::dragging_widget()
		{
			return ((InstancePrivate*)this)->dragging_widget_;
		}

		Widget *Instance::popup_widget()
		{
			return ((InstancePrivate*)this)->popup_widget_;
		}

		void Instance::set_hovering_widget(Widget *w)
		{
			((InstancePrivate*)this)->set_hovering_widget((WidgetPrivate*)w);
		}

		void Instance::set_focus_widget(Widget *w)
		{
			((InstancePrivate*)this)->set_focus_widget((WidgetPrivate*)w);
		}

		void Instance::set_key_focus_widget(Widget *w)
		{
			((InstancePrivate*)this)->set_key_focus_widget((WidgetPrivate*)w);
		}

		void Instance::set_dragging_widget(Widget *w)
		{
			((InstancePrivate*)this)->set_dragging_widget((WidgetPrivate*)w);
		}

		void Instance::set_popup_widget(Widget *w)
		{
			((InstancePrivate*)this)->set_popup_widget((WidgetPrivate*)w);
		}

		void Instance::close_popup()
		{
			((InstancePrivate*)this)->close_popup();
		}

		void Instance::begin(float elp_time)
		{
			((InstancePrivate*)this)->begin(elp_time);
		}

		void Instance::end(Canvas *canvas, const Vec2 &show_off)
		{
			((InstancePrivate*)this)->end(canvas, show_off);
		}

		float Instance::total_time() const
		{
			return ((InstancePrivate*)this)->total_time_;
		}

		Instance *Instance::create()
		{
			return new InstancePrivate;
		}

		Instance *Instance::create(Window *w)
		{
			return new InstancePrivate(w);
		}

		void Instance::destroy(Instance *i)
		{
			delete (InstancePrivate*)i;
		}
	}
}

