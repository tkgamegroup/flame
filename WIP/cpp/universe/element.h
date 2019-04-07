#include <flame/foundation/foundation.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/universe.h>
#include <flame/universe/style.h>
#include <flame/universe/animation.h>

namespace flame
{
	struct VariableInfo;
	struct SerializableNode;

	struct UI;

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

	struct Element;
	typedef Element* ElementPtr;

	struct Element
	{
		enum Flag
		{
			FlagNull,
			FlagJustCreated,
			FlagJustCreatedNeedModual,
			FlagNeedToRemoveFromParent,
			FlagNeedToTakeFromParent
		};

		SizePolicy size_policy_hori$;
		SizePolicy size_policy_vert$;

		Align align$;

		LayoutType layout_type$;
		float item_padding$;
		int grid_hori_count$;
		bool clip$;

		float scroll_offset$;

		bool want_key_focus$;

		bool visible$;

		Vec2 global_pos; // vaild after instance processing
		float global_scale; // vaild after instance processing

		bool cliped; // valid after arranging by parent
		int content_size; // valid after arranging
		bool showed; // vaild after processing
		State state; // vaild after processing

		UI* ui;
		Element* parent;
		int layer;
		Flag flag;
		bool need_arrange;

		Array<Element*> children_1$;
		Array<Element*> children_2$;

		bool draw_default$;
		FLAME_PACKAGE_BEGIN_4(ExtraDrawParm, ElementPtr, thiz, p, graphics::CanvasPtr, canvas, p, Vec2, off, f2, float, scl, f1)
		FLAME_PACKAGE_END_4
		Array<Function<ExtraDrawParm>> extra_draws$;

		int closet_id$;
		int style_level;
		Array<Style> styles$;

		Array<Animation> animations$;

		FLAME_PACKAGE_BEGIN_3(FoucusListenerParm, ElementPtr, thiz, p, FocusType, type, i1, int, is_keyfocus, i1)
		FLAME_PACKAGE_END_3

		FLAME_PACKAGE_BEGIN_3(KeyListenerParm, ElementPtr, thiz, p, KeyState, action, i1, int, value, i1)
		FLAME_PACKAGE_END_3

		FLAME_PACKAGE_BEGIN_4(MouseListenerParm, ElementPtr, thiz, p, KeyState, action, i1, MouseKey, key, i1, Vec2, value, f2)
		FLAME_PACKAGE_END_4

		FLAME_PACKAGE_BEGIN_2(DropListenerParm, ElementPtr, thiz, p, ElementPtr, src, p)
		FLAME_PACKAGE_END_2

		FLAME_PACKAGE_BEGIN_1(ChangedListenerParm, ElementPtr, thiz, p)
		FLAME_PACKAGE_END_1

		enum ChildOp
		{
			ChildAdd,
			ChildRemove
		};
		FLAME_PACKAGE_BEGIN_3(ChildListenerParm, ElementPtr, thiz, p, ChildOp, op, i1, ElementPtr, src, p)
		FLAME_PACKAGE_END_3

		FLAME_UNIVERSE_EXPORTS Element(UI* ui);
		FLAME_UNIVERSE_EXPORTS ~Element();

		FLAME_UNIVERSE_EXPORTS void set_width(float x, Element* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_height(float y, Element* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_size(const Vec2& v, Element* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS void set_visibility(bool v);

		FLAME_UNIVERSE_EXPORTS void add_child(Element* w, int layer = 0, int pos = -1, bool modual = false);
		FLAME_UNIVERSE_EXPORTS void remove_child(int layer, int idx, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void take_child(int layer, int idx, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void clear_children(int layer, int begin, int end, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void take_children(int layer, int begin, int end, bool delay = false);
		FLAME_UNIVERSE_EXPORTS void remove_from_parent(bool delay = false);
		FLAME_UNIVERSE_EXPORTS void take_from_parent(bool delay = false);
		FLAME_UNIVERSE_EXPORTS int find_child(int layer, Element* w);
		FLAME_UNIVERSE_EXPORTS void set_to_foreground();

		FLAME_UNIVERSE_EXPORTS void do_arrange(); // only ui can call this

		FLAME_UNIVERSE_EXPORTS float get_content_size() const;

		FLAME_UNIVERSE_EXPORTS void remove_animations();

		FLAME_UNIVERSE_EXPORTS void on_draw(graphics::Canvas* c, const Vec2& off, float scl);
		FLAME_UNIVERSE_EXPORTS void on_focus(FocusType type, int is_keyfocus);
		FLAME_UNIVERSE_EXPORTS void on_key(KeyState action, int value);
		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyState action, MouseKey key, const Vec2& value);
		FLAME_UNIVERSE_EXPORTS void on_drop(Element* src);
		FLAME_UNIVERSE_EXPORTS void on_changed();

		FLAME_UNIVERSE_EXPORTS SerializableNode* save();

		enum { DATA_SIZE = 0 };

		FLAME_UNIVERSE_EXPORTS static Element* create(UI* ui);
		template<typename T, typename ... Args>
		inline static T* createT(UI* ui, Args ... args)
		{
			auto w = (T*)create(ui);
			w->init(args...);
			return w;
		}
		FLAME_UNIVERSE_EXPORTS static void create_from_typeinfo(UI* ui, int font_atlas_index, VariableInfo* info, void* p, Element* dst); // use variable to create element, e.g. string->edit, bool->checkbox
		FLAME_UNIVERSE_EXPORTS static Element* create_from_file(UI* ui, SerializableNode* src);
		FLAME_UNIVERSE_EXPORTS static void destroy(Element* w);
	};

	FLAME_ELEMENT_BEGIN_0(wLayout, Element)
		FLAME_UNIVERSE_EXPORTS void init(LayoutType type = LayoutFree, float item_padding = 0.f);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wCheckbox, Element, int, checked, i1, voidptr, target, p)
		FLAME_UNIVERSE_EXPORTS void init(void* target = nullptr);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_3(wText, Element, int, font_atlas_index, i1, Bvec4, text_col, b4, float, sdf_scale, f1)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index);
		FLAME_UNIVERSE_EXPORTS void set_size_auto();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_0(wButton, wText)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* title);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_1(wToggle, wText, int, toggled, i1)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index);
		FLAME_UNIVERSE_EXPORTS void set_toggle(bool v);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_0(wMenuItem, wText)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* title);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_5(wMenu, wLayout, bool, sub, i1, bool, opened, i1, wTextPtr, w_title, p, wTextPtr, w_rarrow, p, wLayoutPtr, w_items, p)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* title, bool only_for_context_menu = false);
		FLAME_UNIVERSE_EXPORTS void open();
		FLAME_UNIVERSE_EXPORTS void popup(const Vec2& pos);
		FLAME_UNIVERSE_EXPORTS void close();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_0(wMenuBar, wLayout)
		FLAME_UNIVERSE_EXPORTS void init();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_3(wCombo, wMenu, int, sel, i1, voidptr, enum_info, p, voidptr, target, p)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, void* enum_info = nullptr, void* target = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_sel(int idx, bool from_inner = false);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_3(wEdit, wText, int, cursor, i1, voidptr, info, p, voidptr, target, p)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, void* info = nullptr, void* target = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_size_by_width(float width);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_5(wImage, Element, int, id, i1, Vec2, uv0, f2, Vec2, uv1, f2, int, stretch, i1, Vec4, border/* L R T B */, f4)
		FLAME_UNIVERSE_EXPORTS void init();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wScrollbar, wLayout, wButtonPtr, w_btn, p, ElementPtr, w_target, p)
		FLAME_UNIVERSE_EXPORTS void init(Element* target);
		FLAME_UNIVERSE_EXPORTS void scroll(int v);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wSizeDrag, Element, ElementPtr, w_target, p, Vec2, min_size, f2)
		FLAME_UNIVERSE_EXPORTS void init(Element* target);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_3(wSplitter, Element, int, dir /* 0, else = hori, vert */, i1, ElementPtr, w_target1, p, ElementPtr, w_target2, p)
		FLAME_UNIVERSE_EXPORTS void init(int dir, Element* target1, Element* target2);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_1(wListItem, wLayout, wTextPtr, w_title, p)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* title);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wList, wLayout, wListItemPtr, w_sel, p, wScrollbarPtr, w_scrollbar, p)
		FLAME_UNIVERSE_EXPORTS void init();
	FLAME_ELEMENT_END

	struct wTree;
	FLAME_ELEMENT_BEGIN_3(wTreeNode, wLayout, wTextPtr, w_title, p, wLayoutPtr, w_items, p, wTextPtr, w_larrow, p)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* title, wTree* tree = nullptr);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_1(wTree, wLayout, wTreeNodePtr, w_sel, p)
		FLAME_UNIVERSE_EXPORTS void init();
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wDialog, wLayout, wScrollbarPtr, w_scrollbar, p, wSizeDragPtr, w_sizedrag, p)
		FLAME_UNIVERSE_EXPORTS void init(bool resize = false, bool modual = false);
	FLAME_ELEMENT_END

	FLAME_ELEMENT_BEGIN_2(wMessageDialog, wDialog, wTextPtr, w_text, p, wButtonPtr, w_ok, p)
		FLAME_UNIVERSE_EXPORTS void init(int font_atlas_index, const wchar_t* text);
	FLAME_ELEMENT_END

	//struct wYesNoDialog : wDialog
	//{
	//	FLAME_UNIVERSE_EXPORTS void init(const wchar_t *text, const wchar_t *prompt, cconst std::function<void(bool)> &callback);

	//	FLAME_UNIVERSE_EXPORTS wTextPtr &w_text();
	//	FLAME_UNIVERSE_EXPORTS wLayoutPtr &w_buttons();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_yes();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_no();
	//};

	//struct wInputDialog : wDialog
	//{
	//	FLAME_UNIVERSE_EXPORTS void init(const wchar_t *title, float sdf_scale, const std::function<void(bool ok, const wchar_t *input)> &callback);

	//	FLAME_UNIVERSE_EXPORTS wEditPtr &w_input();
	//	FLAME_UNIVERSE_EXPORTS wLayoutPtr &w_buttons();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_ok();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_cancel();
	//};

	//struct wFileDialog : wDialog
	//{
	//	FLAME_UNIVERSE_EXPORTS void init(const wchar_t *title, int io, const std::function<void(bool ok, const wchar_t *filename)> &callback, const wchar_t *exts = nullptr);

	//	FLAME_UNIVERSE_EXPORTS wMenuBarPtr &w_pathstems();
	//	FLAME_UNIVERSE_EXPORTS wListPtr &w_list();
	//	FLAME_UNIVERSE_EXPORTS wEditPtr &w_input();
	//	FLAME_UNIVERSE_EXPORTS wComboPtr &w_ext();
	//	FLAME_UNIVERSE_EXPORTS wLayoutPtr &w_buttons();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_ok();
	//	FLAME_UNIVERSE_EXPORTS wButtonPtr &w_cancel();

	//	FLAME_UNIVERSE_EXPORTS const wchar_t *curr_path();
	//	FLAME_UNIVERSE_EXPORTS int curr_path_len();
	//	FLAME_UNIVERSE_EXPORTS void set_curr_path(const wchar_t *path);
	//	FLAME_UNIVERSE_EXPORTS const wchar_t *curr_exts();
	//	FLAME_UNIVERSE_EXPORTS int curr_exts_len();
	//	FLAME_UNIVERSE_EXPORTS void set_curr_exts(const wchar_t *exts);

	//	FLAME_UNIVERSE_EXPORTS void set_path(const wchar_t *path);
	//};

	FLAME_ELEMENT_BEGIN_0(wDocker, Element)
		enum Dir
		{
			DirCenter = 1 << 0,
			DirLeft = 1 << 1,
			DirRight = 1 << 2,
			DirTop = 1 << 3,
			DirBottom = 1 << 4,
			DirAll = DirCenter | DirLeft | DirRight | DirTop | DirBottom
		};

		enum Type
		{
			TypeCenter,
			TypeHorizontal,
			TypeVertical
		};

		FLAME_UNIVERSE_EXPORTS void init();
	FLAME_ELEMENT_END
}
