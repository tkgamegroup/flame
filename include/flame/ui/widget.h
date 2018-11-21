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

#include "UI.h"

#include <flame/math.h>
#include <flame/function.h>

namespace flame
{
	struct XmlNode;

	namespace ui
	{
		struct Instance;
		struct Canvas;

		/*
		           pos                        size.x
		               +------------------------------------------------
		               |	              top inner padding              
		               |			****************************         
		               |	 left   *                          *  right  
		        size.y |	 inner  *          content         *  inner  
		               |	padding *                          * padding 
		               |	        ****************************         
		               |			     bottom inner padding            
		*/

		enum SizePolicy
		{
			SizeFixed,
			SizeFitChildren,
			SizeFitLayout,
			SizeGreedy
		};

		enum Align
		{
			AlignFree,
			AlignLittleEnd,
			AlignLargeEnd,
			AlignMiddle,
			AlignLeft,
			AlignRight,
			AlignTop,
			AlignBottom,
			AlignLeftTop,
			AlignLeftBottom,
			AlignRightTop,
			AlignRightBottom,
			AlignLeftNoPadding,
			AlignRightNoPadding,
			AlignTopNoPadding,
			AlignBottomNoPadding,
			AlignLeftTopNoPadding,
			AlignLeftBottomNoPadding,
			AlignRightTopNoPadding,
			AlignRightBottomNoPadding,
			AlignCenter,
			AlignLeftOutside,
			AlignRightOutside,
			AlignTopOutside,
			AlignBottomOutside
		};

		enum EventAttitude
		{
			EventAccept,
			EventIgnore,
			EventBlackHole
		};

		enum LayoutType
		{
			LayoutFree,
			LayoutVertical,
			LayoutHorizontal,
			LayoutGrid
		};

		enum State
		{
			StateNormal,
			StateHovering,
			StateActive
		};

		struct Widget : R
		{
			uint class_hash$;

			char *name$;
			uint name_hash;

			Vec2 pos$;
			Vec2 size$;

			float alpha$;
			float scale$;

			Vec4 inner_padding$; // L R T B
			float layout_padding$;

			Vec4 background_offset$; // L T R B
			float background_round_radius$;
			int background_round_flags$;
			float background_frame_thickness$;
			Bvec4 background_col$;
			Bvec4 background_frame_col$;
			float background_shaow_thickness$;

			SizePolicy size_policy_hori$;
			SizePolicy size_policy_vert$;

			Align align$;

			LayoutType layout_type$;
			float item_padding$;
			int grid_hori_count$;
			bool clip$;

			float scroll_offset$;

			EventAttitude event_attitude$;
			bool want_key_focus$;

			bool visible$;

			Vec2 global_pos; // vaild after instance processing
			float global_scale; // vaild after instance processing

			bool cliped; // valid after arranging by parent
			int content_size; // valid after arranging
			bool showed; // vaild after instance processing
			State state; // vaild after instance processing

			FLAME_UI_EXPORTS void set_name(const char *_name);

			FLAME_UI_EXPORTS void set_width(float x, Widget *sender = nullptr);
			FLAME_UI_EXPORTS void set_height(float y, Widget *sender = nullptr);
			FLAME_UI_EXPORTS void set_size(const Vec2 &v, Widget *sender = nullptr);

			FLAME_UI_EXPORTS void set_visibility(bool v);

			FLAME_UI_EXPORTS Instance *instance() const;
			FLAME_UI_EXPORTS Widget *parent() const;
			FLAME_UI_EXPORTS int layer() const;

			FLAME_UI_EXPORTS void add_child(Widget *w, int layer = 0, int pos = -1, bool delay = false, void(*func)(CommonData*) = nullptr, char *capture_fmt = nullptr, ...);
			FLAME_UI_EXPORTS void remove_child(int layer, int idx, bool delay = false);
			FLAME_UI_EXPORTS void remove_child(Widget *w, bool delay = false);
			FLAME_UI_EXPORTS void take_child(int layer, int idx, bool delay = false);
			FLAME_UI_EXPORTS void take_child(Widget *w, bool delay = false);
			FLAME_UI_EXPORTS void clear_children(int layer, int begin, int end, bool delay = false);
			FLAME_UI_EXPORTS void take_children(int layer, int begin, int end, bool delay = false);
			FLAME_UI_EXPORTS void remove_from_parent(bool delay = false);
			FLAME_UI_EXPORTS void take_from_parent(bool delay = false);
			FLAME_UI_EXPORTS int children_count(int layer) const;
			FLAME_UI_EXPORTS Widget *child(int layer, int idx);
			FLAME_UI_EXPORTS int find_child(Widget *w);

			FLAME_UI_EXPORTS float get_content_size() const;

			FLAME_UI_EXPORTS void arrange();

			FLAME_UI_EXPORTS void resize_closet(int count);
			FLAME_UI_EXPORTS void set_curr_closet(int idx);

			// (Widget *w)
			FLAME_UI_EXPORTS void add_style(int closet_idx, PF pf, char *capture_fmt, ...);

			// (Widget *w, float curr_time)
			FLAME_UI_EXPORTS void add_animation(float total_time, bool looping, PF pf, char *capture_fmt, ...);

			FLAME_UI_EXPORTS void on_draw(Canvas *c, const Vec2 &off, float scl);
			FLAME_UI_EXPORTS void on_mouseenter();
			FLAME_UI_EXPORTS void on_mouseleave();
			FLAME_UI_EXPORTS void on_lmousedown(const Vec2 &mpos);
			FLAME_UI_EXPORTS void on_rmousedown(const Vec2 &mpos);
			FLAME_UI_EXPORTS void on_mousemove(const Vec2 &disp);
			FLAME_UI_EXPORTS void on_clicked();
			FLAME_UI_EXPORTS void on_doubleclicked();
			FLAME_UI_EXPORTS void on_mousescroll(int scroll);
			FLAME_UI_EXPORTS void on_keydown(int code);
			FLAME_UI_EXPORTS void on_keyup(int code);
			FLAME_UI_EXPORTS void on_char(wchar_t ch);
			FLAME_UI_EXPORTS void on_drop(Widget *src);

			FLAME_UI_EXPORTS void report_changed() const;

			FLAME_UI_EXPORTS void add_draw_command(PF pf, char *capture_fmt, ...);
			FLAME_UI_EXPORTS void add_default_draw_command();
			FLAME_UI_EXPORTS void remove_draw_command(int idx);

			// "mouse enter":      ()
			// "mouse leave":      ()
			// "left mouse down":  (Vec2 pos)
			// "right mouse down": (Vec2 pos)
			// "mouse move":       (Vec2 pos)
			// "mouse scroll":     (int scroll)
			// "clicked":          ()
			// "double clicked":   ()
			// "key down":         (int key)
			// "char":             (int ch)
			// "char filter":      (int ch, out int pass)
			// "drop":             (Widget *w)
			// "changed":          ()
			// "add child":        (Widget *w)
			// "remove child":     (Widget *w)

			FLAME_UI_EXPORTS Function *add_listener(unsigned int type, PF pf, char *capture_fmt, ...);
			FLAME_UI_EXPORTS void remove_listener(unsigned int type, Function *f, bool delay = false);

			FLAME_UI_EXPORTS void resize_data_storage(int count);
			FLAME_UI_EXPORTS CommonData &data_storage(int idx);

			FLAME_UI_EXPORTS void resize_string_storage(int count);
			FLAME_UI_EXPORTS const wchar_t *string_storage(int idx);
			FLAME_UI_EXPORTS int string_storage_len(int idx);
			FLAME_UI_EXPORTS void set_string_storage(int idx, const wchar_t *str);

			FLAME_UI_EXPORTS void save(XmlNode *dst);

			FLAME_UI_EXPORTS static Widget *create(Instance *ui);
			FLAME_UI_EXPORTS static Widget *create_from_file(Instance *ui, XmlNode *src);
			FLAME_UI_EXPORTS static void destroy(Widget *w);
		};

		typedef Widget* WidgetPtr;

		struct wLayout : Widget
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS static wLayout *create(Instance *ui);
		};

		typedef wLayout* wLayoutPtr;

		struct wCheckbox : Widget
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS int &checked();

			FLAME_UI_EXPORTS static wCheckbox *create(Instance *ui);
		};

		typedef wCheckbox* wCheckboxPtr;

		struct wText : Widget
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS Bvec4 &text_col();
			FLAME_UI_EXPORTS float &sdf_scale();

			FLAME_UI_EXPORTS const wchar_t *text();
			FLAME_UI_EXPORTS int text_len();
			FLAME_UI_EXPORTS void set_text(const wchar_t *str);

			FLAME_UI_EXPORTS void set_size_auto(const wchar_t *str);
			FLAME_UI_EXPORTS void set_size_auto();
			FLAME_UI_EXPORTS void set_text_and_size(const wchar_t *str);

			FLAME_UI_EXPORTS static wText *create(Instance *ui);
		};

		typedef wText* wTextPtr;

		struct wButton : wText
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS void set_classic(const wchar_t *text, float sdf_scale = -1.f, float alpha = 1.f);

			FLAME_UI_EXPORTS static wButton *create(Instance *ui);
		};

		typedef wButton* wButtonPtr;

		struct wToggle : wText
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS int &toggled();

			FLAME_UI_EXPORTS void set_toggle(bool v);

			FLAME_UI_EXPORTS static wToggle *create(Instance *ui);
		};

		typedef wToggle* wTogglePtr;

		struct wMenuItem : wButton
		{
			FLAME_UI_EXPORTS void init(const wchar_t *title);

			FLAME_UI_EXPORTS static wMenuItem *create(Instance *ui, const wchar_t *title);
		};

		typedef wMenuItem* wMenuItemPtr;

		struct wMenu : wLayout
		{
			FLAME_UI_EXPORTS void init(const wchar_t *title, float alpha = 1.f);

			FLAME_UI_EXPORTS int &sub();
			FLAME_UI_EXPORTS int &opened();
			FLAME_UI_EXPORTS wButtonPtr &w_btn();
			FLAME_UI_EXPORTS wTextPtr &w_rarrow();
			FLAME_UI_EXPORTS wLayoutPtr &w_items();

			FLAME_UI_EXPORTS void open();
			FLAME_UI_EXPORTS void popup(const Vec2 &pos);
			FLAME_UI_EXPORTS void close();

			FLAME_UI_EXPORTS static wMenu *create(Instance *ui, const wchar_t *title, float alpha = 1.f);
		};

		typedef wMenu* wMenuPtr;

		struct wMenuBar : wLayout
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS static wMenuBar *create(Instance *ui);
		};

		typedef wMenuBar* wMenuBarPtr;

		struct wCombo : wMenu
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS int &sel();
			
			FLAME_UI_EXPORTS void set_sel(int idx);

			FLAME_UI_EXPORTS static wCombo *create(Instance *ui);
		};

		typedef wCombo* wComboPtr;

		struct wEdit : wText
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS int &cursor();

			FLAME_UI_EXPORTS void set_size_by_width(float width);
			FLAME_UI_EXPORTS void add_char_filter_int();
			FLAME_UI_EXPORTS void add_char_filter_float();

			FLAME_UI_EXPORTS static wEdit *create(Instance *ui);
		};

		typedef wEdit* wEditPtr;

		struct wImage : Widget
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS int &id();
			FLAME_UI_EXPORTS Vec2 &uv0();
			FLAME_UI_EXPORTS Vec2 &uv1();
			FLAME_UI_EXPORTS int &stretch();
			FLAME_UI_EXPORTS Vec4 &border(); // L R T B

			FLAME_UI_EXPORTS static wImage *create(Instance *ui);
		};

		typedef wImage* wImagePtr;

		struct wSizeDrag : Widget
		{
			FLAME_UI_EXPORTS void init(Widget *target);

			FLAME_UI_EXPORTS Vec2 &min_size();

			FLAME_UI_EXPORTS static wSizeDrag *create(Instance *ui, Widget *target);
		};

		typedef wSizeDrag* wSizeDragPtr;

		struct wScrollbar : Widget
		{
			FLAME_UI_EXPORTS void init(Widget *target);

			FLAME_UI_EXPORTS wButtonPtr &w_btn();
			FLAME_UI_EXPORTS WidgetPtr &w_target();

			FLAME_UI_EXPORTS void scroll(int v);

			FLAME_UI_EXPORTS static wScrollbar *create(Instance *ui, Widget *target);
		};

		typedef wScrollbar* wScrollbarPtr;

		struct wListItem : wLayout
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS wButtonPtr &w_btn();

			FLAME_UI_EXPORTS static wListItem *create(Instance *ui);
		};

		typedef wListItem* wListItemPtr;

		struct wList : wLayout
		{
			FLAME_UI_EXPORTS void init();
			
			FLAME_UI_EXPORTS wListItemPtr &w_sel();
			FLAME_UI_EXPORTS wScrollbarPtr &w_scrollbar();

			FLAME_UI_EXPORTS static wList *create(Instance *ui);
		};

		typedef wList* wListPtr;

		struct wTreeNode : wLayout
		{
			FLAME_UI_EXPORTS void init(const wchar_t *title, const Bvec4 &normal_col, const Bvec4 &else_col);

			FLAME_UI_EXPORTS wButtonPtr &w_btn();
			FLAME_UI_EXPORTS wLayoutPtr &w_items();
			FLAME_UI_EXPORTS wTextPtr &w_larrow();

			FLAME_UI_EXPORTS static wTreeNode *create(Instance *ui, const wchar_t *title, const Bvec4 &normal_col, const Bvec4 &else_col);
		};

		typedef wTreeNode* wTreeNodePtr;

		//struct wDialog : wLayout
		//{
		//	FLAME_UI_EXPORTS void init(const wchar_t *title, float sdf_scale = -1.f, bool resize = false);

		//	FLAME_UI_EXPORTS wTextPtr &w_title();
		//	FLAME_UI_EXPORTS wLayoutPtr &w_content();
		//	FLAME_UI_EXPORTS wSizeDragPtr &w_sizedrag();

		//	FLAME_UI_EXPORTS static wDialog *create(Instance *ui, const wchar_t *title, float sdf_scale = -1.f, bool resize = false);
		//};

		//struct wMessageDialog : wDialog 
		//{
		//	FLAME_UI_EXPORTS void init(const wchar_t *title, float sdf_scale, const wchar_t *text);

		//	FLAME_UI_EXPORTS wTextPtr &w_text();
		//	FLAME_UI_EXPORTS wButtonPtr &w_ok();

		//	FLAME_UI_EXPORTS static wMessageDialog *create(Instance *ui, const wchar_t *title, float sdf_scale, const wchar_t *text);
		//};

		//struct wYesNoDialog : wDialog
		//{
		//	FLAME_UI_EXPORTS void init(const wchar_t *title, float sdf_scale, const wchar_t *text, const wchar_t *yes_text, const wchar_t *no_text, const std::function<void(bool)> &callback);

		//	FLAME_UI_EXPORTS wTextPtr &w_text();
		//	FLAME_UI_EXPORTS wLayoutPtr &w_buttons();
		//	FLAME_UI_EXPORTS wButtonPtr &w_yes();
		//	FLAME_UI_EXPORTS wButtonPtr &w_no();

		//	FLAME_UI_EXPORTS static wYesNoDialog *create(Instance *ui, const wchar_t *title, float sdf_scale, const wchar_t *text, const wchar_t *yes_text, const wchar_t *no_text, const std::function<void(bool)> &callback);
		//};

		//struct wInputDialog : wDialog
		//{
		//	FLAME_UI_EXPORTS void init(const wchar_t *title, float sdf_scale, const std::function<void(bool ok, const wchar_t *input)> &callback);

		//	FLAME_UI_EXPORTS wEditPtr &w_input();
		//	FLAME_UI_EXPORTS wLayoutPtr &w_buttons();
		//	FLAME_UI_EXPORTS wButtonPtr &w_ok();
		//	FLAME_UI_EXPORTS wButtonPtr &w_cancel();

		//	FLAME_UI_EXPORTS static wInputDialog *create(Instance *ui, const wchar_t *title, float sdf_scale, const std::function<void(bool ok, const wchar_t *input)> &callback);
		//};

		//struct wFileDialog : wDialog
		//{
		//	FLAME_UI_EXPORTS void init(const wchar_t *title, int io, const std::function<void(bool ok, const wchar_t *filename)> &callback, const wchar_t *exts = nullptr);

		//	FLAME_UI_EXPORTS wMenuBarPtr &w_pathstems();
		//	FLAME_UI_EXPORTS wListPtr &w_list();
		//	FLAME_UI_EXPORTS wEditPtr &w_input();
		//	FLAME_UI_EXPORTS wComboPtr &w_ext();
		//	FLAME_UI_EXPORTS wLayoutPtr &w_buttons();
		//	FLAME_UI_EXPORTS wButtonPtr &w_ok();
		//	FLAME_UI_EXPORTS wButtonPtr &w_cancel();

		//	FLAME_UI_EXPORTS const wchar_t *curr_path();
		//	FLAME_UI_EXPORTS int curr_path_len();
		//	FLAME_UI_EXPORTS void set_curr_path(const wchar_t *path);
		//	FLAME_UI_EXPORTS const wchar_t *curr_exts();
		//	FLAME_UI_EXPORTS int curr_exts_len();
		//	FLAME_UI_EXPORTS void set_curr_exts(const wchar_t *exts);

		//	FLAME_UI_EXPORTS void set_path(const wchar_t *path);

		//	FLAME_UI_EXPORTS static wFileDialog *create(Instance *ui, const wchar_t *title, int io, const std::function<void(bool ok, const wchar_t *filename)> &callback, const wchar_t *exts = nullptr);
		//};
	}
}
