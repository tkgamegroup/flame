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

#pragma once

#include <flame/window.h>

#include "widget_private.h"
#include <flame/ui/instance.h>

#include <vector>

namespace flame
{
	struct Img;
	struct FontAtlas;

	namespace graphics
	{
		struct Buffer;
		struct Image;
		struct Imageview;
		struct Renderpass;
		struct Framebuffer;
		struct Pipeline;
		struct Sampler;
		struct Descriptorset;
		struct Swapchain;
	}

	namespace ui
	{
		struct ShareData 
		{
			FontAtlas *font_atlas;

			graphics::Device *d;

			graphics::SampleCount sample_count;

			graphics::Renderpass *renderpass;

			graphics::Image *white_image;
			graphics::Image *font_stroke_image;
			graphics::Image *font_sdf_image;

			graphics::Imageview *white_imageview;
			graphics::Imageview *font_stroke_imageview;
			graphics::Imageview *font_sdf_imageview;

			graphics::Imageview *image_views[MaxImageviewCount];

			graphics::Pipeline *pl_plain;
			graphics::Descriptorset *ds_plain;

			graphics::Pipeline *pl_text_stroke;
			graphics::Descriptorset *ds_text_stroke;

			graphics::Pipeline *pl_text_sdf;
			graphics::Descriptorset *ds_text_sdf;

			void create(graphics::Device *_d, graphics::SampleCount _sample_count);
			void destroy();
		};

		extern ShareData share_data;

		struct SwapchainDataPrivate : SwapchainData
		{
			Window *w;
			graphics::Swapchain *sc;

			graphics::Image *image_ms;

			graphics::Framebuffer *framebuffers[2];

			SwapchainDataPrivate(graphics::Swapchain *_sc);
			~SwapchainDataPrivate();
		};

		typedef SwapchainDataPrivate* SwapchainDataPrivatePtr;

		struct InstancePrivate : Instance
		{
			Ivec2 mouse_prev_pos;

			std::unique_ptr<WidgetPrivate> root_;
			WidgetPrivate *hovering_widget_;
			WidgetPrivate *focus_widget_;
			WidgetPrivate *key_focus_widget_;
			WidgetPrivate *dragging_widget_;
			WidgetPrivate *popup_widget_;
			bool popup_widget_modual_;
			WidgetPrivate *potential_doubleclick_widget_;
			float doubleclick_timer_;
			std::vector<int> keydown_inputs_;
			std::vector<int> keyup_inputs_;
			std::vector<wchar_t> char_inputs_;
			bool char_input_compelete_;

			float elp_time_; // second
			float total_time_;

			InstancePrivate();
			InstancePrivate(Window *w);

			void on_key(KeyState action, int value);
			void on_mouse(KeyState action, MouseKey key, const Ivec2 &pos);
			void on_resize(const Ivec2 &size);

			void set_hovering_widget(WidgetPrivate *w);
			void set_key_focus_widget(WidgetPrivate *w);
			void set_focus_widget(WidgetPrivate *w);
			void set_dragging_widget(WidgetPrivate *w);
			void set_popup_widget(WidgetPrivate *w, bool modual = false);
			void close_popup();

			void begin(float elp_time);
			void preprocessing_children(void *__p, WidgetPrivate *w, const Array<Widget*> &children, const Vec2 &off, float scl);
			void preprocessing(void *__p, WidgetPrivate *w, bool visible, const Vec2 &off, float scl);
			void show_children(void *__p, WidgetPrivate *w, const Array<Widget*> &children, bool visible, const Vec2 &off, float scl);
			void show(void *__p, WidgetPrivate *w, bool visible, const Vec2 &off, float scl);
			void postprocessing_children(const Array<Widget*> &children);
			void postprocessing(WidgetPrivate *w);
			void end(Canvas *canvas, const Vec2 &show_off = Vec2(0.f));
		};
	}
}

