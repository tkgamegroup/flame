#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>

namespace flame
{
	struct cStyleColorPrivate : cStyleColor
	{
		cStyleColorPrivate(const Vec4c& _color_normal, const Vec4c& _color_hovering, const Vec4c& _color_active)
		{
			element = nullptr;
			event_receiver = nullptr;

			color_normal = _color_normal;
			color_hovering = _color_hovering;
			color_active = _color_active;
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
		}

		void update()
		{
			if (event_receiver->active)
				element->color = color_active;
			else if (event_receiver->hovering)
				element->color = color_hovering;
			else
				element->color = color_normal;
		}

		Component* copy()
		{
			return new cStyleColorPrivate(color_normal, color_hovering, color_active);
		}
	};

	void cStyleColor::start()
	{
		((cStyleColorPrivate*)this)->start();
	}

	void cStyleColor::update()
	{
		((cStyleColorPrivate*)this)->update();
	}

	Component* cStyleColor::copy()
	{
		return ((cStyleColorPrivate*)this)->copy();
	}

	cStyleColor* cStyleColor::create(const Vec4c& color_normal, const Vec4c& color_hovering, const Vec4c& color_active)
	{
		return new cStyleColorPrivate(color_normal, color_hovering, color_active);
	}

	struct cStyleTextColorPrivate : cStyleTextColor
	{
		cStyleTextColorPrivate(const Vec4c& _color_normal, const Vec4c& _color_else)
		{
			text = nullptr;
			event_receiver = nullptr;

			color_normal = _color_normal;
			color_else = _color_else;
		}

		void start()
		{
			text = (cText*)(entity->find_component(cH("Text")));
			assert(text);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
		}

		void update()
		{
			if (event_receiver->active || event_receiver->hovering)
				text->color = color_else;
			else
				text->color = color_normal;
		}

		Component* copy()
		{
			return new cStyleTextColorPrivate(color_normal, color_else);
		}
	};

	void cStyleTextColor::start()
	{
		((cStyleTextColorPrivate*)this)->start();
	}

	void cStyleTextColor::update()
	{
		((cStyleTextColorPrivate*)this)->update();
	}

	Component* cStyleTextColor::copy()
	{
		return ((cStyleTextColorPrivate*)this)->copy();
	}

	cStyleTextColor* cStyleTextColor::create(const Vec4c& color_normal, const Vec4c& color_else)
	{
		return new cStyleTextColorPrivate(color_normal, color_else);
	}
}
