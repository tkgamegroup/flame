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
			StringAndHash class$;

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

				FLAME_PARM_PACKAGE_SEPARATOR

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
			FLAME_PARM_PACKAGE_END
			Array<Function*> extra_draws$;

			int closet_id$;
			FLAME_PARM_PACKAGE_BEGIN(StyleParm)
				FLAME_PARM_PACKAGE_SEPARATOR

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
				FLAME_PARM_PACKAGE_DEFAULT_CAPT(int, closet_id, i1)
			FLAME_PARM_PACKAGE_END
			int style_level;
			Array<Function*> styles$;

			FLAME_PARM_PACKAGE_BEGIN(AnimationParm)
				FLAME_PARM_PACKAGE_PARM(float, time, f1)

				FLAME_PARM_PACKAGE_SEPARATOR

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

				FLAME_PARM_PACKAGE_SEPARATOR

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
			FLAME_PARM_PACKAGE_END

			FLAME_PARM_PACKAGE_BEGIN(KeyListenerParm)
				/*
					- when key down/up, action is KeyStateDown or KeyStateUp, value is Key
					- when char, action is KeyStateNull, value is ch
				*/
				FLAME_PARM_PACKAGE_PARM(KeyState, action, i1)
				FLAME_PARM_PACKAGE_PARM(int, value, i1)

				FLAME_PARM_PACKAGE_SEPARATOR

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

				FLAME_PARM_PACKAGE_SEPARATOR

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
			FLAME_PARM_PACKAGE_END

			FLAME_PARM_PACKAGE_BEGIN(DropListenerParm)
				FLAME_PARM_PACKAGE_PARM(WidgetPtr, src, p)

				FLAME_PARM_PACKAGE_SEPARATOR

				FLAME_PARM_PACKAGE_DEFAULT_CAPT(WidgetPtr, thiz, p)
			FLAME_PARM_PACKAGE_END

			FLAME_PARM_PACKAGE_BEGIN(ChangedListenerParm)
				FLAME_PARM_PACKAGE_SEPARATOR

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

				FLAME_PARM_PACKAGE_SEPARATOR

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
				class$ = "";

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

			FLAME_UI_EXPORTS void add_style(int closet_id, PF pf, const std::vector<CommonData> &capt, int pos = -1);
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

			FLAME_UI_EXPORTS void add_data_storages(const std::vector<CommonData> &datas);
			FLAME_UI_EXPORTS void add_string_storages(int count);

			FLAME_UI_EXPORTS SerializableNode *save();

			enum { DATA_SIZE = 0 };
			enum { STRING_SIZE = 0 };

			FLAME_UI_EXPORTS static Widget *create(Instance *ins);
			template<typename T, typename ... Args>
			inline static T *createT(Instance *ins, Args ... args)
			{
				auto w = (T*)create(ins);
				w->init(args...);
				return w;
			}
			FLAME_UI_EXPORTS static void create_from_typeinfo(Instance *ins, VaribleInfo *info, void *p, Widget *dst);
			FLAME_UI_EXPORTS static Widget *create_from_file(Instance *ins, SerializableNode *src);
			FLAME_UI_EXPORTS static void destroy(Widget *w);
		};

#define FLAME_WIDGET_BEGIN(name, base) \
	struct name;\
	typedef name* name##Ptr;\
	struct name : base\
	{\
		enum { D_BASE = __COUNTER__ + 1 };\
		enum { B_D_SIZE = base::DATA_SIZE };\
		enum { B_S_SIZE = base::STRING_SIZE };
#define FLAME_WIDGET_DATA(t, n, tf) \
		inline t &n()\
		{\
			return (t&)data_storages$[__COUNTER__ - D_BASE + B_D_SIZE].tf();\
		}
#define FLAME_WIDGET_SEPARATOR \
		enum { DATA_SIZE = __COUNTER__ - D_BASE + B_D_SIZE };\
		enum { S_BASE = __COUNTER__ + 1 };
#define FLAME_WIDGET_STRING(n) \
		inline StringW &n()\
		{\
			return string_storages$[__COUNTER__ - S_BASE + B_S_SIZE];\
		}
#define FLAME_WIDGET_END \
		enum { STRING_SIZE = __COUNTER__ - S_BASE + B_S_SIZE };\
	};
		
		FLAME_WIDGET_BEGIN(wLayout, Widget)
			FLAME_UI_EXPORTS void init(LayoutType type = LayoutFree, float item_padding = 0.f);
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wCheckbox, Widget)
			FLAME_UI_EXPORTS void init(void *target = nullptr);
			FLAME_WIDGET_DATA(int, checked, i1)
			FLAME_WIDGET_DATA(voidptr, target, p)
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wText, Widget)
			FLAME_UI_EXPORTS void init();
			FLAME_WIDGET_DATA(Bvec4, text_col, b4)
			FLAME_WIDGET_DATA(float, sdf_scale, f1)
			FLAME_WIDGET_SEPARATOR
			FLAME_WIDGET_STRING(text)
			FLAME_UI_EXPORTS void set_size_auto();
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wButton, wText)
			FLAME_UI_EXPORTS void init(const wchar_t *title);
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wToggle, wText)
			FLAME_UI_EXPORTS void init();
			FLAME_WIDGET_DATA(int, toggled, i1)
			FLAME_WIDGET_SEPARATOR
			FLAME_UI_EXPORTS void set_toggle(bool v);
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wMenuItem, wText)
			FLAME_UI_EXPORTS void init(const wchar_t *title);
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wMenu, wLayout)
			FLAME_UI_EXPORTS void init(const wchar_t *title, bool only_for_context_menu = false);
			FLAME_WIDGET_DATA(int, sub, i1)
			FLAME_WIDGET_DATA(int, opened, i1)
			FLAME_WIDGET_DATA(wTextPtr, w_title, p)
			FLAME_WIDGET_DATA(wTextPtr, w_rarrow, p)
			FLAME_WIDGET_DATA(wLayoutPtr, w_items, p)
			FLAME_WIDGET_SEPARATOR
			FLAME_UI_EXPORTS void open();
			FLAME_UI_EXPORTS void popup(const Vec2 &pos);
			FLAME_UI_EXPORTS void close();
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wMenuBar, wLayout)
			FLAME_UI_EXPORTS void init();
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wCombo, wMenu)
			FLAME_UI_EXPORTS void init(void *enum_info = nullptr, void *target = nullptr);
			FLAME_WIDGET_DATA(int, sel, i1)
			FLAME_WIDGET_DATA(voidptr, enum_info, p)
			FLAME_WIDGET_DATA(voidptr, target, p)
			FLAME_WIDGET_SEPARATOR
			FLAME_UI_EXPORTS void set_sel(int idx, bool from_inner = false);
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wEdit, wText)
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
			FLAME_WIDGET_DATA(int, cursor, i1)
			FLAME_WIDGET_DATA(int, type, i1)
			FLAME_WIDGET_DATA(voidptr, target, p)
			FLAME_WIDGET_SEPARATOR
			FLAME_UI_EXPORTS void set_size_by_width(float width);
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wImage, Widget)
			FLAME_UI_EXPORTS void init();
			FLAME_WIDGET_DATA(int, id, i1)
			FLAME_WIDGET_DATA(Vec2, uv0, f2)
			FLAME_WIDGET_DATA(Vec2, uv1, f2)
			FLAME_WIDGET_DATA(int, stretch, i1)
			FLAME_WIDGET_DATA(Vec4, border, f4) // L R T B
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wSizeDrag, Widget)
			FLAME_UI_EXPORTS void init(Widget *target);
			FLAME_WIDGET_DATA(Vec2, min_size, f2)
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wScrollbar, wLayout)
			FLAME_UI_EXPORTS void init(Widget *target);
			FLAME_WIDGET_DATA(wButtonPtr, w_btn, p)
			FLAME_WIDGET_DATA(WidgetPtr, w_target, p)
			FLAME_WIDGET_SEPARATOR
			FLAME_UI_EXPORTS void scroll(int v);
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wListItem, wLayout)
			FLAME_UI_EXPORTS void init(const wchar_t *title);
			FLAME_WIDGET_DATA(wTextPtr, w_title, p)
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wList, wLayout)
			FLAME_UI_EXPORTS void init();
			FLAME_WIDGET_DATA(wListItemPtr, w_sel, p)
			FLAME_WIDGET_DATA(wScrollbarPtr, w_scrollbar, p)
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		struct wTree;
		FLAME_WIDGET_BEGIN(wTreeNode, wLayout)
			FLAME_UI_EXPORTS void init(const wchar_t *title, wTree *tree = nullptr);
			FLAME_WIDGET_DATA(wTextPtr, w_title, p)
			FLAME_WIDGET_DATA(wLayoutPtr, w_items, p)
			FLAME_WIDGET_DATA(wTextPtr, w_larrow, p)
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wTree, wLayout)
			FLAME_UI_EXPORTS void init();
			FLAME_WIDGET_DATA(wTreeNodePtr, w_sel, p)
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wDialog, wLayout)
			FLAME_UI_EXPORTS void init(bool resize = false, bool modual = false);
			FLAME_WIDGET_DATA(wScrollbarPtr, w_scrollbar, p)
			FLAME_WIDGET_DATA(wSizeDragPtr, w_sizedrag, p)
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		FLAME_WIDGET_BEGIN(wMessageDialog, wDialog)
			FLAME_UI_EXPORTS void init(const wchar_t *text);
			FLAME_WIDGET_DATA(wTextPtr, w_text, p)
			FLAME_WIDGET_DATA(wButtonPtr, w_ok, p)
			FLAME_WIDGET_SEPARATOR
		FLAME_WIDGET_END

		//struct wYesNoDialog : wDialog
		//{
		//	FLAME_UI_EXPORTS void init(const wchar_t *text, const wchar_t *prompt, cconst std::function<void(bool)> &callback);

		//	FLAME_UI_EXPORTS wTextPtr &w_text();
		//	FLAME_UI_EXPORTS wLayoutPtr &w_buttons();
		//	FLAME_UI_EXPORTS wButtonPtr &w_yes();
		//	FLAME_UI_EXPORTS wButtonPtr &w_no();
		//};

		//struct wInputDialog : wDialog
		//{
		//	FLAME_UI_EXPORTS void init(const wchar_t *title, float sdf_scale, const std::function<void(bool ok, const wchar_t *input)> &callback);

		//	FLAME_UI_EXPORTS wEditPtr &w_input();
		//	FLAME_UI_EXPORTS wLayoutPtr &w_buttons();
		//	FLAME_UI_EXPORTS wButtonPtr &w_ok();
		//	FLAME_UI_EXPORTS wButtonPtr &w_cancel();
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
		//};
	}
}
