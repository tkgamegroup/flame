#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/widget.h>

namespace flame
{
	struct cWidgetPrivate : cWidget
	{
		cElement* element;

		//Array<Function<FoucusListenerParm>> focus_listeners$;
		//Array<Function<KeyListenerParm>> key_listeners$;
		//Array<Function<MouseListenerParm>> mouse_listeners$;
		//Array<Function<DropListenerParm>> drop_listeners$;
		//Array<Function<ChangedListenerParm>> changed_listeners$;
		//Array<Function<ChildListenerParm>> child_listeners$;

		cWidgetPrivate() :
			element(nullptr)
		{
			blackhole = false;
			want_key = false;

			hovering.v = false;
			hovering.frame = 0;
			dragging.v = false;
			dragging.frame = 0;
			focusing.v = false;
			focusing.frame = 0;
		}

		void on_attach()
		{
			element = (cElement*)(entity->component(cH("Element")));
			assert(element);
		}

		void update()
		{
		}

		bool contains(const Vec2f& pos) const
		{
			return rect_contains(Vec4f(element->global_x.v, element->global_y.v, element->global_width.v, element->global_height.v), pos);
		}
	};

	cWidget::~cWidget()
	{
	}

#define NAME "Widget"
	const char* cWidget::type_name() const
	{
		return NAME;
	}

	uint cWidget::type_hash() const
	{
		return cH(NAME);
	}
#undef NAME

	void cWidget::on_attach()
	{
		((cWidgetPrivate*)this)->on_attach();
	}

	void cWidget::update()
	{
		((cWidgetPrivate*)this)->update();
	}

	bool cWidget::contains(const Vec2f& pos) const
	{
		return ((cWidgetPrivate*)this)->contains(pos);
	}

	cWidget* cWidget::create()
	{
		return new cWidgetPrivate();
	}
}
