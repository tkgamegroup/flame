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

#include <flame/math.h>
#include <flame/array.h>
#include <flame/string.h>
#include <flame/function.h>
#include <flame/input.h>
#include <flame/ui/ui.h>
#include <flame/ui/canvas.h>

#include <vector>

namespace flame
{
	struct VaribleInfo;
	struct SerializableNode;

	namespace ui
	{
		struct Instance;

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

		struct Widget;
		typedef Widget* WidgetPtr;

		struct Widget : R
		{
			uint class_hash$;

			String name$;

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

			Array<Widget*> children_1$;
			Array<Widget*> children_2$;

			bool draw_default$;
			FLAME_PARM_PACKAGE_BEGIN(ExtraDrawParm)
				FLAME_PARM_PACKAGE_PARM(CanvasPtr, canvas, p)
				FLAME_PARM_PACKAGE_PARM(Vec2, off, f2)
				FLAME_PARM_PACKAGE_PARM(float, scl, f1)

				FLAME_PARM_PACKAGE_PARM_SIZE

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
			FLAME_PARM_PACKAGE_END
			Array<Function*> extra_draws$;

			int closet_id$;
			FLAME_PARM_PACKAGE_BEGIN(StyleParm)
				FLAME_PARM_PACKAGE_PARM_SIZE

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
				FLAME_PARM_PACKAGE_DEFAULT_CAPT(int, closet_id, i1)
			FLAME_PARM_PACKAGE_END
			Array<Function*> styles$;

			FLAME_PARM_PACKAGE_BEGIN(AnimationParm)
				FLAME_PARM_PACKAGE_PARM(float, time, f1)

				FLAME_PARM_PACKAGE_PARM_SIZE

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
				FLAME_PARM_PACKAGE_DEFAULT_CAPT(float, duration, f1)
				FLAME_PARM_PACKAGE_DEFAULT_CAPT(int, looping, i1)
			FLAME_PARM_PACKAGE_END
			Array<Function*> animations$;

			enum Listener
			{
				ListenerFocus,
				ListenerKey,
				ListenerMouse,
				ListenerDrop,
				ListenerChanged,
				ListenerChild
			};

			FLAME_PARM_PACKAGE_BEGIN(FoucusListenerParm)
				FLAME_PARM_PACKAGE_PARM(FocusType, type, i1)
				FLAME_PARM_PACKAGE_PARM(int, focus_or_keyfocus, i1)

				FLAME_PARM_PACKAGE_PARM_SIZE

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
			FLAME_PARM_PACKAGE_END

			FLAME_PARM_PACKAGE_BEGIN(KeyListenerParm)
				/*
					- when key down/up, action is KeyStateDown or KeyStateUp, value is Key
					- when char, action is KeyStateNull, value is ch
				*/
				FLAME_PARM_PACKAGE_PARM(KeyState, action, i1)
				FLAME_PARM_PACKAGE_PARM(int, value, i1)

				FLAME_PARM_PACKAGE_PARM_SIZE

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
			FLAME_PARM_PACKAGE_END

			FLAME_PARM_PACKAGE_BEGIN(MouseListenerParm)
				/*
					- when enter/leave, action is KeyStateDown or KeyStateUp, key is Mouse_Null
					- when down/up, action is KeyStateDown or KeyStateUp, key is MouseKey, value is pos
					- when move, action is KeyStateNull, key is Mouse_Null, value is disp
					- when scroll, action is KeyStateNull, key is Mouse_Middle, value.x is scroll value
					- when clicked, action is KeyStateDown | KeyStateUp | (KeyStateDouble ? for double clicked), key is Mouse_Null
				*/
				FLAME_PARM_PACKAGE_PARM(KeyState, action, i1)
				FLAME_PARM_PACKAGE_PARM(MouseKey, key, i1)
				FLAME_PARM_PACKAGE_PARM(Vec2, value, f2)

				FLAME_PARM_PACKAGE_PARM_SIZE

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
			FLAME_PARM_PACKAGE_END

			FLAME_PARM_PACKAGE_BEGIN(DropListenerParm)
				FLAME_PARM_PACKAGE_PARM(WidgetPtr, src, p)

				FLAME_PARM_PACKAGE_PARM_SIZE

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
			FLAME_PARM_PACKAGE_END

			FLAME_PARM_PACKAGE_BEGIN(ChangedListenerParm)
				FLAME_PARM_PACKAGE_PARM_SIZE

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
			FLAME_PARM_PACKAGE_END

			enum ChildOp
			{
				ChildAdd,
				ChildRemove
			};
			FLAME_PARM_PACKAGE_BEGIN(ChildListenerParm)
				FLAME_PARM_PACKAGE_PARM(ChildOp, op, i1)
				FLAME_PARM_PACKAGE_PARM(WidgetPtr, src, p)

				FLAME_PARM_PACKAGE_PARM_SIZE

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
			FLAME_PARM_PACKAGE_END

			Array<Function*> focus_listeners$;
			Array<Function*> key_listeners$;
			Array<Function*> mouse_listeners$;
			Array<Function*> drop_listeners$;
			Array<Function*> changed_listeners$;
			Array<Function*> child_listeners$;

			Array<CommonData> data_storages$;
			Array<StringW> string_storages$;

			inline Widget()
			{
				class_hash$ = 0;

				pos$ = Vec2(0.f);
				size$ = Vec2(0.f);

				alpha$ = 1.f;
				scale$ = 1.f;

				inner_padding$ = Vec4(0.f);
				layout_padding$ = 0.f;

				background_offset$ = Vec4(0.f);
				background_round_radius$ = 0.f;
				background_round_flags$ = 0;
				background_frame_thickness$ = 0.f;
				background_col$ = Bvec4(0);
				background_frame_col$ = Bvec4(255);
				background_shaow_thickness$ = 0.f;

				size_policy_hori$ = SizeFixed;
				size_policy_vert$ = SizeFixed;

				align$ = AlignFree;

				layout_type$ = LayoutFree;
				item_padding$ = 0.f;
				grid_hori_count$ = 1;
				clip$ = false;

				scroll_offset$ = 0.f;

				event_attitude$ = EventAccept;
				want_key_focus$ = false;

				visible$ = true;

				global_pos = Vec2(0.f);
				global_scale = 1.f;

				cliped = false;
				content_size = 0.f;
				showed = false;
				state = StateNormal;

				closet_id$ = 0;

				draw_default$ = true;
			}

			FLAME_UI_EXPORTS void set_width(float x, Widget *sender = nullptr);
			FLAME_UI_EXPORTS void set_height(float y, Widget *sender = nullptr);
			FLAME_UI_EXPORTS void set_size(const Vec2 &v, Widget *sender = nullptr);

			FLAME_UI_EXPORTS void set_visibility(bool v);

			FLAME_UI_EXPORTS Instance *instance() const;
			FLAME_UI_EXPORTS Widget *parent() const;
			FLAME_UI_EXPORTS int layer() const;

			FLAME_UI_EXPORTS void add_child(Widget *w, int layer = 0, int pos = -1, bool delay = false, bool modual = false);
			FLAME_UI_EXPORTS void remove_child(int layer, int idx, bool delay = false);
			FLAME_UI_EXPORTS void remove_child(Widget *w, bool delay = false);
			FLAME_UI_EXPORTS void take_child(int layer, int idx, bool delay = false);
			FLAME_UI_EXPORTS void take_child(Widget *w, bool delay = false);
			FLAME_UI_EXPORTS void clear_children(int layer, int begin, int end, bool delay = false);
			FLAME_UI_EXPORTS void take_children(int layer, int begin, int end, bool delay = false);
			FLAME_UI_EXPORTS void remove_from_parent(bool delay = false);
			FLAME_UI_EXPORTS void take_from_parent(bool delay = false);
			FLAME_UI_EXPORTS int find_child(Widget *w);
			FLAME_UI_EXPORTS void set_to_foreground();

			FLAME_UI_EXPORTS float get_content_size() const;

			FLAME_UI_EXPORTS void arrange();

			FLAME_UI_EXPORTS void add_extra_draw(PF pf, const std::vector<CommonData> &capt);

			FLAME_UI_EXPORTS void add_style(int closet_id, PF pf, const std::vector<CommonData> &capt);
			FLAME_UI_EXPORTS void remove_style(int idx);
			FLAME_UI_EXPORTS void add_animation(float duration, int looping, PF pf, const std::vector<CommonData> &capt);

			FLAME_UI_EXPORTS void on_draw(Canvas *c, const Vec2 &off, float scl);
			FLAME_UI_EXPORTS void on_focus(FocusType type, int focus_or_keyfocus);
			FLAME_UI_EXPORTS void on_key(KeyState action, int value);
			FLAME_UI_EXPORTS void on_mouse(KeyState action, MouseKey key, const Vec2 &value);
			FLAME_UI_EXPORTS void on_drop(Widget *src);
			FLAME_UI_EXPORTS void on_changed();

			FLAME_UI_EXPORTS Function *add_listener(Listener l , PF pf, void *thiz, const std::vector<CommonData> &capt);
			FLAME_UI_EXPORTS void remove_listener(Listener l, Function *f, bool delay = false);

			FLAME_UI_EXPORTS void add_data_storages(const char *fmt);
			FLAME_UI_EXPORTS void add_string_storages(int count);

			FLAME_UI_EXPORTS SerializableNode *save();

			FLAME_UI_EXPORTS static Widget *create(Instance *ui);
			FLAME_UI_EXPORTS static void create_from_typeinfo(Instance *ui, VaribleInfo *info, void *p, Widget *dst);
			FLAME_UI_EXPORTS static Widget *create_from_file(Instance *ui, SerializableNode *src);
			FLAME_UI_EXPORTS static void destroy(Widget *w);
		};

		struct wLayout : Widget
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS static wLayout *create(Instance *ui);
		};

		typedef wLayout* wLayoutPtr;

		struct wCheckbox : Widget
		{
			FLAME_UI_EXPORTS void init(void *target = nullptr);

			FLAME_UI_EXPORTS int &checked();
			FLAME_UI_EXPORTS voidptr &target();

			FLAME_UI_EXPORTS static wCheckbox *create(Instance *ui, void *target = nullptr);
		};

		typedef wCheckbox* wCheckboxPtr;

		struct wText : Widget
		{
			FLAME_UI_EXPORTS void init();

			FLAME_UI_EXPORTS Bvec4 &text_col();
			FLAME_UI_EXPORTS float &sdf_scale();
			FLAME_UI_EXPORTS StringW &text();

			FLAME_UI_EXPORTS void set_size_auto();

			FLAME_UI_EXPORTS static wText *create(Instance *ui);
		};

		typedef wText* wTextPtr;

		struct wButton : wText
		{
			FLAME_UI_EXPORTS void init();

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

		struct wMenuItem : wText
		{
			FLAME_UI_EXPORTS void init(const wchar_t *title);

			FLAME_UI_EXPORTS static wMenuItem *create(Instance *ui, const wchar_t *title);
		};

		typedef wMenuItem* wMenuItemPtr;

		struct wMenu : wLayout
		{
			FLAME_UI_EXPORTS void init(const wchar_t *title);

			FLAME_UI_EXPORTS int &sub();
			FLAME_UI_EXPORTS int &opened();
			FLAME_UI_EXPORTS wTextPtr &w_title();
			FLAME_UI_EXPORTS wTextPtr &w_rarrow();
			FLAME_UI_EXPORTS wLayoutPtr &w_items();

			FLAME_UI_EXPORTS void open();
			FLAME_UI_EXPORTS void popup(const Vec2 &pos);
			FLAME_UI_EXPORTS void close();

			FLAME_UI_EXPORTS static wMenu *create(Instance *ui, const wchar_t *title);
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
			FLAME_UI_EXPORTS void init(void *enum_info = nullptr, void *target = nullptr);

			FLAME_UI_EXPORTS int &sel();
			FLAME_UI_EXPORTS voidptr &enum_info();
			FLAME_UI_EXPORTS voidptr &target();
			
			FLAME_UI_EXPORTS void set_sel(int idx, bool from_inner = false);

			FLAME_UI_EXPORTS static wCombo *create(Instance *ui, void *enum_info = nullptr, void *target = nullptr);
		};

		typedef wCombo* wComboPtr;

		struct wEdit : wText
		{
			enum Type
			{
				TypeNull,
				TypeString,
				TypeStringW,
				TypeInt,
				TypeUint,
				TypeFloat,
				TypeUchar
			};

			FLAME_UI_EXPORTS void init(Type type = TypeNull, void *target = nullptr);

			FLAME_UI_EXPORTS int &cursor();
			FLAME_UI_EXPORTS int &type();
			FLAME_UI_EXPORTS voidptr &target();

			FLAME_UI_EXPORTS void set_size_by_width(float width);

			FLAME_UI_EXPORTS static wEdit *create(Instance *ui, Type type = TypeNull, void *target = nullptr);
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
			FLAME_UI_EXPORTS void init(const wchar_t *title);

			FLAME_UI_EXPORTS wTextPtr &w_title();

			FLAME_UI_EXPORTS static wListItem *create(Instance *ui, const wchar_t *title);
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
			FLAME_UI_EXPORTS void init(const wchar_t *title);

			FLAME_UI_EXPORTS wTextPtr &w_title();
			FLAME_UI_EXPORTS wLayoutPtr &w_items();
			FLAME_UI_EXPORTS wTextPtr &w_larrow();

			FLAME_UI_EXPORTS static wTreeNode *create(Instance *ui, const wchar_t *title);
		};

		typedef wTreeNode* wTreeNodePtr;

		struct wDialog : wLayout
		{
			FLAME_UI_EXPORTS void init(bool resize = false, bool modual = false);

			FLAME_UI_EXPORTS wScrollbarPtr &w_scrollbar();
			FLAME_UI_EXPORTS wSizeDragPtr &w_sizedrag();

			FLAME_UI_EXPORTS static wDialog *create(Instance *ui, bool resize = false, bool modual = false);
		};

		struct wMessageDialog : wDialog 
		{
			FLAME_UI_EXPORTS void init(const wchar_t *text);

			FLAME_UI_EXPORTS wTextPtr &w_text();
			FLAME_UI_EXPORTS wButtonPtr &w_ok();

			FLAME_UI_EXPORTS static wMessageDialog *create(Instance *ui, const wchar_t *text);
		};

		//struct wYesNoDialog : wDialog
		//{
		//	FLAME_UI_EXPORTS void init(const wchar_t *text, const wchar_t *prompt, cconst std::function<void(bool)> &callback);

		//	FLAME_UI_EXPORTS wTextPtr &w_text();
		//	FLAME_UI_EXPORTS wLayoutPtr &w_buttons();
		//	FLAME_UI_EXPORTS wButtonPtr &w_yes();
		//	FLAME_UI_EXPORTS wButtonPtr &w_no();

		//	FLAME_UI_EXPORTS static wYesNoDialog *create(Instance *ui, const wchar_t *text, const wchar_t *prompt, const std::function<void(bool)> &callback);
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
